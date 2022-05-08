#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <time.h>
#include <fftw3.h>
#include <iostream>
#pragma comment(lib, "winmm.lib")

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
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
    void Record();
    void ProcessData(BYTE* pData);

    BOOL bDone = FALSE;
    double mag[N/2];

private:
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pwfx = NULL;


    fftw_complex* in;
    fftw_complex* out;
    fftw_plan p;
};


#endif // AUDIORECORDER_H
