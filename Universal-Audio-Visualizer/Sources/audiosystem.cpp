#include "audiosystem.h"

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#include "aubio/mathutils.h"
#include "string.h"
#include <assert.h>
#include <avrt.h>
#include <iostream>
#include <stdafx.h>
#include "audiosystem.h"
#include <math.h>
#include <QtDebug>

const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

//
//  A simple WASAPI Capture client.
//

AudioSystem::AudioSystem() :
    refCount(1),
    deviceEnumerator(nullptr),
    endpoint(nullptr),
    audioClient(nullptr),
    captureClient(nullptr),
    endpointVolume(nullptr),
    captureThread(nullptr),
    shutdownEvent(nullptr),
    audioSamplesReadyEvent(nullptr),
    mixFormat(nullptr),
    streamSwitchEvent(nullptr),
    streamSwitchCompleteEvent(nullptr),
    audioSessionControl(nullptr),
    inStreamSwitch(false),
    endpointID(nullptr),
    bpm(0),
    fftIn(nullptr),
    fftOut(nullptr),
    tempoIn(nullptr),
    tempoOut(nullptr),
    fftObject(nullptr),
    tempoObject(nullptr),
    analysisSamplesReadyEvent(nullptr)
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
    HRESULT hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 0, 0, mixFormat, NULL);

    if(FAILED(hr))
    {
        printf("Unable to initialize audio client: %x.\n", hr);
        return false;
    }

    //
    //  Retrieve the buffer size for the audio client.
    //
    hr = audioClient->GetBufferSize(&bufferSize);
    if(FAILED(hr))
    {
        printf("Unable to get audio client buffer: %x. \n", hr);
        return false;
    }

    //qDebug() << "BUFFER SIZE " << _BufferSize << " FRAMESIZE " << _FrameSize << " CHANNEL COUNT" << ChannelCount() << " BitsPerSample" << _MixFormat->wBitsPerSample << Qt::endl;

    hr = audioClient->SetEventHandle(audioSamplesReadyEvent);
    if(FAILED(hr))
    {
        printf("Unable to set ready event: %x.\n", hr);
        return false;
    }

    hr = audioClient->GetService(IID_PPV_ARGS(&captureClient));
    if(FAILED(hr))
    {
        printf("Unable to get new capture client: %x.\n", hr);
        return false;
    }

    return true;
}

bool AudioSystem::InitializeAubio()
{
    // Potenitally lose a little bit of data if _BufferSize > FRAMECOUNT(1024 ATM)
    // but if _BufferSize used then the program just crashes
    mag.resize(FRAMECOUNT / 2);
    fftIn = new_fvec(FRAMECOUNT);
    fftOut = new_cvec(FRAMECOUNT);
    tempoIn = new_fvec(FRAMECOUNT / 2);
    tempoOut = new_fvec(2);
    fftObject = new_aubio_fft(FRAMECOUNT);
    tempoObject = new_aubio_tempo("default", FRAMECOUNT, FRAMECOUNT / 2, SamplesPerSecond() );
    return true;
}

