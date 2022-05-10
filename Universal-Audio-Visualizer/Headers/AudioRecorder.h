#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <cmath>
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <time.h>
#include <fftw3.h>
#include <iostream>
#include <aubio/aubio.h>
#pragma comment(lib, "winmm.lib")

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  500000
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
    void Test();

    BOOL bDone = FALSE;
    double mag[N/2];
    DWORD sampleRate;
    smpl_t bpm = 0;

private:
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pwfx = NULL;

    fvec_t* aubioIn;
    fvec_t* aubioOut;
    aubio_tempo_t* aubioTempo;

    fftw_complex* in;
    fftw_complex* out;
    fftw_plan p;
};


#endif // AUDIORECORDER_H
