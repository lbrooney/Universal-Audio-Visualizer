#include "endpointmenu.h"
// https://stackoverflow.com/questions/9773822/how-to-fix-a-linker-error-with-pkey-device-friendlyname
#include <initguid.h>  // Put this in to get rid of linker errors.
#include <devpkey.h>  // Property keys defined here are now defined inline.
#include <Functiondiscoverykeys_devpkey.h>
#include <vector>
#include <mmdeviceapi.h>
#include "stdafx.h"
#include <QActionGroup>
#include <QDebug>

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
    IMMDeviceEnumerator* pEnumerator = nullptr;
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
    SafeRelease(&pEnumerator);

    connect(endpointGroup, SIGNAL(triggered(QAction*)), this, SLOT(setNewAudioEndpoint(QAction*)));
    return;
}


EndpointMenu::~EndpointMenu()
{
    delete endpointGroup;
    return;
}

void EndpointMenu::showEvent(QShowEvent *event)
{
    return;
}

void EndpointMenu::setNewAudioEndpoint(QAction* a)
{
    if(a->objectName().compare("Default"))
    {
        pSystem->selectedDefault();
    }
    else
    {
        LPWSTR id = (LPWSTR) malloc((a->objectName().size() + 1) * sizeof(WCHAR));
        a->objectName().toWCharArray(id);
        pSystem->selectedEndpoint(id);
        free(id);
    }
}