//
//  Retrieve the format we'll use to capture samples.
//
//  We use the Mix format since we're capturing in shared mode.
//
bool AudioSystem::LoadFormat()
{
    HRESULT hr = audioClient->GetMixFormat(&mixFormat);
    if(FAILED(hr))
    {
        printf("Unable to get mix format on audio client: %x.\n", hr);
        return false;
    }

    frameSize = (mixFormat->wBitsPerSample / 8) * mixFormat->nChannels;

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
    shutdownEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if(shutdownEvent == nullptr)
    {
        printf("Unable to create shutdown event: %d.\n", GetLastError());
        return false;
    }

    audioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if(audioSamplesReadyEvent == nullptr)
    {
        printf("Unable to create samples ready event: %d.\n", GetLastError());
        return false;
    }

    analysisSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if(analysisSamplesReadyEvent == nullptr)
    {
        printf("Unable to analysis ready event event: %d.\n", GetLastError());
        return false;
    }

    //
    //  Create our stream switch event- we want auto reset events that start in the not-signaled state.
    //  Note that we create this event even if we're not going to stream switch - that's because the event is used
    //  in the main loop of the capturer and thus it has to be set.
    //
    streamSwitchEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if(streamSwitchEvent == nullptr)
    {
        printf("Unable to create default stream switch event: %d.\n", GetLastError());
        return false;
    }

    HRESULT hr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if(FAILED(hr))
    {
        printf("Unable to instantiate device enumerator: %x\n", hr);
        return false;
    }

    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &endpoint);
    if(FAILED(hr))
    {
        printf("Unable to get default audio endpoint: %x\n", hr);
        return false;
    }

    defaultSelected = true;

    //
    //  Now activate an IAudioClient object on our preferred endpoint and retrieve the mix format for that endpoint.
    //
    hr = endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&audioClient));
    if(FAILED(hr))
    {
        printf("Unable to activate audio client: %x.\n", hr);
        return false;
    }

    //
    //  Activate the endpoint volume on the new endpoint.
    //
    hr = endpoint->Activate(IID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&endpointVolume));
    if(FAILED(hr))
    {
        printf("Unable to activate endpoint volume: %x.\n", hr);
        return false;
    }

    //
    // Load the MixFormat.  This may differ depending on the shared mode used
    //
    if(!LoadFormat())
    {
        printf("Failed to load the mix format \n");
        return false;
    }

    //
    //  Remember our configured latency in case we'll need it for a stream switch later.
    //

    if(!InitializeAudioEngine())
    {
        return false;
    }


    //
    // setup the aubio functionality
    //
    if(!InitializeAubio())
    {
        return false;
    }

    if(!InitializeStreamSwitch())
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
    if(captureThread != nullptr)
    {
        SetEvent(shutdownEvent);
        WaitForSingleObject(captureThread, INFINITE);
        CloseHandle(captureThread);
        captureThread = nullptr;
    }

    if(analysisThread != nullptr)
    {
        SetEvent(shutdownEvent);
        WaitForSingleObject(analysisThread, INFINITE);
        CloseHandle(analysisThread);
        analysisThread = nullptr;
    }

    if(shutdownEvent != nullptr)
    {
        CloseHandle(shutdownEvent);
        shutdownEvent = nullptr;
    }
    if(audioSamplesReadyEvent != nullptr)
    {
        CloseHandle(audioSamplesReadyEvent);
        audioSamplesReadyEvent = nullptr;
    }
    if(analysisSamplesReadyEvent != nullptr)
    {
        CloseHandle(analysisSamplesReadyEvent);
        analysisSamplesReadyEvent = nullptr;
    }
    if(streamSwitchEvent != nullptr)
    {
        CloseHandle(streamSwitchEvent);
        streamSwitchEvent = nullptr;
    }

    SafeRelease(&endpoint);
    SafeRelease(&endpointVolume);
    SafeRelease(&audioClient);
    SafeRelease(&captureClient);

    // aubio resources
    if(fftIn != nullptr)
    {
        del_fvec(fftIn);
        fftIn = nullptr;
    }
    if(fftOut != nullptr)
    {
        del_cvec(fftOut);
        fftOut = nullptr;
    }
    if(tempoIn != nullptr)
    {
        del_fvec(tempoIn);
        tempoIn = nullptr;
    }
    if(tempoOut != nullptr)
    {
        del_fvec(tempoOut);
        tempoOut = nullptr;
    }
    if(fftObject != nullptr)
    {
        del_aubio_fft(fftObject);
        fftObject = nullptr;
    }
    if(tempoObject != nullptr)
    {
        del_aubio_tempo(tempoObject);
        tempoObject = nullptr;
    }

    if(endpointID != nullptr)
    {
        free(endpointID);
        endpointID = nullptr;
    }

    if(mixFormat != nullptr)
    {
        CoTaskMemFree(mixFormat);
        mixFormat = nullptr;
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
    captureThread = CreateThread(NULL, 0, WASAPICaptureThread, this, 0, NULL);
    if(captureThread == nullptr)
    {
        printf("Unable to create transport thread: %x.", GetLastError());
        return false;
    }

    analysisThread = CreateThread(NULL, 0, AudioAnalysisThread, this, 0, NULL);
    if(analysisThread == nullptr)
    {
        printf("Unable to create analysis thread: %x.", GetLastError());
        return false;
    }

    //
    //  We're ready to go, start capturing!
    //
    hr = audioClient->Start();
    if(FAILED(hr))
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
    if(shutdownEvent)
    {
        SetEvent(shutdownEvent);
    }

    hr = audioClient->Stop();
    if(FAILED(hr))
    {
        printf("Unable to stop audio client: %x\n", hr);
    }

    if(captureThread)
    {
        WaitForSingleObject(captureThread, INFINITE);

        CloseHandle(captureThread);
        captureThread = nullptr;
    }

    if(analysisThread)
    {
        WaitForSingleObject(analysisThread, INFINITE);

        CloseHandle(analysisThread);
        analysisThread = nullptr;
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
    HANDLE waitArray[3] = {shutdownEvent, streamSwitchEvent, audioSamplesReadyEvent };

    UINT32 framesAvailable;
    UINT32 packetLength = 0;
    BYTE* pData;
    DWORD flags = AUDCLNT_BUFFERFLAGS_SILENT;
    int frameCounter = 0;
        float* byteArray = new float[FRAMECOUNT];

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if(FAILED(hr))
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
            if(!HandleStreamSwitchEvent())
            {
                stillPlaying = false;
            }
            break;
        case WAIT_OBJECT_0 + 2:     // _AudioSamplesReadyEvent
            captureClient->GetNextPacketSize(&packetLength);
            while (packetLength != 0 && stillPlaying)
            {
                captureClient->GetBuffer(&pData, &framesAvailable, &flags, NULL, NULL);
                if(flags & AUDCLNT_BUFFERFLAGS_SILENT)
                {
                    pData = NULL;  // Tell CopyData to write silence.
                }
                else
                {
                    //copy data the in buffer
                    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pData);
                    for(int i = 0; i < framesAvailable; i++, frameCounter++)
                    {
                        if(pData != nullptr)
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
                            SetEvent(analysisSamplesReadyEvent);
                            byteArray = new float[FRAMECOUNT];
                        }

                        ptr += sizeof(float);
                    }
                }

                captureClient->ReleaseBuffer(framesAvailable);
                captureClient->GetNextPacketSize(&packetLength);
            }
            //if no audio is playing, fill data buffers with 0
            if(flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                while(frameCounter < FRAMECOUNT )
                {
                    byteArray[frameCounter] = 0;
                    frameCounter++;
                }
                frameCounter = 0;
                dataQueue.push(byteArray);
                SetEvent(analysisSamplesReadyEvent);
                byteArray = new float[FRAMECOUNT];
                Sleep(25);
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
    HRESULT hr = audioClient->GetService(IID_PPV_ARGS(&audioSessionControl));
    if(FAILED(hr))
    {
        printf("Unable to retrieve session control: %x\n", hr);
        return false;
    }

    //
    //  Create the stream switch complete event- we want a manual reset event that starts in the not-signaled state.
    //
    streamSwitchCompleteEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if(streamSwitchCompleteEvent == nullptr)
    {
        printf("Unable to create stream switch complete event: %d.\n", GetLastError());
        return false;
    }

    //
    //  Register for session and endpoint change notifications.
    //
    //  A stream switch is initiated when we receive a session disconnect notification or we receive a default device changed notification.
    //
    hr = audioSessionControl->RegisterAudioSessionNotification(this);
    if(FAILED(hr))
    {
        printf("Unable to register for stream switch notifications: %x\n", hr);
        return false;
    }

    hr = deviceEnumerator->RegisterEndpointNotificationCallback(this);
    if(FAILED(hr))
    {
        printf("Unable to register for stream switch notifications: %x\n", hr);
        return false;
    }

    return true;
}

