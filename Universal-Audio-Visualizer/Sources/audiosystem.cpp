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
    _StreamSwitchDefaultEvent(nullptr),
    _StreamSwitchCompleteDefaultEvent(nullptr),
    _StreamSwitchSelectedEvent(nullptr),
    _StreamSwitchCompleteSelectedEvent(nullptr),
    _AudioSessionControl(nullptr),
    _DeviceEnumerator(nullptr),
    _InStreamSwitch(false),
    _CircularBuffer(CBBUFFERSIZE)
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

    for(int i = 0; i < CBBUFFERSIZE; i +=1)
    {
        _CircularBuffer[i]._IsSilent = false;
        _CircularBuffer[i]._CaptureBufferSize = _BufferSize * _FrameSize;
        _CircularBuffer[i]._CaptureBuffer = (BYTE *)realloc(_CircularBuffer[i]._CaptureBuffer, _BufferSize * _FrameSize);
        if(_CircularBuffer[i]._CaptureBuffer == nullptr)
        {
            printf("Unable to allocate memory in circular buffer: %x.\n", i);
            return false;
        }
        ZeroMemory(_CircularBuffer[i]._CaptureBuffer, _BufferSize * _FrameSize);
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
    _BufferSize * _FrameSize;
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
    _ShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
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

    //
    //  Create our stream switch event- we want auto reset events that start in the not-signaled state.
    //  Note that we create this event even if we're not going to stream switch - that's because the event is used
    //  in the main loop of the capturer and thus it has to be set.
    //
    _StreamSwitchDefaultEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_StreamSwitchDefaultEvent == nullptr)
    {
        printf("Unable to create default stream switch event: %d.\n", GetLastError());
        return false;
    }

    _StreamSwitchSelectedEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_StreamSwitchDefaultEvent == nullptr)
    {
        printf("Unable to create selected stream switch event: %d.\n", GetLastError());
        return false;
    }

    HRESULT hr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_DeviceEnumerator));
    if (FAILED(hr))
    {
        printf("Unable to instantiate device enumerator: %x\n", hr);
        return false;
    }

    hr = _deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &_Endpoint);
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
    if (InitializeAubio())
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
    if (_StreamSwitchDefaultEvent)
    {
        CloseHandle(_StreamSwitchDefaultEvent);
        _StreamSwitchDefaultEvent = nullptr;
    }
    if (_StreamSwitchSelectedEvent)
    {
        CloseHandle(_StreamSwitchSelectedEvent);
        _StreamSwitchSelectedEvent = nullptr;
    }

    SafeRelease(&_Endpoint);
    SafeRelease(&_AudioClient);
    SafeRelease(&_CaptureClient);

    for(UINT i = 0; i < CBBUFFERSIZE; i += 1)
    {
        _CircularBuffer[i]._IsSilent = true;
        _CircularBuffer[i]._CaptureBufferSize = -1;
        if(_CircularBuffer[i]._CaptureBuffer != nullptr)
        {
            free(_CircularBuffer[i]._CaptureBuffer);
            _CircularBuffer[i]._CaptureBuffer = nullptr;
        }
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
    HANDLE waitArray[4] = {_ShutdownEvent, _StreamSwitchDefaultEvent, _StreamSwitchSelectedEvent, _AudioSamplesReadyEvent };
    HANDLE mmcssHandle = NULL;
    DWORD mmcssTaskIndex = 0;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("Unable to initialize COM in render thread: %x\n", hr);
        return hr;
    }

    if (!DisableMMCSS)
    {
        mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
        if (mmcssHandle == NULL)
        {
            printf("Unable to enable MMCSS on capture thread: %d\n", GetLastError());
        }
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
        case WAIT_OBJECT_0 + 1:     // _StreamSwitchDefaultEvent
            //
            //  We need to stop the capturer, tear down the _AudioClient and _CaptureClient objects and re-create them on the new.
            //  endpoint if possible.  If this fails, abort the thread.
            //
            if (!HandleStreamSwitchDefaultEvent())
            {
                stillPlaying = false;
            }
            break;
        case WAIT_OBJECT_0 + 2:     // _StreamSwitchSelectedEvent
            //
            //  We need to stop the capturer, tear down the _AudioClient and _CaptureClient objects and re-create them on the new.
            //  endpoint if possible.  If this fails, abort the thread.
            //
            if (!HandleStreamSwitchSelectedEvent())
            {
                stillPlaying = false;
            }
            break;
        case WAIT_OBJECT_0 + 3:     // _AudioSamplesReadyEvent
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
                    _BufferElem temp;
                    temp._IsSilent = flags & AUDCLNT_BUFFERFLAGS_SILENT;
                    temp._CaptureBufferSize = framesAvailable * _FrameSize;
                    temp._CaptureBuffer = pData;
                    _CircularBuffer.push_back(temp);

                }
                hr = _CaptureClient->ReleaseBuffer(framesAvailable);
                if (FAILED(hr))
                {
                    printf("Unable to release capture buffer: %x!\n", hr);
                }
            }
            break;
        }
    }
    if (!DisableMMCSS)
    {
        AvRevertMmThreadCharacteristics(mmcssHandle);
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
    _StreamSwitchCompleteDefaultEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_StreamSwitchCompleteDefaultEvent == nullptr)
    {
        printf("Unable to create stream switch default complete event: %d.\n", GetLastError());
        return false;
    }

    //
    //  Create the stream switch complete event- we want a manual reset event that starts in the not-signaled state.
    //
    _StreamSwitchCompleteSelectedEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_StreamSwitchCompleteSelectedEvent == nullptr)
    {
        printf("Unable to create stream switch selected complete event: %d.\n", GetLastError());
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

    _DeviceEnumerator->UnregisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        printf("Unable to unregister for endpoint notifications: %x\n", hr);
    }

    if (_StreamSwitchCompleteDefaultEvent)
    {
        CloseHandle(_StreamSwitchCompleteDefaultEvent);
        _StreamSwitchCompleteDefaultEvent = nullptr;
    }
    if (_StreamSwitchCompleteSelectedEvent)
    {
        CloseHandle(_StreamSwitchCompleteDefaultEvent);
        _StreamSwitchCompleteDefaultEvent = nullptr;
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
bool AudioSystem::HandleStreamSwitchDefaultEvent()
{
    HRESULT hr;

    assert(_DefaultSelected);
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

    //
    //  Step 3.  Wait for the default device to change.
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
    waitResult = WaitForSingleObject(_StreamSwitchCompleteDefaultEvent, 500);
    if (waitResult == WAIT_TIMEOUT)
    {
        printf("Stream switch timeout - aborting...\n");
        goto ErrorExit;
    }

    //
    //  Step 4.  If we can't get the new endpoint, we need to abort the stream switch.  If there IS a new device,
    //          we should be able to retrieve it.
    //
    hr = _DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &_Endpoint);
    if (FAILED(hr))
    {
        printf("Unable to retrieve new default device during stream switch: %x\n", hr);
        goto ErrorExit;
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
    ResetEvent(_StreamSwitchCompleteDefaultEvent);
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
//  Handle the selected stream switch.
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
bool AudioSystem::HandleStreamSwitchSelectedEvent()
{
    HRESULT hr;

    assert(!_DefaultSelected);
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

    //
    //  Step 3.  Wait for the default device to change.
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
    waitResult = WaitForSingleObject(_StreamSwitchCompleteSelectedEvent, 500);
    if (waitResult == WAIT_TIMEOUT)
    {
        printf("Stream switch timeout - aborting...\n");
        goto ErrorExit;
    }

    //
    //  Step 4.  If we can't get the new endpoint, we need to abort the stream switch.  If there IS a new device,
    //          we should be able to retrieve it.
    //
    hr = _DeviceEnumerator->GetDevice(_EndpointID, &_Endpoint);
    if (FAILED(hr))
    {
        printf("Unable to retrieve new device during stream switch: %x\n", hr);
        goto ErrorExit;
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
    WAVEFORMATEX *wfxNew;
    hr = _AudioClient->GetMixFormat(&wfxNew);
    if (FAILED(hr))
    {
        printf("Unable to retrieve mix format for new audio client: %x.\n", hr);
        goto ErrorExit;
    }

    //
    //  Note that this is an intentionally naive comparison.  A more sophisticated comparison would
    //  compare the sample rate, channel count and format and apply the appropriate conversions into the capture pipeline.
    //
    if (memcmp(_MixFormat, wfxNew, sizeof(WAVEFORMATEX) + wfxNew->cbSize) != 0)
    {
        printf("New mix format doesn't match old mix format.  Aborting.\n");
        CoTaskMemFree(wfxNew);
        goto ErrorExit;
    }
    CoTaskMemFree(wfxNew);

    //
    //  Step 7:  Re-initialize the audio client.
    //
    if (!InitializeAudioEngine())
    {
        goto ErrorExit;
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
    ResetEvent(_StreamSwitchCompleteDefaultEvent);
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
    _DefaultSelected = false;
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
        SetEvent(_StreamSwitchDefaultEvent);
    }
    if (DisconnectReason == DisconnectReasonFormatChanged)
    {
        _InStreamSwitch = true;
        if(_DefaultSelected)
            {
            //
            //  The stream was disconnected because the format changed on our capture device.
            //
            //  We want to flag that we're in a stream switch and then set the stream switch event (which breaks out of the capturer).  We also
            //  want to set the _StreamSwitchCompleteEvent because we're not going to see a default device changed event after this.
            //
            SetEvent(_StreamSwitchDefaultEvent);
            SetEvent(_StreamSwitchCompleteDefaultEvent);
        }
        else
        {
            SetEvent(_StreamSwitchSelectedEvent);
            SetEvent(_StreamSwitchCompleteSelectedEvent);
        }
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
            SetEvent(_StreamSwitchDefaultEvent);
        }
        SetEvent(_StreamSwitchCompleteDefaultEvent);
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
         SetEvent(_StreamSwitchDefaultEvent);
     }
     SetEvent(_StreamSwitchCompleteDefaultEvent);
    return true;
 }


 bool AudioSystem::selectedEndpoint(LPWSTR input)
 {
    if (!_InStreamSwitch)
    {
        _InStreamSwitch = true;
        if(_EndpointID != nullptr)
        {
            if(wcscmp(input, _EndpointID) != 0)
            {
                if(_EndpointID)
                {
                    free(_EndpointID);
                }
                _EndpointID = _wcsdup(input);
            }
        }
        SetEvent(_StreamSwitchSelectedEvent);
        // no need to compare just copy in and call event
        // copy
        // call event

    }
    SetEvent(_StreamSwitchCompleteSelectedEvent);
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

AudioSystem::_ProcessedAudio AudioSystem::ProcessAudio()
{
    _ProcessedAudio data = _CircularBuffer.front();
    _CircularBuffer.pop_front();

    hannWindowFunction(data);

    for(int i = 0; i < FRAMECOUNT && !tempoQueue.empty(); i++, aubioIndex++)
    {
        fvec_set_sample(aubioIn, tempoQueue.front(), aubioIndex);
        tempoQueue.pop();
        if(aubioIndex == 511)
        {
            aubio_tempo_do(aubioTempo, aubioIn, aubioOut);
            if (aubioOut->data[0] != 0) {
                bpm = aubio_tempo_get_bpm(aubioTempo);
                #ifdef QT_DEBUG
                    qDebug() << "Realtime Tempo: " << aubio_tempo_get_bpm(aubioTempo) << " " << aubio_tempo_get_confidence(aubioTempo) << Qt::endl;
                #endif
                myTempo = (double) aubio_tempo_get_bpm(aubioTempo);
            }
            aubioIndex = -1;
        }
    }

    delete[] data;
    dataQueue.pop();

    //run FFT on data
    fftw_execute(p);

    //calculate log magnitude on transformed data
    for(int j = 0; j < FRAMECOUNT / 2; j++)
    {
        float r = out[j][0] / FRAMECOUNT;
        float i = out[j][1] / FRAMECOUNT;
        mag[j] = log(sqrt((r * r) + (i * i))) * 20;
    }
}

void AudioSystem::hannWindowFunction(_ProcessedAudio& in)
{
    switch(BytesPerSample())
    {
    case 1: // char / BYTES
    {
        for(int dataIndex = 0; dataIndex < in._CaptureBufferSize; dataIndex += 1)
        {
            //apply Hann window function to captured data
            double multiplier = 0.5 * (1 - cos(2 * M_PI * dataIndex) / (in._CaptureBufferSize - 1));
            in._CaptureBuffer[dataIndex] =  in._CaptureBuffer[dataIndex] * multiplier;
        }
        break;
    }
    case 2: // UINT_16
    {
        uint16_t* buffer = reinterpret_cast<uint16_t*>(in._CaptureBuffer);
        for(int dataIndex = 0; dataIndex < (in._CaptureBufferSize / 2); dataIndex += 1)
        {
            //apply Hann window function to captured data
            double multiplier = 0.5 * (1 - cos(2 * M_PI * dataIndex) / (in._CaptureBufferSize - 1));
            buffer[dataIndex] *= multiplier;
        }
        break;
    }
    case 3: // UINT24, pretty rare but common enough
    {
        for(int dataIndex = 0; dataIndex < (in._CaptureBufferSize / 3); dataIndex += 1)
        {
            //apply Hann window function to captured data
            int realIndex = dataIndex * 3;
            uint32_t num = 0;
            num = (in._CaptureBuffer[realIndex] & 0xFF) << 16;
            num = num + (in._CaptureBuffer[realIndex + 1] << 8);
            num = num + in._CaptureBuffer[realIndex + 2];
            double multiplier = 0.5 * (1 - cos(2 * M_PI * dataIndex) / (in._CaptureBufferSize - 1));

            in._CaptureBuffer[dataIndex] =  in._CaptureBuffer[dataIndex] * multiplier;
        }
        break;
    }
    case 4: // UINT_32 OMEGA RARE
    {
        uint32_t* buffer = reinterpret_cast<uint32_t*>(in._CaptureBuffer);
        for(int dataIndex = 0; dataIndex < (in._CaptureBufferSize / 4); dataIndex += 1)
        {
            //apply Hann window function to captured data
            double multiplier = 0.5 * (1 - cos(2 * M_PI * dataIndex) / (in._CaptureBufferSize - 1));
            buffer[dataIndex] *= multiplier;
        }
        break;
    }

    default:
        break;
    }
}
