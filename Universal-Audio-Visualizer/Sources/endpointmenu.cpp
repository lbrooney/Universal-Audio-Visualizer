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

const IID IID_IMMDevice = __uuidof(IMMDevice);

EndpointMenu::EndpointMenu(const QString &title, QWidget *parent, AudioSystem *a)
    : QMenu{title, parent}, aSystem(a)
{
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
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr))
    {
        printf("Unable to instantiate device enumerator: %x\n", hr);
    }

    IMMDeviceCollection* collection = nullptr;
    IMMDevice* endpoint = nullptr;

    enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection);
    UINT count = 0;
    collection->GetCount(&count);
    for (UINT i = 0; i < count; i+=1) {
        IMMDevice* endpoint = nullptr;
        collection->Item(i, &endpoint);
        LPWSTR id = nullptr;
        endpoint->GetId(&id);
        IPropertyStore* pProps = nullptr;
        PROPVARIANT varName;
        endpoint->OpenPropertyStore(
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
        SafeRelease(&endpoint);
    }
    SafeRelease(&collection);
    hr = enumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        printf("Unable to register for stream switch notifications: %x\n", hr);
    }

    connect(endpointGroup, SIGNAL(triggered(QAction*)), this, SLOT(SetNewAudioEndpoint(QAction*)));
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
    HRESULT hr = enumerator->UnregisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        printf("Unable to unregister for endpoint notifications: %x\n", hr);
    }
    SafeRelease(&enumerator);
}

void EndpointMenu::SetNewAudioEndpoint(QAction* a)
{
    if(a->objectName().compare("Default") == 0)
    {
        aSystem->SelectedDefault();
    }
    else
    {
        LPWSTR id = (LPWSTR) calloc((a->objectName().size() + 1), sizeof(WCHAR));
        a->objectName().toWCharArray(id);
        aSystem->SelectedEndpoint(id);
        free(id);
    }
}

void EndpointMenu::AddDevice(QString DeviceId)
{
    IMMDevice* endpoint = nullptr;
    LPWSTR id = (LPWSTR) calloc(DeviceId.size() + 1, sizeof(WCHAR));
    DeviceId.toWCharArray(id);
    aSystem->SelectedEndpoint(id);
    enumerator->GetDevice(id, &endpoint);
    free(id);
    IPropertyStore* props = nullptr;
    PROPVARIANT varName;
    endpoint->OpenPropertyStore(
                STGM_READ, &props);
    PropVariantInit(&varName);
    props->GetValue(PKEY_Device_FriendlyName, &varName);
    QAction *temp = new QAction(
                QString::fromWCharArray(varName.pwszVal, -1), endpointGroup);
    temp->setCheckable(true);
    temp->setObjectName(DeviceId);
    actionList.push_back(temp);
    endpointGroup->addAction(temp);
    this->addAction(temp);
    PropVariantClear(&varName);
    SafeRelease(&props);
    SafeRelease(&endpoint);
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
    enumerator->GetDevice(DeviceId, &pDevice);
    IMMEndpoint* endpoint = nullptr;
    pDevice->QueryInterface(IID_IMMDevice, (void**)&endpoint);
    EDataFlow dataFlow;
    endpoint->GetDataFlow(&dataFlow);
    if(dataFlow == eRender)
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
    SafeRelease(&endpoint);
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