void AudioSystem::TerminateStreamSwitch()
{
    HRESULT hr = audioSessionControl->UnregisterAudioSessionNotification(this);
    if(FAILED(hr))
    {
        printf("Unable to unregister for session notifications: %x\n", hr);
    }

    hr = deviceEnumerator->UnregisterEndpointNotificationCallback(this);
    if(FAILED(hr))
    {
        printf("Unable to unregister for endpoint notifications: %x\n", hr);
    }

    if(streamSwitchCompleteEvent  != nullptr)
    {
        CloseHandle(streamSwitchCompleteEvent);
        streamSwitchCompleteEvent = nullptr;
    }

    SafeRelease(&audioSessionControl);
    SafeRelease(&deviceEnumerator);
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

    assert(inStreamSwitch);
    //
    //  Step 1.  Stop capturing.
    //
    hr = audioClient->Stop();
    DWORD waitResult;
    if(FAILED(hr))
    {
        printf("Unable to stop audio client during stream switch: %x\n", hr);
        goto ErrorExit;
    }

    //
    //  Step 2.  Release our resources.  Note that we don't release the mix format, we need it for step 6.
    //
    hr = audioSessionControl->UnregisterAudioSessionNotification(this);
    if(FAILED(hr))
    {
        printf("Unable to stop audio client during stream switch: %x\n", hr);
        goto ErrorExit;
    }

    SafeRelease(&audioSessionControl);
    SafeRelease(&captureClient);
    SafeRelease(&audioClient);
    SafeRelease(&endpointVolume);
    SafeRelease(&endpoint);

    if(tempoObject)
    {
        del_aubio_tempo(tempoObject);
        tempoObject = nullptr;
    }

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
    waitResult = WaitForSingleObject(streamSwitchCompleteEvent, 500);
    if(waitResult == WAIT_TIMEOUT)
    {
        printf("Stream switch timeout - aborting...\n");
        goto ErrorExit;
    }

    //
    //  Step 4.  If we can't get the new endpoint, we need to abort the stream switch.  If there IS a new device,
    //          we should be able to retrieve it.
    //
    if(defaultSelected)
    {
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &endpoint);
        if(FAILED(hr))
        {
            printf("Unable to retrieve new default device during stream switch: %x\n", hr);
            goto ErrorExit;
        }
    }
    else
    {
        hr = deviceEnumerator->GetDevice(endpointID, &endpoint);
        if(FAILED(hr))
        {
            printf("Unable to retrieve new device during stream switch: %x\n", hr);
            goto ErrorExit;
        }
    }
    //
    //  Step 5 - Re-instantiate the audio client on the new endpoint.
    //
    hr = endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&audioClient));
    if(FAILED(hr))
    {
        printf("Unable to activate audio client on the new endpoint: %x.\n", hr);
        goto ErrorExit;
    }
    //
    //  Re-instantiate the endpoint volume on the new endpoint.
    //
    hr = endpoint->Activate(IID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&endpointVolume));
    if(FAILED(hr))
    {
        printf("Unable to activate endpoint volume on the new endpoint: %x.\n", hr);
        goto ErrorExit;
    }
    //
    //  Step 6 - Retrieve the new mix format.
    //
    CoTaskMemFree(mixFormat);
    hr = audioClient->GetMixFormat(&mixFormat);
    if(FAILED(hr))
    {
        printf("Unable to retrieve mix format for new audio client: %x.\n", hr);
        goto ErrorExit;
    }

    frameSize = (mixFormat->wBitsPerSample / 8) * mixFormat->nChannels;



    //
    //  Step 7:  Re-initialize the audio client.
    //
    if(!InitializeAudioEngine())
    {
        goto ErrorExit;
    }

    /*qDebug() << "BUFFER SIZE " << _BufferSize << " FRAMESIZE "
             << _FrameSize << " CHANNEL COUNT"
             << ChannelCount() << " BitsPerSample"
             << _MixFormat->wBitsPerSample
             << " SAMPS PER SEC " << SamplesPerSecond() << Qt::endl;*/

    tempoObject = new_aubio_tempo("default", FRAMECOUNT, FRAMECOUNT / 2, SamplesPerSecond() );

    /*
    if(!InitializeAubio())
    {
        return false;
    }
    */

    //
    //  Step 8: Re-register for session disconnect notifications.
    //
    hr = audioClient->GetService(IID_PPV_ARGS(&audioSessionControl));
    if(FAILED(hr))
    {
        printf("Unable to retrieve session control on new audio client: %x\n", hr);
        goto ErrorExit;
    }
    hr = audioSessionControl->RegisterAudioSessionNotification(this);
    if(FAILED(hr))
    {
        printf("Unable to retrieve session control on new audio client: %x\n", hr);
        goto ErrorExit;
    }

    //
    //  Reset the stream switch complete event because it's a manual reset event.
    //
    ResetEvent(streamSwitchCompleteEvent);
    //
    //  And we're done.  Start capturing again.
    //
    hr = audioClient->Start();
    if(FAILED(hr))
    {
        printf("Unable to start the new audio client: %x\n", hr);
        goto ErrorExit;
    }

    inStreamSwitch = false;
    return true;

