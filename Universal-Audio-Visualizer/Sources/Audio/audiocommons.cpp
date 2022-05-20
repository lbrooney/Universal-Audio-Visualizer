#include "Audio/audiocommons.h"
#include "Audio/AudioMacros.h"
#include <iostream>
#include <wchar.h>

AudioCommons::AudioCommons()
{
    IMMDevice *pDevice = nullptr;
    CoCreateInstance(
                    CLSID_MMDeviceEnumerator, NULL,
                    CLSCTX_ALL, IID_IMMDeviceEnumerator,
                    (void**)&pEnumerator);
    pEnumerator->GetDefaultAudioEndpoint(
                    eRender, eConsole, &pDevice);
    pDevice->GetId(&pSelectedDeviceID);
    SAFE_RELEASE(pDevice);
    refreshEndpoints();
}

AudioCommons::~AudioCommons()
{
    std::cout << " | audio common delete start";
    SAFE_RELEASE(pEnumerator);
    CoTaskMemFree(pSelectedDeviceID);
    clearEndpointVector();
    std::cout << " | audio common delete end" << std::endl;
}

void AudioCommons::clearEndpointVector()
{
    if(activeEndpoints.empty())
    {
        return;
    }
    for(int i = 0; i < activeEndpoints.size(); i += 1)
    {
        CoTaskMemFree(activeEndpoints.at(i));
    }
    activeEndpoints.clear();
}

void AudioCommons::refreshEndpoints(void)
{
    clearEndpointVector();
    IMMDeviceCollection* pCollection = nullptr;
    IMMDevice* pEndpoint = nullptr;
    pEnumerator->GetDefaultAudioEndpoint(
                eRender, eConsole, &pEndpoint);
    LPWSTR id = nullptr;
    pEndpoint->GetId(&id);
    activeEndpoints.push_back( {id} );
    pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
    UINT count = 0;
    pCollection->GetCount(&count);
    for (UINT i = 0; i < count; i+=1) {
        pCollection->Item(i, &pEndpoint);
        pEndpoint->GetId(&id);
        // { } may not work. may need to allocate memory for copy of id
        activeEndpoints.push_back( {id} );
    }
    return;
}

const std::vector<LPWSTR>& AudioCommons::getEndpoints(void) const
{
    return activeEndpoints;
}


// REQUESTED PROGRAM MUST CALL CoTaskMemFree on string
LPWSTR AudioCommons::getSelectedDeviceID(void) const
{
    SIZE_T strSize = sizeof(LPWSTR) * (wcslen(pSelectedDeviceID) + 1);
    LPWSTR copy = (LPWSTR)CoTaskMemAlloc(strSize);
    memcpy(copy, pSelectedDeviceID, strSize);
    return copy;
}

// OTHER CLASSES SHOULD NOT RELEASE THIS, AUDIO INTERFACE DOES IT
IMMDeviceEnumerator* AudioCommons::getEnumerator(void) const
{
    return pEnumerator;
}

