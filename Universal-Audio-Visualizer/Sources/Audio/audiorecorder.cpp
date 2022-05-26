#include "Audio/audiorecorder.h"
#include "Audio/AudioMacros.h"
#include <iostream>
#include <wchar.h>
#include <QDebug>

using namespace std;
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

double myTempo = 0.0;

AudioRecorder::AudioRecorder(AudioCommons* input) : dataSemaphore(0)
{
    pCommons = input;

    pCommons->getSelectedDeviceID(pEndpointID);

    pCommons->getEnumerator()->GetDevice(pEndpointID, &pEndpoint);

    pEndpoint->Activate(
                IID_IAudioClient, CLSCTX_ALL,
                NULL, (void**)&pAudioClient);

    pEndpoint->Activate(
                IID_IAudioEndpointVolume, CLSCTX_ALL,
                NULL, (void**)&pEndpointVolume);

    pAudioClient->GetMixFormat(&pwfx);

    sampleRate = pwfx->nSamplesPerSec;

    pAudioClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_LOOPBACK,
                REFTIMES_PER_SEC,
                0,
                pwfx,
                NULL);

    pAudioClient->GetService(
                IID_IAudioCaptureClient,
                (void**)&pCaptureClient);

    pAudioClient->Start();

    //was 512
    uint_t hop_size = FRAMECOUNT;
    aubioIn = new_fvec(hop_size);
    aubioOut = new_fvec(1);
    aubioTempo = new_aubio_tempo("default", hop_size * 2, hop_size, sampleRate);
    aubioFFT = new_aubio_fft(hop_size);
    aubiofftOut = new_fvec(hop_size);
    aubiofftGrain = new_cvec(hop_size);

    fftwIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMECOUNT);
    fftwOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMECOUNT);
    fftwPlan = fftw_plan_dft_1d(FRAMECOUNT, fftwIn, fftwOut, FFTW_FORWARD, FFTW_ESTIMATE);

    recordingThread = std::thread(&AudioRecorder::Record, this);
}

AudioRecorder::~AudioRecorder()
{
    stopRecording();
    pAudioClient->Stop();  // Stop recording.
    CoTaskMemFree(pEndpointID);
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pEndpointVolume);
    SAFE_RELEASE(pEndpoint);


    fftw_destroy_plan(fftwPlan);
    fftw_free(fftwIn);
    fftw_free(fftwOut);

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

void AudioRecorder::changeRecordingDevice(LPWSTR input)
{
    pAudioClient->Stop();  // Stop recording.
    CoTaskMemFree(pEndpointID);
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pEndpointVolume);
    SAFE_RELEASE(pEndpoint);
    pEndpointID = input;
    pCommons->getEnumerator()->GetDevice(pEndpointID, &pEndpoint);

    pEndpoint->Activate(
                IID_IAudioClient, CLSCTX_ALL,
                NULL, (void**)&pAudioClient);

    pEndpoint->Activate(
                IID_IAudioEndpointVolume, CLSCTX_ALL,
                NULL, (void**)&pEndpointVolume);

    pAudioClient->GetMixFormat(&pwfx);

    sampleRate = pwfx->nSamplesPerSec;

    pAudioClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_LOOPBACK,
                REFTIMES_PER_SEC,
                0,
                pwfx,
                NULL);

    pAudioClient->GetService(
                IID_IAudioCaptureClient,
                (void**)&pCaptureClient);

    pAudioClient->Start();
}

void AudioRecorder::Record(void)
{
    UINT32 numFramesAvailable;
    UINT32 packetLength = 0;
    BYTE* pData;
    DWORD flags = AUDCLNT_BUFFERFLAGS_SILENT;

    int frameCounter = 0;
    float* byteArray = new float[FRAMECOUNT];

    while(!stopRecordingFlag)
    {
        LPWSTR temp = nullptr;
        pCommons->getSelectedDeviceID(temp);
        if(wcscmp(temp, pEndpointID))
        {
            changeRecordingDevice(temp);
        } else {
            CoTaskMemFree(temp);
        }
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
                const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pData);
                for(int i = 0; i < numFramesAvailable; i++, frameCounter++)
                {
                    if(pData)
                    {
                        float sample = *reinterpret_cast<const float*>(ptr);
                        byteArray[frameCounter] = sample;
                    }
                    else
                    {
                        byteArray[frameCounter] = 0.0f;
                    }
                    if(frameCounter == FRAMECOUNT - 1)
                    {
                        frameCounter = -1;
                        dataQueue.push(byteArray);
                        dataSemaphore.release();
                        byteArray = new float[FRAMECOUNT];
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
            byteArray = new float[FRAMECOUNT];
            Sleep(25);
        }
    }
}

void AudioRecorder::ProcessData()
{
    float* data = dataQueue.front();

    for(int i = 0; i < FRAMECOUNT; i++)
    {
        fvec_set_sample(aubioIn, data[i], i);
    }

    aubio_tempo_do(aubioTempo, aubioIn, aubioOut);
    if (aubioOut->data[0] != 0) {
        bpm = aubio_tempo_get_bpm(aubioTempo);
        #ifdef QT_DEBUG
            qDebug() << "Realtime Tempo: " << aubio_tempo_get_bpm(aubioTempo) << " " << aubio_tempo_get_confidence(aubioTempo) << Qt::endl;
        #endif
        myTempo = (double) aubio_tempo_get_bpm(aubioTempo);
    }

    aubio_fft_do(aubioFFT, aubioIn, aubiofftGrain);
    //calculate log magnitude on transformed data
    for(int j = 0; j < FRAMECOUNT / 4; j+=1)
    {
        float r = aubiofftGrain->norm[j] * cos(aubiofftGrain->phas[j]);
        float i = aubiofftGrain->norm[j] * sin(aubiofftGrain->phas[j]);;
        mag[j] = log(sqrt((r * r) + (i * i))) * 10;
    }

    delete[] data;
    dataQueue.pop();
}

smpl_t AudioRecorder::GetBeatPeriod()
{
    return aubio_tempo_get_period_s(aubioTempo);
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