ErrorExit:
    inStreamSwitch = false;
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
    if(DisconnectReason == DisconnectReasonDeviceRemoval) // if device removed go to default device
    {
        //
        //  The stream was disconnected because the device we're capturing to was removed.
        //
        //  We want to reset the stream switch complete event (so we'll block when the HandleStreamSwitchEvent function
        //  waits until the default device changed event occurs).
        //
        //  Note that we don't set the _StreamSwitchCompleteEvent - that will be set when the OnDefaultDeviceChanged event occurs.
        //
        inStreamSwitch = true;
        SetEvent(streamSwitchEvent);
    }
    if(DisconnectReason == DisconnectReasonFormatChanged)
    {
        inStreamSwitch = true;
        //
        //  The stream was disconnected because the format changed on our capture device.
        //
        //  We want to flag that we're in a stream switch and then set the stream switch event (which breaks out of the capturer).  We also
        //  want to set the _StreamSwitchCompleteEvent because we're not going to see a default device changed event after this.
        //
        SetEvent(streamSwitchEvent);
        SetEvent(streamSwitchCompleteEvent);
    }
    return S_OK;
}
//
//  Called when the default capture device changed.  We just want to set an event which lets the stream switch logic know that it's ok to
//  continue with the stream switch.
//
__attribute__((nothrow)) HRESULT AudioSystem::OnDefaultDeviceChanged(EDataFlow Flow, ERole Role, LPCWSTR /*NewDefaultDeviceId*/)
{
    if(Flow == eRender && Role == eConsole && defaultSelected)
    {
        //
        //  The default capture device for our configured role was changed.
        //
        //  If we're not in a stream switch already, we want to initiate a stream switch event.
        //  We also we want to set the stream switch complete event.  That will signal the capture thread that it's ok to re-initialize the
        //  audio capturer.
        //
        if(!inStreamSwitch)
        {
            inStreamSwitch = true;
            SetEvent(streamSwitchEvent);
        }
        SetEvent(streamSwitchCompleteEvent);
    }
    return S_OK;
}

