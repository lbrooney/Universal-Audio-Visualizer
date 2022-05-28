#include "audiosystem.h"

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#include "string.h"
#include <assert.h>
#include <avrt.h>
#include <stdafx.h>
#include "audiosystem.h"
#include <math.h>
#include <QtDebug>

//
//  A simple WASAPI Capture client.
//

AudioSystem::AudioSystem() :
    _RefCount(1),
    _deviceEnumerator(nullptr),
    _Endpoint(nullptr),
    _AudioClient(nullptr),
    _CaptureClient(nullptr),
    _CaptureThread(nullptr),
    _ShutdownEvent(nullptr),
    _AudioSamplesReadyEvent(nullptr),
    _MixFormat(nullptr),
    _StreamSwitchEvent(nullptr),
    _StreamSwitchCompleteEvent(nullptr),
    _AudioSessionControl(nullptr),
    _DeviceEnumerator(nullptr),
    _InStreamSwitch(false),
    _BPM(0),
    _FFTIn(nullptr),
    _FFTOut(nullptr),
    _TempoIn(nullptr),
    _TempoOut(nullptr),
    _FFTObject(nullptr),
    _TempoObject(nullptr),
    _AnalysisSamplesReadyEvent(nullptr)
{
}

//
//  Empty destructor - everything should be released in the Shutdown() call.
//
AudioSystem::~AudioSystem(void)
{
}


//
//  Initialize WASAPI in event driven mode, associate the audio client with our samples ready event handle, retrieve
//  a capture client for the transport, create the capture thread and start the audio engine.
//
bool AudioSystem::InitializeAudioEngine()
{
    HRESULT hr = _AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 0, 0, _MixFormat, NULL);

    if (FAILED(hr))
    {
        printf("Unable to initialize audio client: %x.\n", hr);
        return false;
    }

    //
    //  Retrieve the buffer size for the audio client.
    //
    hr = _AudioClient->GetBufferSize(&_BufferSize);
    if(FAILED(hr))
    {
        printf("Unable to get audio client buffer: %x. \n", hr);
        return false;
    }

    hr = _AudioClient->SetEventHandle(_AudioSamplesReadyEvent);
    if (FAILED(hr))
    {
        printf("Unable to set ready event: %x.\n", hr);
        return false;
    }

    hr = _AudioClient->GetService(IID_PPV_ARGS(&_CaptureClient));
    if (FAILED(hr))
    {
        printf("Unable to get new capture client: %x.\n", hr);
        return false;
    }

    return true;
}

bool AudioSystem::InitializeAubio()
{
    _Mag.resize(FRAMECOUNT);
    _FFTIn = new_fvec(FRAMECOUNT);
    _FFTOut = new_cvec(FRAMECOUNT);
    _TempoIn = new_fvec(FRAMECOUNT);
    _TempoOut = new_fvec(2);
    _FFTObject = new_aubio_fft(FRAMECOUNT);
    _TempoObject = new_aubio_tempo("default", FRAMECOUNT, FRAMECOUNT / 4, SamplesPerSecond() );
    return true;
}

//
//  Retrieve the format we'll use to capture samples.
//
//  We use the Mix format since we're capturing in shared mode.
//
bool AudioSystem::LoadFormat()
{
    HRESULT hr = _AudioClient->GetMixFormat(&_MixFormat);
    if (FAILED(hr))
    {
        printf("Unable to get mix format on audio client: %x.\n", hr);
        return false;
    }

    _FrameSize = (_MixFormat->wBitsPerSample / 8) * _MixFormat->nChannels;
    return true;
}

