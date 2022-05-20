#include "Audio/audiocommons.h"
#include "Audio/AudioMacros.h"

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
    SAFE_RELEASE(pEnumerator);
    CoTaskMemFree(pSelectedDeviceID);
}

void AudioCommons::clearEndpointVector()
{
    if(activeEndpoints.empty())
    {
        return;
    }
    for(auto it = activeEndpoints.begin(); it != activeEndpoints.end();)
    {
        CoTaskMemFree(&it);
        it = activeEndpoints.erase(it);
    }
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

const LPWSTR AudioCommons::getSelectedDeviceID(void) const
{
    return pSelectedDeviceID;
}

IMMDeviceEnumerator* AudioCommons::getEnumerator(void) const
{
    return pEnumerator;
}

