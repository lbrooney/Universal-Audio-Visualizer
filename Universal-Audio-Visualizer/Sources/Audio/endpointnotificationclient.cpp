#include "Audio/endpointnotificationclient.h"
#include "Audio/AudioMacros.h"

EndpointNotificationClient::EndpointNotificationClient() :
    _cRef(1),
    _pEnumerator(NULL)
{

}

EndpointNotificationClient::~EndpointNotificationClient()
{
    SAFE_RELEASE(_pEnumerator)
}

// IUnknown methods -- AddRef, Release, and QueryInterface

ULONG STDMETHODCALLTYPE EndpointNotificationClient::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG STDMETHODCALLTYPE EndpointNotificationClient::Release()
{
    ULONG ulRef = InterlockedDecrement(&_cRef);
    if (0 == ulRef)
    {
        delete this;
    }
    return ulRef;
}

HRESULT STDMETHODCALLTYPE EndpointNotificationClient::QueryInterface(
                            REFIID riid, VOID **ppvInterface)
{
    if (IID_IUnknown == riid)
    {
        AddRef();
        *ppvInterface = (IUnknown*)this;
    }
    else if (__uuidof(IMMNotificationClient) == riid)
    {
        AddRef();
        *ppvInterface = (IMMNotificationClient*)this;
    }
    else
    {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

// Callback methods for device-event notifications.

HRESULT STDMETHODCALLTYPE EndpointNotificationClient::OnDefaultDeviceChanged(
                            EDataFlow flow, ERole role,
                            LPCWSTR pwstrDeviceId)
{

    if(flow == eRender)
    {
        if(role == eConsole)
        {
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE EndpointNotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    return S_OK;
};

HRESULT STDMETHODCALLTYPE EndpointNotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE EndpointNotificationClient::OnDeviceStateChanged(
                            LPCWSTR pwstrDeviceId,
                            DWORD dwNewState)
{


    if (dwNewState == DEVICE_STATE_ACTIVE)
    {

    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE EndpointNotificationClient::OnPropertyValueChanged(
                            LPCWSTR pwstrDeviceId,
                            const PROPERTYKEY key)
{

    return S_OK;
}