//
//  Initialize the capturer.
//
bool AudioSystem::Initialize()
{
    //
    //  Create our shutdown and samples ready events- we want auto reset events that start in the not-signaled state.
    //
    _ShutdownEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_ShutdownEvent == nullptr)
    {
        printf("Unable to create shutdown event: %d.\n", GetLastError());
        return false;
    }

    _AudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_AudioSamplesReadyEvent == nullptr)
    {
        printf("Unable to create samples ready event: %d.\n", GetLastError());
        return false;
    }

    _AnalysisSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_AnalysisSamplesReadyEvent == nullptr)
    {
        printf("Unable to analysis ready event event: %d.\n", GetLastError());
        return false;
    }

    //
    //  Create our stream switch event- we want auto reset events that start in the not-signaled state.
    //  Note that we create this event even if we're not going to stream switch - that's because the event is used
    //  in the main loop of the capturer and thus it has to be set.
    //
    _StreamSwitchEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_StreamSwitchEvent == nullptr)
    {
        printf("Unable to create default stream switch event: %d.\n", GetLastError());
        return false;
    }

    HRESULT hr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&_DeviceEnumerator);
    if (FAILED(hr))
    {
        printf("Unable to instantiate device enumerator: %x\n", hr);
        return false;
    }

    hr = _DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &_Endpoint);
    if (FAILED(hr))
    {
        printf("Unable to get default audio endpoint: %x\n", hr);
        return false;
    }

    _DefaultSelected = true;

    //
    //  Now activate an IAudioClient object on our preferred endpoint and retrieve the mix format for that endpoint.
    //
    hr = _Endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&_AudioClient));
    if (FAILED(hr))
    {
        printf("Unable to activate audio client: %x.\n", hr);
        return false;
    }

    //
    // Load the MixFormat.  This may differ depending on the shared mode used
    //
    if (!LoadFormat())
    {
        printf("Failed to load the mix format \n");
        return false;
    }

    //
    //  Remember our configured latency in case we'll need it for a stream switch later.
    //

    if (!InitializeAudioEngine())
    {
        return false;
    }


    //
    // setup the aubio functionality
    //
    if (!InitializeAubio())
    {
        return false;
    }

    if (!InitializeStreamSwitch())
    {
        return false;
    }

    return true;
}

//
//  Shut down the capture code and free all the resources.
//
void AudioSystem::Shutdown()
{
    if (_CaptureThread)
    {
        SetEvent(_ShutdownEvent);
        WaitForSingleObject(_CaptureThread, INFINITE);
        CloseHandle(_CaptureThread);
        _CaptureThread = nullptr;
    }

    if (_AnalysisThread)
    {
        SetEvent(_ShutdownEvent);
        WaitForSingleObject(_AnalysisThread, INFINITE);
        CloseHandle(_AnalysisThread);
        _AnalysisThread = nullptr;
    }

    if (_ShutdownEvent)
    {
        CloseHandle(_ShutdownEvent);
        _ShutdownEvent = nullptr;
    }
    if (_AudioSamplesReadyEvent)
    {
        CloseHandle(_AudioSamplesReadyEvent);
        _AudioSamplesReadyEvent = nullptr;
    }
    if (_AnalysisSamplesReadyEvent)
    {
        CloseHandle(_AnalysisSamplesReadyEvent);
        _AnalysisSamplesReadyEvent = nullptr;
    }
    if (_StreamSwitchEvent)
    {
        CloseHandle(_StreamSwitchEvent);
        _StreamSwitchEvent = nullptr;
    }

    SafeRelease(&_Endpoint);
    SafeRelease(&_AudioClient);
    SafeRelease(&_CaptureClient);

    // aubio resources
    if(_FFTIn)
    {
        del_fvec(_FFTIn);
        _FFTIn = nullptr;
    }
    if(_FFTOut)
    {
        del_cvec(_FFTOut);
        _FFTOut = nullptr;
    }
    if(_TempoIn)
    {
        del_fvec(_TempoIn);
        _TempoIn = nullptr;
    }
    if(_TempoOut)
    {
        del_fvec(_TempoOut);
        _TempoOut = nullptr;
    }
    if(_FFTObject)
    {
        del_aubio_fft(_FFTObject);
        _FFTObject = nullptr;
    }
    if(_TempoObject)
    {
        del_aubio_tempo(_TempoObject);
        _TempoObject = nullptr;
    }

    if (_EndpointID)
    {
        free(_EndpointID);
        _EndpointID = nullptr;
    }

    if (_MixFormat)
    {
        CoTaskMemFree(_MixFormat);
        _MixFormat = nullptr;
    }

    TerminateStreamSwitch();
}


