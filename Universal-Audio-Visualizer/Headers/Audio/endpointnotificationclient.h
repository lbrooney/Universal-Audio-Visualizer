#ifndef ENDPOINTNOTIFICATIONCLIENT_H
#define ENDPOINTNOTIFICATIONCLIENT_H

#include <mmdeviceapi.h>

class EndpointNotificationClient : public IMMNotificationClient
{
private:
    LONG _cRef;
    IMMDeviceEnumerator *_pEnumerator;

public:
    EndpointNotificationClient();
    ~EndpointNotificationClient();
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(
                                REFIID riid, VOID **ppvInterface);
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
                                EDataFlow flow, ERole role,
                                LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(
                                LPCWSTR pwstrDeviceId,
                                DWORD dwNewState);
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
                                LPCWSTR pwstrDeviceId,
                                const PROPERTYKEY key);
};

#endif // ENDPOINTNOTIFICATIONCLIENT_H
