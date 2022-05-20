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

#include "Audio/audiocommons.h"

#pragma comment(lib, "winmm.lib")

// REFERENCE_TIME time units per second and per millisecond
const REFERENCE_TIME REFTIMES_PER_SEC = 1000000;
const REFERENCE_TIME REFTIMES_PER_MILLISEC =  10000;

const int frameCount = 1024;

class AudioRecorder
{
public:
    AudioRecorder(AudioCommons* input = nullptr);
    ~AudioRecorder();
    void Record(void);
    void ProcessData();
    float GetVolume();

    BOOL bDone = FALSE;
    double mag[frameCount/2];
    DWORD sampleRate;
    smpl_t bpm = 0;
    std::queue<double*> dataQueue;
    std::queue<float> tempoQueue;

    std::counting_semaphore<100> dataSemaphore;



private:

    AudioCommons* pCommons = nullptr;
    std::thread recordingThread;
    std::atomic_bool stopRecordingFlag = false;

    LPWSTR pDeviceID = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pwfx = nullptr;
    IAudioEndpointVolume* pEndpointVolume = nullptr;

    fvec_t* aubioIn;
    fvec_t* aubioOut;
    aubio_tempo_t* aubioTempo;

    fftw_complex* in;
    fftw_complex* out;
    fftw_plan p;

    void stopRecording(void);
};


#endif // AUDIORECORDER_H