//
//  Start capturing...
//
bool AudioSystem::Start()
{
    HRESULT hr;

    //
    //  Now create the thread which is going to drive the capture.
    //
    _CaptureThread = CreateThread(NULL, 0, WASAPICaptureThread, this, 0, NULL);
    if (_CaptureThread == nullptr)
    {
        printf("Unable to create transport thread: %x.", GetLastError());
        return false;
    }

    _AnalysisThread = CreateThread(NULL, 0, AudioAnalysisThread, this, 0, NULL);
    if (_AnalysisThread == nullptr)
    {
        printf("Unable to create analysis thread: %x.", GetLastError());
        return false;
    }

    //
    //  We're ready to go, start capturing!
    //
    hr = _AudioClient->Start();
    if (FAILED(hr))
    {
        printf("Unable to start capture client: %x.\n", hr);
        return false;
    }

    return true;
}

//
//  Stop the capturer.
//
void AudioSystem::Stop()
{
    HRESULT hr;

    //
    //  Tell the capture thread to shut down, wait for the thread to complete then clean up all the stuff we
    //  allocated in Start().
    //
    if (_ShutdownEvent)
    {
        SetEvent(_ShutdownEvent);
    }

    hr = _AudioClient->Stop();
    if (FAILED(hr))
    {
        printf("Unable to stop audio client: %x\n", hr);
    }

    if (_CaptureThread)
    {
        WaitForSingleObject(_CaptureThread, INFINITE);

        CloseHandle(_CaptureThread);
        _CaptureThread = nullptr;
    }
}


//
//  Capture thread - processes samples from the audio engine
//
DWORD AudioSystem::WASAPICaptureThread(LPVOID Context)
{
    AudioSystem *capturer = static_cast<AudioSystem *>(Context);
    return capturer->DoCaptureThread();
}

DWORD AudioSystem::DoCaptureThread()
{
    bool stillPlaying = true;
    HANDLE waitArray[3] = {_ShutdownEvent, _StreamSwitchEvent, _AudioSamplesReadyEvent };
    HANDLE mmcssHandle = NULL;
    DWORD mmcssTaskIndex = 0;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("Unable to initialize COM in render thread: %x\n", hr);
        return hr;
    }

    while (stillPlaying)
    {
        //HRESULT hr;
        DWORD waitResult = WaitForMultipleObjects(3, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // _ShutdownEvent
            stillPlaying = false;       // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:     // _StreamSwitchEvent
            //
            //  We need to stop the capturer, tear down the _AudioClient and _CaptureClient objects and re-create them on the new.
            //  endpoint if possible.  If this fails, abort the thread.
            //
            if (!HandleStreamSwitchEvent())
            {
                stillPlaying = false;
            }
            break;
        case WAIT_OBJECT_0 + 2:     // _AudioSamplesReadyEvent
            //
            //  We need to retrieve the next buffer of samples from the audio capturer.
            //
            BYTE *pData;
            UINT32 framesAvailable;
            DWORD  flags;

            //
            //  Find out how much capture data is available.  We need to make sure we don't run over the length
            //  of our capture buffer.  We'll discard any samples that don't fit in the buffer.
            //
            hr = _CaptureClient->GetBuffer(&pData, &framesAvailable, &flags, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                if (framesAvailable != 0)
                {
                    //
                    //  The flags on capture tell us information about the data.
                    //
                    //  We only really care about the silent flag since we want to put frames of silence into the buffer
                    //  when we receive silence.  We rely on the fact that a logical bit 0 is silence for both float and int formats.
                    //
                    std::vector<BYTE> temp = std::vector<BYTE>( framesAvailable * _FrameSize );
                    if(!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
                    {
                        std::memcpy(temp.data(), pData, framesAvailable * _FrameSize);
                    }
                    _AudioQueue.push(temp);

                }
                hr = _CaptureClient->ReleaseBuffer(framesAvailable);
                if (FAILED(hr))
                {
                    printf("Unable to release capture buffer: %x!\n", hr);
                }
                SetEvent(_AnalysisSamplesReadyEvent);
            }
            break;
        }
    }

    CoUninitialize();
    return 0;
}


