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
    uint_t tempoHopSize = 512;
    tempoIn = new_fvec(tempoHopSize);
    tempoOut = new_fvec(1);
    tempoObject = new_aubio_tempo("default", tempoHopSize * 2, tempoHopSize, sampleRate);

    uint_t fftHopSize = FRAMECOUNT;
    fftObject = new_aubio_fft(fftHopSize);
    fftIn = new_fvec(fftHopSize);
    fftOut = new_cvec(fftHopSize);

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

    del_aubio_tempo(tempoObject);
    del_fvec(fftIn);
    del_fvec(tempoOut);
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

    for(int fftIndex = 0, tempoIndex = 0; fftIndex < FRAMECOUNT; fftIndex++, tempoIndex++)
    {
        fvec_set_sample(fftIn, data[fftIndex], fftIndex);
        fvec_set_sample(tempoIn, data[fftIndex], tempoIndex);
        if(tempoIndex == 511)
        {
            tempoIndex = -1;
            aubio_tempo_do(tempoObject, tempoIn, tempoOut);
            if (tempoOut->data[0] != 0) {
                bpm = aubio_tempo_get_bpm(tempoObject);
                #ifdef QT_DEBUG
                    qDebug() << "Realtime Tempo: " << aubio_tempo_get_bpm(tempoObject) << Qt::endl;
                #endif
                myTempo = (double) aubio_tempo_get_bpm(tempoObject);
            }
        }
    }



    aubio_fft_do(fftObject, fftIn, fftOut);
    //calculate log magnitude on transformed data
    for(int j = 0; j < FRAMECOUNT / 4; j+=1)
    {
        float r = fftOut->norm[j] * cos(fftOut->phas[j]);
        float i = fftOut->norm[j] * sin(fftOut->phas[j]);;
        mag[j] = log(sqrt((r * r) + (i * i))) * 10;
    }

    delete[] data;
    dataQueue.pop();
}

smpl_t AudioRecorder::GetBeatPeriod()
{
    return aubio_tempo_get_period_s(tempoObject);
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
