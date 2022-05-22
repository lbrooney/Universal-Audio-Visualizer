#include "Audio/audiorecorder.h"
#include "Audio/AudioMacros.h"
#include <iostream>
using namespace std;
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

AudioRecorder::AudioRecorder(AudioCommons* input) : dataSemaphore(0)
{
    pCommons = input;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;

    pDeviceID = pCommons->getSelectedDeviceID();

    pCommons->getEnumerator()->GetDevice(pDeviceID, &pDevice);

    pDevice->Activate(
                IID_IAudioClient, CLSCTX_ALL,
                NULL, (void**)&pAudioClient);

    pDevice->Activate(
                IID_IAudioEndpointVolume, CLSCTX_ALL,
                NULL, (void**)&pEndpointVolume);

    pAudioClient->GetMixFormat(&pwfx);

    sampleRate = pwfx->nSamplesPerSec;

    pAudioClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_LOOPBACK,
                hnsRequestedDuration,
                0,
                pwfx,
                NULL);

    pAudioClient->GetService(
                IID_IAudioCaptureClient,
                (void**)&pCaptureClient);

    pAudioClient->Start();

    uint_t hop_size = 512;
    aubioIn = new_fvec(hop_size);
    aubioOut = new_fvec(1);
    aubioTempo = new_aubio_tempo("default", 1024, hop_size, sampleRate);

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMECOUNT);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMECOUNT);
    p = fftw_plan_dft_1d(FRAMECOUNT, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    recordingThread = std::thread(&AudioRecorder::Record, this);
}

AudioRecorder::~AudioRecorder()
{
    stopRecording();
    pAudioClient->Stop();  // Stop recording.
    CoTaskMemFree(pDeviceID);
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pEndpointVolume);


    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    del_aubio_tempo(aubioTempo);
    del_fvec(aubioIn);
    del_fvec(aubioOut);
}

void AudioRecorder::stopRecording(void)
{
    if(recordingThread.joinable())
    {
        stopRecordingFlag = true;
        recordingThread.join();
    }
    return;
}

void AudioRecorder::Record(void)
{
    UINT32 numFramesAvailable;
    UINT32 packetLength = 0;
    BYTE* pData;
    DWORD flags;

    int frameCounter = 0;
    double* byteArray = new double[FRAMECOUNT];

    while(!stopRecordingFlag)
    {
        pCaptureClient->GetNextPacketSize(&packetLength);
        while (packetLength != 0 && !stopRecordingFlag)
        {
            pCaptureClient->GetBuffer(
                        &pData,
                        &numFramesAvailable,
                        &flags, NULL, NULL);
            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;  // Tell CopyData to write silence.
            }
            else
            {
                //copy data the in buffer
                for(int i = 0; i < numFramesAvailable; i++, frameCounter++)
                {
                    if(pData)
                    {
                        byteArray[frameCounter] = static_cast<double>(pData[i]);
                    }
                    else
                    {
                        byteArray[frameCounter] = 0;
                    }

                    if(frameCounter == FRAMECOUNT - 1)
                    {
                        frameCounter = 0;
                        dataQueue.push(byteArray);
                        dataSemaphore.release();
                        byteArray = new double[FRAMECOUNT];
                    }
                }

                const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pData);
                for(int i = 0; i < numFramesAvailable; i++)
                {
                    if(pData)
                    {
                        float sample = *reinterpret_cast<const float*>(ptr);
                        tempoQueue.push(sample);
                    }
                    else
                    {
                        tempoQueue.push(0.0f);
                    }

                    ptr += sizeof(float);
                }
            }

            pCaptureClient->ReleaseBuffer(numFramesAvailable);
            pCaptureClient->GetNextPacketSize(&packetLength);
        }
        //if no audio is playing, fill data buffers with 0
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
        {
            while(frameCounter < FRAMECOUNT )
            {
                byteArray[frameCounter] = 0;
                tempoQueue.push(0.0f);
                frameCounter++;
            }
            frameCounter = 0;
            dataQueue.push(byteArray);
            dataSemaphore.release();
            byteArray = new double[FRAMECOUNT];
            Sleep(25);
        }
    }
}

void AudioRecorder::ProcessData()
{
    double* data = dataQueue.front();
    int aubioIndex = 0;
    for(int dataIndex = 0; dataIndex < FRAMECOUNT; dataIndex++)
    {
        //apply Hann window function to captured data
        double multiplier = 0.5 * (1 - cos(2 * 3.1416 * dataIndex) / (FRAMECOUNT - 1));
        in[dataIndex][0] =  data[dataIndex] * multiplier;
        in[dataIndex][1] = 0;
    }

    for(int i = 0; i < FRAMECOUNT && !tempoQueue.empty(); i++, aubioIndex++)
    {
        fvec_set_sample(aubioIn, tempoQueue.front(), aubioIndex);
        tempoQueue.pop();
        if(aubioIndex == 511)
        {
            aubio_tempo_do(aubioTempo, aubioIn, aubioOut);
            if (aubioOut->data[0] != 0) {
                bpm = aubio_tempo_get_bpm(aubioTempo);
                std::cout << "Realtime Tempo: " << aubio_tempo_get_bpm(aubioTempo) << " " << aubio_tempo_get_confidence(aubioTempo) << std::endl;
            }
            aubioIndex = -1;
        }
    }

    delete[] data;
    dataQueue.pop();

    //run FFT on data
    fftw_execute(p);

    //calculate log magnitude on transformed data
    for(int j = 0; j < FRAMECOUNT / 2; j++)
    {
        float r = out[j][0] / FRAMECOUNT;
        float i = out[j][1] / FRAMECOUNT;
        mag[j] = log(sqrt((r * r) + (i * i))) * 20;
    }
}

float AudioRecorder::GetVolume()
{
    float vol = 0.0f;
    pEndpointVolume->GetMasterVolumeLevelScalar(&vol);
    return vol;
}

float AudioRecorder::SetVolume(float vol)
{
    if(pEndpointVolume->SetMasterVolumeLevelScalar(vol, NULL))
    {
        return vol;
    }
    return -1;
}
