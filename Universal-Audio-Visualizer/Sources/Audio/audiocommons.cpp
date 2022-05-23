#include "Audio/audiocommons.h"
#include "Audio/AudioMacros.h"
#include <initguid.h>  // Put this in to get rid of linker errors.
#include <devpkey.h>  // Property keys defined here are now defined inline.
#include <Functiondiscoverykeys_devpkey.h>
#include <iostream>
#include <wchar.h>
#include <QDebug>

AudioCommons::AudioCommons()
{
    CoCreateInstance(
                    CLSID_MMDeviceEnumerator, NULL,
                    CLSCTX_ALL, IID_IMMDeviceEnumerator,
                    (void**)&pEnumerator);
    refreshEndpoints();
}

AudioCommons::~AudioCommons()
{
    SAFE_RELEASE(pEnumerator);
    clearEndpointVector();
}

void AudioCommons::clearEndpointVector()
{
    if(activeEndpoints.empty())
    {
        return;
    }
    for(auto it : activeEndpoints)
    {
        CoTaskMemFree(it);
    }
    activeEndpoints.clear();
}

void AudioCommons::refreshEndpoints(void)
{
    clearEndpointVector();
    IMMDeviceCollection* pCollection = nullptr;
    IMMDevice* pEndpoint = nullptr;
    LPWSTR id = nullptr;
    pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
    UINT count = 0;
    pCollection->GetCount(&count);
    for (UINT i = 0; i < count; i+=1) {
        pCollection->Item(i, &pEndpoint);
        pEndpoint->GetId(&id);
        // { } may not work. may need to allocate memory for copy of id
        activeEndpoints.push_back( id );
    }
    pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pEndpoint);
    pEndpoint->GetId(&id);
    for(UINT i = 0; i < activeEndpoints.size(); i += 1)
    {
        if(wcscmp(id, activeEndpoints.at(i)) == 0)
        {
            defaultIDSpot = i;
            break;
        }
    }
    selectIDSpot = defaultIDSpot;
    SAFE_RELEASE(pEndpoint)
    SAFE_RELEASE(pCollection)
    return;
}

const std::vector<LPWSTR>& AudioCommons::getEndpoints(void) const
{
    return activeEndpoints;
}


// REQUESTED PROGRAM MUST CALL CoTaskMemFree on string
void AudioCommons::getSelectedDeviceID(LPWSTR& input)
{
    SIZE_T strSize = sizeof(LPWSTR) * (wcslen(activeEndpoints.at(selectIDSpot)) + 1) ;
    input = (LPWSTR)CoTaskMemAlloc(strSize);
    memcpy(input, activeEndpoints.at(selectIDSpot), strSize);
    return;
}

// OTHER CLASSES SHOULD NOT RELEASE THIS, AUDIO INTERFACE DOES IT
IMMDeviceEnumerator* AudioCommons::getEnumerator(void) const
{
    return pEnumerator;
}

void AudioCommons::setAudioEndpoint(const UINT input)
{
#ifdef QT_DEBUG
    qDebug() << "cur: " << input << "input: " << selectIDSpot << Qt::endl;
    qDebug() << "Current Device Name ";
    printDeviceName(activeEndpoints.at(selectIDSpot));
#endif
    if(input == -1)
    {
        selectIDSpot = defaultIDSpot;
    } else {
        selectIDSpot = input;
    }
#ifdef QT_DEBUG
    qDebug() << "New Device Name ";
    printDeviceName(activeEndpoints.at(selectIDSpot));
    qDebug() << "input :" << input << "new: " << selectIDSpot << Qt::endl;
#endif
    return;
}

void AudioCommons::printDeviceName(const LPWSTR input) const
{
    IMMDevice* pEndpoint = nullptr;
    pEnumerator->GetDevice(input, &pEndpoint);
    IPropertyStore* pProps = nullptr;
    PROPVARIANT varName;
    pEndpoint->OpenPropertyStore(
                STGM_READ, &pProps);
    PropVariantInit(&varName);
    pProps->GetValue(PKEY_Device_FriendlyName, &varName);
    qDebug() << QString::fromWCharArray(varName.pwszVal, -1);
    PropVariantClear(&varName);
    SAFE_RELEASE(pProps);
    SAFE_RELEASE(pEndpoint)
}
