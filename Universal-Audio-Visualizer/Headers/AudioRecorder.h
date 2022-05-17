#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <cmath>
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <endpointvolume.h>
#include <time.h>
#include <fftw3.h>
#include <iostream>
#include <vector>
#include <aubio/aubio.h>

#include <thread>
#include <atomic>
#include <queue>
#include <semaphore>

#pragma comment(lib, "winmm.lib")

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  1000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
                  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
                  if ((punk) != NULL)  \
                    { (punk)->Release(); (punk) = NULL; }

#define N 1024

class AudioRecorder
{
public:
    AudioRecorder();
    ~AudioRecorder();
    void Record(std::atomic_bool &exit_flag);
    void ProcessData();
    float GetVolume();
    std::thread RecordThread(std::atomic_bool &exit_flag)
    {
        return std::thread(&AudioRecorder::Record, this, std::ref(exit_flag));
    }

    BOOL bDone = FALSE;
    double mag[N/2];
    DWORD sampleRate;
    smpl_t bpm = 0;
    std::queue<double*> dataQueue;
    std::queue<float> tempoQueue;

    std::counting_semaphore<100> dataSemaphore;

private:
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    IAudioEndpointVolume* pEndpointVolume = NULL;

    fvec_t* aubioIn;
    fvec_t* aubioOut;
    aubio_tempo_t* aubioTempo;

    fftw_complex* in;
    fftw_complex* out;
    fftw_plan p;
};


#endif // AUDIORECORDER_H
