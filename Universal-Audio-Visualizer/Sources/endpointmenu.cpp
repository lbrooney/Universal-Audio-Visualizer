#include "endpointmenu.h"
// https://stackoverflow.com/questions/9773822/how-to-fix-a-linker-error-with-pkey-device-friendlyname
#include <initguid.h>  // Put this in to get rid of linker errors.
#include <devpkey.h>  // Property keys defined here are now defined inline.
#include <Functiondiscoverykeys_devpkey.h>
#include <iostream>
#include <vector>
#include <mmdeviceapi.h>
#include "stdafx.h"
#include <QActionGroup>
#include <QDebug>
#include <wchar.h>

static const IID IID__IMMEndpoint = {
    //MIDL_INTERFACE("1BE09788-6894-4089-8586-9A2A6C265AC5")
    0x1be09788, 0x6894, 0x4089, {0x85, 0x86, 0x9a, 0x2a, 0x6c, 0x26, 0x5a, 0xc5}
};

#define IID__IMMENDPOINT                       (IID__IMMEndpoint)

EndpointMenu::EndpointMenu(const QString &title, QWidget *parent, AudioSystem *p)
    : QMenu{title, parent}
{
    pSystem = p;

    endpointGroup = new QActionGroup(this);
    endpointGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
    QAction *temp = new QAction(
                "Default", endpointGroup);
    temp->setObjectName("Default");
    temp->setCheckable(true);
    temp->setChecked(true);
    actionList.push_back(temp);
    endpointGroup->addAction(temp);
    this->addAction(temp);
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr))
    {
        printf("Unable to instantiate device enumerator: %x\n", hr);
    }

    IMMDeviceCollection* pCollection = nullptr;
    IMMDevice* pEndpoint = nullptr;

    pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
    UINT count = 0;
    pCollection->GetCount(&count);
    for (UINT i = 0; i < count; i+=1) {
        IMMDevice* pEndpoint = nullptr;
        pCollection->Item(i, &pEndpoint);
        LPWSTR id = nullptr;
        pEndpoint->GetId(&id);
        IPropertyStore* pProps = nullptr;
        PROPVARIANT varName;
        pEndpoint->OpenPropertyStore(
                    STGM_READ, &pProps);
        PropVariantInit(&varName);
        pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        QAction *temp = new QAction(
                    QString::fromWCharArray(varName.pwszVal, -1), endpointGroup);
        temp->setCheckable(true);
        temp->setObjectName(QString::fromWCharArray(id, -1));
        actionList.push_back(temp);
        endpointGroup->addAction(temp);
        this->addAction(temp);
        CoTaskMemFree(id);
        PropVariantClear(&varName);
        SafeRelease(&pProps);
        SafeRelease(&pEndpoint);
    }
    SafeRelease(&pCollection);
    hr = pEnumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        printf("Unable to register for stream switch notifications: %x\n", hr);
    }

    connect(endpointGroup, SIGNAL(triggered(QAction*)), this, SLOT(setNewAudioEndpoint(QAction*)));
    connect(this, SIGNAL(DeviceAdded(QString)), this, SLOT(AddDevice(QString)));
    connect(this, SIGNAL(DeviceRemoved(QString)), this, SLOT(RemoveDevice(QString)));
    return;
}


EndpointMenu::~EndpointMenu()
{

    return;
}

void EndpointMenu::Shutdown()
{
    HRESULT hr = pEnumerator->UnregisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        printf("Unable to unregister for endpoint notifications: %x\n", hr);
    }
    SafeRelease(&pEnumerator);
}

void EndpointMenu::showEvent(QShowEvent *event)
{
    return;
}

void EndpointMenu::setNewAudioEndpoint(QAction* a)
{
    if(a->objectName().compare("Default") == 0)
    {
        pSystem->selectedDefault();
    }
    else
    {
        LPWSTR id = (LPWSTR) calloc((a->objectName().size() + 1), sizeof(WCHAR));
        a->objectName().toWCharArray(id);
        pSystem->selectedEndpoint(id);
        free(id);
    }
}

void EndpointMenu::AddDevice(QString DeviceId)
{
    IMMDevice* pEndpoint = nullptr;
    LPWSTR id = (LPWSTR) calloc(DeviceId.size() + 1, sizeof(WCHAR));
    DeviceId.toWCharArray(id);
    pSystem->selectedEndpoint(id);
    pEnumerator->GetDevice(id, &pEndpoint);
    free(id);
    IPropertyStore* pProps = nullptr;
    PROPVARIANT varName;
    pEndpoint->OpenPropertyStore(
                STGM_READ, &pProps);
    PropVariantInit(&varName);
    pProps->GetValue(PKEY_Device_FriendlyName, &varName);
    QAction *temp = new QAction(
                QString::fromWCharArray(varName.pwszVal, -1), endpointGroup);
    temp->setCheckable(true);
    temp->setObjectName(DeviceId);
    actionList.push_back(temp);
    endpointGroup->addAction(temp);
    this->addAction(temp);
    PropVariantClear(&varName);
    SafeRelease(&pProps);
    SafeRelease(&pEndpoint);
}

void EndpointMenu::RemoveDevice(QString DeviceId)
{
    QAction *temp = nullptr;
    for (int i = 0; i < actionList.size(); i += 1)
    {
        if(actionList.at(i)->objectName().compare(DeviceId) == 0)
        {
            temp = actionList.at(i);
            actionList.removeAt(i);
            if(temp->isChecked())
                actionList.front()->setChecked(true);
            break;
        }
    }
    if(temp)
        delete temp;
    return;
}

 __attribute__((nothrow)) HRESULT EndpointMenu::OnDeviceStateChanged (LPCWSTR DeviceId, DWORD NewState)
{
    QString ID = QString::fromWCharArray(DeviceId, -1);
    IMMDevice* pDevice = nullptr;
    pEnumerator->GetDevice(DeviceId, &pDevice);
    IMMEndpoint* pEndpoint = nullptr;
    pDevice->QueryInterface(IID__IMMENDPOINT, (void**)&pEndpoint);
    EDataFlow pDataFlow;
    pEndpoint->GetDataFlow(&pDataFlow);
    if(pDataFlow == eRender)
    {
        switch(NewState)
        {
        case DEVICE_STATE_ACTIVE:
            emit DeviceAdded(ID);
            break;
        default:
            emit DeviceRemoved(ID);
            break;
        }
    }
    SafeRelease(&pEndpoint);
    SafeRelease(&pDevice);
    return S_OK;
}

//
//  IUnknown
//
 __attribute__((nothrow)) HRESULT EndpointMenu::QueryInterface(REFIID Iid, void **Object)
{
    if (Object == NULL)
    {
        return E_POINTER;
    }
    *Object = NULL;

    if (Iid == IID_IUnknown)
    {
        *Object = static_cast<IUnknown *>(static_cast<IMMNotificationClient *>(this));
        AddRef();
    }
    else if (Iid == __uuidof(IMMNotificationClient))
    {
        *Object = static_cast<IMMNotificationClient *>(this);
        AddRef();
    }
    else
    {
        return E_NOINTERFACE;
    }
    return S_OK;
}

 __attribute__((nothrow)) ULONG EndpointMenu::AddRef()
{
    return InterlockedIncrement(&_RefCount);
}

 __attribute__((nothrow)) ULONG EndpointMenu::Release()
{
    ULONG returnValue = InterlockedDecrement(&_RefCount);
    if (returnValue == 0)
    {
        delete this;
    }
    return returnValue;
}
