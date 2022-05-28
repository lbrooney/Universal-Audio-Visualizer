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
#include "boost/circular_buffer.hpp"
#include "aubio/aubio.h"
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
    WORD ChannelCount() { return _MixFormat->nChannels; }
    UINT32 SamplesPerSecond() { return _MixFormat->nSamplesPerSec; }
    UINT32 BytesPerSample() { return _MixFormat->wBitsPerSample / 8; }
    size_t FrameSize() { return _FrameSize; }
    WAVEFORMATEX *MixFormat() { return _MixFormat; }
    UINT32 BufferSize() { return _BufferSize; }
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    bool selectedDefault();
    bool selectedEndpoint(LPWSTR input);
    smpl_t GetBPM();
    smpl_t GetBeatPeriod();
    std::vector<double>& GetMag();
    void ProcessAudio();

private:
    ~AudioSystem(void);  // Destructor is private to prevent accidental deletion.
    LONG                _RefCount;
    //
    //  Core Audio Capture member variables.
    //
    IMMDeviceEnumerator *_deviceEnumerator;
    IMMDevice           *_Endpoint;
    LPWSTR              _EndpointID;
    IAudioClient        *_AudioClient;
    IAudioCaptureClient *_CaptureClient;

    HANDLE              _CaptureThread;
    HANDLE              _ShutdownEvent;
    HANDLE              _AudioSamplesReadyEvent;
    WAVEFORMATEX        *_MixFormat;
    size_t              _FrameSize;
    UINT32              _BufferSize;

    boost::circular_buffer<std::vector<BYTE>> _CircularBuffer;
    static DWORD __stdcall WASAPICaptureThread(LPVOID Context);
    DWORD DoCaptureThread();
    //
    //  Stream switch related members and methods.
    //
    HANDLE                  _StreamSwitchDefaultEvent;          // Set when the current session is disconnected or the default device changes.
    HANDLE                  _StreamSwitchCompleteDefaultEvent;  // Set when the default device changed.
    HANDLE                  _StreamSwitchSelectedEvent;          // Set when the current session is disconnected or the default device changes.
    HANDLE                  _StreamSwitchCompleteSelectedEvent;  // Set when the default device changed.
    IAudioSessionControl    *_AudioSessionControl;
    IMMDeviceEnumerator     *_DeviceEnumerator;
    bool                    _InStreamSwitch;
    bool                    _DefaultSelected;

    bool InitializeStreamSwitch();
    void TerminateStreamSwitch();
    bool HandleStreamSwitchDefaultEvent();
    bool HandleStreamSwitchSelectedEvent();

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
    smpl_t              _BPM;
    fvec_t              *_FFTIn;
    cvec_t              *_FFTOut;
    fvec_t              *_TempoIn;
    fvec_t              *_TempoOut;
    aubio_fft_t         *_FFTObject;
    aubio_tempo_t       *_TempoObject;
    std::vector<double> _Mag;
    bool                InitializeAubio();

};


#endif // AUDIOSYSTEM_H