bool AudioSystem::SelectedDefault()
{
/*
    if(_DefaultSelected)
    {
        return false;
    }
*/
    if(!inStreamSwitch)
    {
        inStreamSwitch = true;
        defaultSelected = true;
        SetEvent(streamSwitchEvent);
    }
        SetEvent(streamSwitchCompleteEvent);
    return true;
}


bool AudioSystem::SelectedEndpoint(LPWSTR input)
{
   if(!inStreamSwitch)
   {
       inStreamSwitch = true;
       defaultSelected = false;
       if(endpointID)
       {
           if(wcscmp(input, endpointID)) // if they are different then free and swap!
           {
               free(endpointID);
               endpointID = _wcsdup(input);
           }
       }
       else
       {
           endpointID = _wcsdup(input);
       }
       SetEvent(streamSwitchEvent);
       // no need to compare just copy in and call event
       // copy
       // call event

   }
   SetEvent(streamSwitchCompleteEvent);
   return true;
}

//
//  IUnknown
//
 __attribute__((nothrow)) HRESULT AudioSystem::QueryInterface(REFIID Iid, void **Object)
{
    if(Object == nullptr)
    {
        return E_POINTER;
    }
    *Object = NULL;

    if(Iid == IID_IUnknown)
    {
        *Object = static_cast<IUnknown *>(static_cast<IAudioSessionEvents *>(this));
        AddRef();
    }
    else if(Iid == __uuidof(IMMNotificationClient))
    {
        *Object = static_cast<IMMNotificationClient *>(this);
        AddRef();
    }
    else if(Iid == __uuidof(IAudioSessionEvents))
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
    return InterlockedIncrement(&refCount);
}

 __attribute__((nothrow)) ULONG AudioSystem::Release()
{
    ULONG returnValue = InterlockedDecrement(&refCount);
    if(returnValue == 0)
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
    HANDLE waitArray[2] = {shutdownEvent, analysisSamplesReadyEvent };

    while (stillPlaying)
    {
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
    float* data = dataQueue.front();

    for(int fftIndex = 0, tempoIndex = 0; fftIndex < FRAMECOUNT; fftIndex++, tempoIndex++)
    {
        //apply Hann window function to fft data
        float multiplier = 0.5 * (1 - cos(2 * 3.1416 * fftIndex) / (FRAMECOUNT - 1));
        fvec_set_sample(fftIn, data[fftIndex] * multiplier, fftIndex);

        //copy tempo data as is
        fvec_set_sample(tempoIn, data[fftIndex], tempoIndex);
        if(tempoIndex == 511)
        {
            tempoIndex = -1;
            aubio_tempo_do(tempoObject, tempoIn, tempoOut);
            if(tempoOut->data[0] != 0)
            {
                bpm = aubio_tempo_get_bpm(tempoObject);
#ifdef QT_DEBUG
                qDebug() << "Realtime Tempo: " << aubio_tempo_get_bpm(tempoObject) << Qt::endl;
#endif
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


smpl_t AudioSystem::GetBPM()
{
    return bpm;
}

std::vector<double>& AudioSystem::GetMag()
{
    return mag;
}

smpl_t AudioSystem::GetBeatPeriod()
{
    return aubio_tempo_get_period_s(tempoObject);
}

float AudioSystem::GetVolume()
{
    if(inStreamSwitch)
    {
        return 0;
    }
    float vol;
    endpointVolume->GetMasterVolumeLevelScalar(&vol);
    return vol;
}

float AudioSystem::SetVolume(float vol)
{
    if(inStreamSwitch)
    {
        return -1;
    }
    if(endpointVolume->SetMasterVolumeLevelScalar(vol, NULL))
    {
        return vol;
    }
    return -1;
}