//
//  Initialize the stream switch logic.
//
bool AudioSystem::InitializeStreamSwitch()
{
    HRESULT hr = _AudioClient->GetService(IID_PPV_ARGS(&_AudioSessionControl));
    if (FAILED(hr))
    {
        printf("Unable to retrieve session control: %x\n", hr);
        return false;
    }

    //
    //  Create the stream switch complete event- we want a manual reset event that starts in the not-signaled state.
    //
    _StreamSwitchCompleteEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_StreamSwitchCompleteEvent == nullptr)
    {
        printf("Unable to create stream switch complete event: %d.\n", GetLastError());
        return false;
    }

    //
    //  Register for session and endpoint change notifications.
    //
    //  A stream switch is initiated when we receive a session disconnect notification or we receive a default device changed notification.
    //
    hr = _AudioSessionControl->RegisterAudioSessionNotification(this);
    if (FAILED(hr))
    {
        printf("Unable to register for stream switch notifications: %x\n", hr);
        return false;
    }

    hr = _DeviceEnumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        printf("Unable to register for stream switch notifications: %x\n", hr);
        return false;
    }

    return true;
}

void AudioSystem::TerminateStreamSwitch()
{
    HRESULT hr = _AudioSessionControl->UnregisterAudioSessionNotification(this);
    if (FAILED(hr))
    {
        printf("Unable to unregister for session notifications: %x\n", hr);
    }

    hr = _DeviceEnumerator->UnregisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        printf("Unable to unregister for endpoint notifications: %x\n", hr);
    }

    if (_StreamSwitchCompleteEvent)
    {
        CloseHandle(_StreamSwitchCompleteEvent);
        _StreamSwitchCompleteEvent = nullptr;
    }

    SafeRelease(&_AudioSessionControl);
    SafeRelease(&_DeviceEnumerator);
}

