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

const UINT CBBUFFERSIZE = 5;

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
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    bool selectedDefault();
    bool selectedEndpoint(LPWSTR input);

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

    //
    //  Capture buffer management.
    //
    struct _BufferElem {
        bool                    _IsSilent;
        BYTE                    *_CaptureBuffer = nullptr;
        size_t                  _CaptureBufferSize;
        _BufferElem& operator=(_BufferElem& in)
        {
            this->_CaptureBufferSize = in._CaptureBufferSize;
            this->_IsSilent = in._IsSilent;
            if(in._IsSilent)
            {
                ZeroMemory(this->_CaptureBuffer, in._CaptureBufferSize);
            }
            else
            {
                CopyMemory(this->_CaptureBuffer, in._CaptureBuffer, in._CaptureBufferSize);
            }
            return *this;
        }
    };
    boost::circular_buffer<_BufferElem> _CircularBuffer;
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
    aubio_fft_t         *_FFT;
    aubio_tempo_t       *_Tempo;
    fvec_t              *_AubioInput;
    fvec_t              *_TempoOutput;
    cvec_t              *_FFT_Output;
    bool                InitializeAubio();




public:
    struct _ProcessedAudio {
        BYTE                    *_CaptureBuffer = nullptr;
        size_t                  _CaptureBufferSize;
        /*
        _ProcessedAudio& operator=(_BufferElem& in)
        {
            this->_CaptureBufferSize = in._CaptureBufferSize;
            this->_CaptureBuffer = (BYTE *)realloc(this->_CaptureBuffer, in._CaptureBufferSize);
            if(in._IsSilent)
            {
                ZeroMemory(this->_CaptureBuffer, in._CaptureBufferSize);
            }
            else
            {
                CopyMemory(this->_CaptureBuffer, in._CaptureBuffer, in._CaptureBufferSize);
            }
            return *this;
        }
        */
        _ProcessedAudio(_BufferElem& in)
        {
            this->_CaptureBufferSize = in._CaptureBufferSize;
            this->_CaptureBuffer = (BYTE *)realloc(this->_CaptureBuffer, in._CaptureBufferSize);
            if(in._IsSilent)
            {
                ZeroMemory(this->_CaptureBuffer, in._CaptureBufferSize);
            }
            else
            {
                CopyMemory(this->_CaptureBuffer, in._CaptureBuffer, in._CaptureBufferSize);
            }
            return;
        }

        ~_ProcessedAudio()
        {
            if(this->_CaptureBuffer)
                free(this->_CaptureBuffer);
            return;
        }
    };

    _ProcessedAudio ProcessAudio();

private:
    void hannWindowFunction(_ProcessedAudio& in);
};


#endif // AUDIOSYSTEM_H
