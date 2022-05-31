#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>
#include <endpointvolume.h>
#include "aubio/aubio.h"
#include <queue>
#include <vector>

const UINT CBBUFFERSIZE = 5;
CONST int FRAMECOUNT = 1024;

//
//  WASAPI Capture class.
class AudioSystem : public IAudioSessionEvents, IMMNotificationClient
{
public:
    //  Public interface to CWASAPICapture.
    AudioSystem();
    bool Initialize();
    void Shutdown();
    bool Start();
    void Stop();
    WORD ChannelCount() { return mixFormat->nChannels; }
    UINT32 SamplesPerSecond() { return mixFormat->nSamplesPerSec; }
    UINT32 BytesPerSample() { return mixFormat->wBitsPerSample / 8; }
    size_t FrameSize() { return frameSize; }
    WAVEFORMATEX *MixFormat() { return mixFormat; }
    UINT32 BufferSize() { return bufferSize; }
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    bool SelectedDefault();
    bool SelectedEndpoint(LPWSTR input);
    smpl_t GetBPM();
    smpl_t GetBeatPeriod();
    std::vector<double>& GetMag();
    float GetVolume();
    float SetVolume(float vol);


private:
    ~AudioSystem(void);  // Destructor is private to prevent accidental deletion.
    LONG                refCount;
    //
    //  Core Audio Capture member variables.
    //
    IMMDeviceEnumerator  *deviceEnumerator;
    IMMDevice            *endpoint;
    LPWSTR               endpointID;
    IAudioClient         *audioClient;
    IAudioCaptureClient  *captureClient;
    IAudioEndpointVolume *endpointVolume;

    HANDLE               captureThread;
    HANDLE               shutdownEvent;
    HANDLE               audioSamplesReadyEvent;
    WAVEFORMATEX         *mixFormat;
    size_t               frameSize;
    UINT32               bufferSize;

    std::queue<std::vector<BYTE>> audioQueue;
    static DWORD __stdcall WASAPICaptureThread(LPVOID Context);
    DWORD DoCaptureThread();
    //
    //  Stream switch related members and methods.
    //
    HANDLE                  streamSwitchEvent;          // Set when the current session is disconnected or the default device changes.
    HANDLE                  streamSwitchCompleteEvent;  // Set when the default device changed.
    IAudioSessionControl    *audioSessionControl;
    bool                    inStreamSwitch;
    bool                    defaultSelected;

    bool InitializeStreamSwitch();
    void TerminateStreamSwitch();
    bool HandleStreamSwitchEvent();

    STDMETHOD(OnDisplayNameChanged) (LPCWSTR /*NewDisplayName*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnIconPathChanged) (LPCWSTR /*NewIconPath*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnSimpleVolumeChanged) (float /*NewSimpleVolume*/, BOOL /*NewMute*/, LPCGUID /*EventContext*/) { return S_OK; }
    STDMETHOD(OnChannelVolumeChanged) (DWORD /*ChannelCount*/, float /*NewChannelVolumes*/[], DWORD /*ChangedChannel*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnGroupingParamChanged) (LPCGUID /*NewGroupingParam*/, LPCGUID /*EventContext*/) {return S_OK; };
    STDMETHOD(OnStateChanged) (AudioSessionState /*NewState*/) { return S_OK; };
    STDMETHOD(OnSessionDisconnected) (AudioSessionDisconnectReason DisconnectReason);
    STDMETHOD(OnDeviceStateChanged) (LPCWSTR /*DeviceId*/, DWORD /*NewState*/) { return S_OK; }
    STDMETHOD(OnDeviceAdded) (LPCWSTR /*DeviceId*/) { return S_OK; };
    STDMETHOD(OnDeviceRemoved) (LPCWSTR /*DeviceId(*/) { return S_OK; };
    STDMETHOD(OnDefaultDeviceChanged) (EDataFlow Flow, ERole Role, LPCWSTR NewDefaultDeviceId);
    STDMETHOD(OnPropertyValueChanged) (LPCWSTR /*DeviceId*/, const PROPERTYKEY /*Key*/){return S_OK; };
    //
    //  IUnknown
    //
    STDMETHOD(QueryInterface)(REFIID iid, void **pvObject);

    //
    //  Utility functions.
    //
    bool InitializeAudioEngine();
    bool LoadFormat();

    //
    // Aubio Stuff.
    //
    smpl_t              bpm;
    fvec_t              *fftIn;
    cvec_t              *fftOut;
    fvec_t              *tempoIn;
    fvec_t              *tempoOut;
    aubio_fft_t         *fftObject;
    aubio_tempo_t       *tempoObject;
    std::vector<double> mag;
    HANDLE              analysisSamplesReadyEvent;
    HANDLE              analysisThread;
    std::queue<float*>  dataQueue;

    bool                InitializeAubio();
    DWORD                   DoAnalysisThread();
    static DWORD __stdcall  AudioAnalysisThread(LPVOID Context);
    void                    AnalyzeAudio();

};


#endif // AUDIOSYSTEM_H