//
//  Handle the default stream switch.
//
//  When a stream switch happens, we want to do several things in turn:
//
//  1) Stop the current capturer.
//  2) Release any resources we have allocated (the _AudioClient, _AudioSessionControl (after unregistering for notifications) and
//        _CaptureClient).
//  3) Wait until the default device has changed (or 500ms has elapsed).  If we time out, we need to abort because the stream switch can't happen.
//  4) Retrieve the new default endpoint for our role.
//  5) Re-instantiate the audio client on that new endpoint.
//  6) Retrieve the mix format for the new endpoint.  If the mix format doesn't match the old endpoint's mix format, we need to abort because the stream
//      switch can't happen.
//  7) Re-initialize the _AudioClient.
//  8) Re-register for session disconnect notifications and reset the stream switch complete event.
//
bool AudioSystem::HandleStreamSwitchEvent()
{
    HRESULT hr;

    assert(_InStreamSwitch);
    //
    //  Step 1.  Stop capturing.
    //
    hr = _AudioClient->Stop();
    DWORD waitResult;
    if (FAILED(hr))
    {
        printf("Unable to stop audio client during stream switch: %x\n", hr);
        goto ErrorExit;
    }

    //
    //  Step 2.  Release our resources.  Note that we don't release the mix format, we need it for step 6.
    //
    hr = _AudioSessionControl->UnregisterAudioSessionNotification(this);
    if (FAILED(hr))
    {
        printf("Unable to stop audio client during stream switch: %x\n", hr);
        goto ErrorExit;
    }

    SafeRelease(&_AudioSessionControl);
    SafeRelease(&_CaptureClient);
    SafeRelease(&_AudioClient);
    SafeRelease(&_Endpoint);
    /*
    // free our buffers in circular buffer, they are remade in initialize engine
    for(int i = 0; i < CBBUFFERSIZE; i += 1)
    {
        if(_CircularBuffer[i]._CaptureBuffer)
            free(_CircularBuffer[i]._CaptureBuffer);
        _CircularBuffer[i]._Size = 0;
    }
    */

    //
    //  Step 3.  Wait for the default device / new device to change.
    //
    //  There is a race between the session disconnect arriving and the new default device
    //  arriving (if applicable).  Wait the shorter of 500 milliseconds or the arrival of the
    //  new default device, then attempt to switch to the default device.  In the case of a
    //  format change (i.e. the default device does not change), we artificially generate  a
    //  new default device notification so the code will not needlessly wait 500ms before
    //  re-opening on the new format.  (However, note below in step 6 that in this SDK
    //  sample, we are unlikely to actually successfully absorb a format change, but a
    //  real audio application implementing stream switching would re-format their
    //  pipeline to deliver the new format).
    //
    waitResult = WaitForSingleObject(_StreamSwitchCompleteEvent, 500);
    if (waitResult == WAIT_TIMEOUT)
    {
        printf("Stream switch timeout - aborting...\n");
        goto ErrorExit;
    }

    //
    //  Step 4.  If we can't get the new endpoint, we need to abort the stream switch.  If there IS a new device,
    //          we should be able to retrieve it.
    //
    if(_DefaultSelected)
    {
        hr = _DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &_Endpoint);
        if (FAILED(hr))
        {
            printf("Unable to retrieve new default device during stream switch: %x\n", hr);
            goto ErrorExit;
        }
    }
    else
    {
        hr = _DeviceEnumerator->GetDevice(_EndpointID, &_Endpoint);
        if (FAILED(hr))
        {
            printf("Unable to retrieve new device during stream switch: %x\n", hr);
            goto ErrorExit;
        }
    }
    //
    //  Step 5 - Re-instantiate the audio client on the new endpoint.
    //
    hr = _Endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&_AudioClient));
    if (FAILED(hr))
    {
        printf("Unable to activate audio client on the new endpoint: %x.\n", hr);
        goto ErrorExit;
    }
    //
    //  Step 6 - Retrieve the new mix format.
    //
    CoTaskMemFree(_MixFormat);
    hr = _AudioClient->GetMixFormat(&_MixFormat);
    if (FAILED(hr))
    {
        printf("Unable to retrieve mix format for new audio client: %x.\n", hr);
        goto ErrorExit;
    }

    _FrameSize = (_MixFormat->wBitsPerSample / 8) * _MixFormat->nChannels;



    //
    //  Step 7:  Re-initialize the audio client.
    //
    if (!InitializeAudioEngine())
    {
        goto ErrorExit;
    }

    if (InitializeAubio())
    {
        return false;
    }

    //
    //  Step 8: Re-register for session disconnect notifications.
    //
    hr = _AudioClient->GetService(IID_PPV_ARGS(&_AudioSessionControl));
    if (FAILED(hr))
    {
        printf("Unable to retrieve session control on new audio client: %x\n", hr);
        goto ErrorExit;
    }
    hr = _AudioSessionControl->RegisterAudioSessionNotification(this);
    if (FAILED(hr))
    {
        printf("Unable to retrieve session control on new audio client: %x\n", hr);
        goto ErrorExit;
    }

    //
    //  Reset the stream switch complete event because it's a manual reset event.
    //
    ResetEvent(_StreamSwitchCompleteEvent);
    //
    //  And we're done.  Start capturing again.
    //
    hr = _AudioClient->Start();
    if (FAILED(hr))
    {
        printf("Unable to start the new audio client: %x\n", hr);
        goto ErrorExit;
    }

    _InStreamSwitch = false;
    _DefaultSelected = true;
    return true;

ErrorExit:
    _InStreamSwitch = false;
    return false;
}

//
//  Called when an audio session is disconnected.
//
//  When a session is disconnected because of a device removal or format change event, we just want
//  to let the capture thread know that the session's gone away
//
__attribute__((nothrow)) HRESULT AudioSystem::OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason)
{
   if (DisconnectReason == DisconnectReasonDeviceRemoval) // if device removed go to default device
   {
       //
       //  The stream was disconnected because the device we're capturing to was removed.
       //
       //  We want to reset the stream switch complete event (so we'll block when the HandleStreamSwitchEvent function
       //  waits until the default device changed event occurs).
       //
       //  Note that we don't set the _StreamSwitchCompleteEvent - that will be set when the OnDefaultDeviceChanged event occurs.
       //
       _InStreamSwitch = true;
       SetEvent(_StreamSwitchEvent);
   }
   if (DisconnectReason == DisconnectReasonFormatChanged)
   {
       _InStreamSwitch = true;
       //
       //  The stream was disconnected because the format changed on our capture device.
       //
       //  We want to flag that we're in a stream switch and then set the stream switch event (which breaks out of the capturer).  We also
       //  want to set the _StreamSwitchCompleteEvent because we're not going to see a default device changed event after this.
       //
       SetEvent(_StreamSwitchEvent);
       SetEvent(_StreamSwitchCompleteEvent);
   }
   return S_OK;
}
//
//  Called when the default capture device changed.  We just want to set an event which lets the stream switch logic know that it's ok to
//  continue with the stream switch.
//
__attribute__((nothrow)) HRESULT AudioSystem::OnDefaultDeviceChanged(EDataFlow Flow, ERole Role, LPCWSTR /*NewDefaultDeviceId*/)
{
   if (Flow == eRender && Role == eConsole && _DefaultSelected)
   {
       //
       //  The default capture device for our configured role was changed.
       //
       //  If we're not in a stream switch already, we want to initiate a stream switch event.
       //  We also we want to set the stream switch complete event.  That will signal the capture thread that it's ok to re-initialize the
       //  audio capturer.
       //
       if (!_InStreamSwitch)
       {
           _InStreamSwitch = true;
           SetEvent(_StreamSwitchEvent);
       }
       SetEvent(_StreamSwitchCompleteEvent);
   }
   return S_OK;
}

bool AudioSystem::selectedDefault()
{
/*
    if(_DefaultSelected)
    {
        return false;
    }
*/
   if (!_InStreamSwitch)
   {
    _InStreamSwitch = true;
    _DefaultSelected = true;
    SetEvent(_StreamSwitchEvent);
   }
   SetEvent(_StreamSwitchCompleteEvent);
   return true;
}


bool AudioSystem::selectedEndpoint(LPWSTR input)
{
   if (!_InStreamSwitch)
   {
       _InStreamSwitch = true;
       _DefaultSelected = false;
       if(_EndpointID)
       {
           if(wcscmp(input, _EndpointID)) // if they are different then free and swap!
           {
               free(_EndpointID);
               _EndpointID = _wcsdup(input);
           }
       }
       else
       {
           _EndpointID = _wcsdup(input);
       }
       SetEvent(_StreamSwitchEvent);
       // no need to compare just copy in and call event
       // copy
       // call event

   }
   SetEvent(_StreamSwitchCompleteEvent);
   return true;
}

//
//  IUnknown
//
 __attribute__((nothrow)) HRESULT AudioSystem::QueryInterface(REFIID Iid, void **Object)
{
    if (Object == NULL)
    {
        return E_POINTER;
    }
    *Object = NULL;

    if (Iid == IID_IUnknown)
    {
        *Object = static_cast<IUnknown *>(static_cast<IAudioSessionEvents *>(this));
        AddRef();
    }
    else if (Iid == __uuidof(IMMNotificationClient))
    {
        *Object = static_cast<IMMNotificationClient *>(this);
        AddRef();
    }
    else if (Iid == __uuidof(IAudioSessionEvents))
    {
        *Object = static_cast<IAudioSessionEvents *>(this);
        AddRef();
    }
    else
    {
        return E_NOINTERFACE;
    }
    return S_OK;
}

 __attribute__((nothrow)) ULONG AudioSystem::AddRef()
{
    return InterlockedIncrement(&_RefCount);
}

 __attribute__((nothrow)) ULONG AudioSystem::Release()
{
    ULONG returnValue = InterlockedDecrement(&_RefCount);
    if (returnValue == 0)
    {
        delete this;
    }
    return returnValue;
}

 //
 //  Capture thread - processes samples from the audio engine
 //
DWORD AudioSystem::AudioAnalysisThread(LPVOID Context)
{
    AudioSystem *analyzer = static_cast<AudioSystem *>(Context);
    return analyzer->DoAnalysisThread();
}

DWORD AudioSystem::DoAnalysisThread()
{
    bool stillPlaying = true;
    HANDLE waitArray[2] = {_ShutdownEvent, _AnalysisSamplesReadyEvent };

    while (stillPlaying)
    {
    //HRESULT hr;
    DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
            case WAIT_OBJECT_0 + 0:     // _ShutdownEvent
            stillPlaying = false;       // We're done, exit the loop.
            break;
            case WAIT_OBJECT_0 + 1:     // if a sample is available then work with it
                AnalyzeAudio();
            break;
        }
    }

    return 0;
}

void AudioSystem::AnalyzeAudio()
{
    // have fvec_t of size buffer
    // set fft input to 0
    fvec_zeros(_FFTIn);
    fvec_zeros(_TempoIn);
    if(!_AudioQueue.empty())
    {
        std::vector<BYTE> data = std::vector<BYTE>(_AudioQueue.front());
        _AudioQueue.pop();
        uint_t wrapAt = (1 << ( _MixFormat->wBitsPerSample - 1 ) );
        uint_t wrapWith = (1 << _MixFormat->wBitsPerSample);
        smpl_t scaler = 1. / wrapAt;
        int i = 0;
        int j = 0;
        while(i < data.size() && j < FRAMECOUNT)

        //for (int i, j = 0; i < data.size() && j < FRAMECOUNT; j += 1)
        {

            for (int k = 0; k < ChannelCount(); k += 1)
            {
                uint32_t unsignedVal = 0;
                for (int b = 0; b < _MixFormat->wBitsPerSample; b+=8 )
                {
                    unsignedVal += data.at(i) << b;
                    i += 1;
                }
                int32_t signedVal = unsignedVal;
                // FIXME why does 8 bit conversion maps [0;255] to [-128;127]
                // instead of [0;127] to [0;127] and [128;255] to [-128;-1]
                if (_MixFormat->wBitsPerSample == 8) signedVal -= wrapAt;
                else if (unsignedVal >= wrapAt) signedVal = unsignedVal - wrapWith;
                _FFTIn->data[j] += (smpl_t)(signedVal * scaler); // want to include both signed ints in this
                _TempoIn->data[j] += (smpl_t)(signedVal * scaler);
                /*
                qDebug() << "i " << i << " j " << j <<" k " << k
                          <<" signed value "<< signedVal
                         <<" inputed value " << _FFTIn->data[j]<< Qt::endl;
                         */
            }
            _FFTIn->data[j] /= (smpl_t)ChannelCount();
            _TempoIn->data[j] /= (smpl_t)ChannelCount();
            /*
            qDebug() << "i " << i << " j " << j
                     <<" inputed value " << _FFTIn->data[j]<< Qt::endl;
                     */
            /* HANN WINDOW FUNCTION
            double multiplier = 0.5 * (1 - cos(2 * M_PI * j) / (_BufferSize - 1));
            _FFTIn->data[j] *= multiplier;
            _TempoIn->data[j] *= multiplier;
            */
            j += 1;
        }
    }
    aubio_fft_do(_FFTObject, _FFTIn, _FFTOut);
    aubio_tempo_do(_TempoObject, _TempoIn, _TempoOut);

    _BPM = aubio_tempo_get_bpm(_TempoObject);
    _Mag.resize(_FFTOut->length, 0);
    for(int j = 0; j < _FFTOut->length; j+=1)
    {
        float r = _FFTOut->norm[j] * cos(_FFTOut->phas[j]);
        float i = _FFTOut->norm[j] * sin(_FFTOut->phas[j]);;
        _Mag.at(j) = log(sqrt((r * r) + (i * i))) * 10;
    }
    return;
}


smpl_t AudioSystem::GetBPM()
{
    if(_InStreamSwitch)
        return 0;
    return _BPM;
}

std::vector<double>& AudioSystem::GetMag()
{
    return _Mag;
}

smpl_t AudioSystem::GetBeatPeriod()
{
    return aubio_tempo_get_period_s(_TempoObject);
}
