#include "endpointmenu.h"
// https://stackoverflow.com/questions/9773822/how-to-fix-a-linker-error-with-pkey-device-friendlyname
#include <initguid.h>  // Put this in to get rid of linker errors.
#include <devpkey.h>  // Property keys defined here are now defined inline.
#include <Functiondiscoverykeys_devpkey.h>
#include <vector>
#include <mmdeviceapi.h>
#include "Audio/AudioMacros.h"
#include <QActionGroup>
#include <QDebug>

EndpointMenu::EndpointMenu(const QString &title, QWidget *parent, AudioInterface *p)
    : QMenu{title, parent}
{
    pInterface = p;
    const std::vector<LPWSTR> endpoints = pInterface->getEndpoints();

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

    for(int i = 0; i < endpoints.size(); i += 1)
    {
        addEndpointAction(endpoints.at(i));
    }
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

void EndpointMenu::addEndpointAction(LPWSTR input)
{
    IMMDevice* pDevice = nullptr;
    pInterface->getEnumerator()->GetDevice(input, &pDevice);
    IPropertyStore* pProps = nullptr;
    PROPVARIANT varName;
    pDevice->OpenPropertyStore(
                STGM_READ, &pProps);
    PropVariantInit(&varName);
    pProps->GetValue(PKEY_Device_FriendlyName, &varName);
    QAction *temp = new QAction(
                QString::fromWCharArray(varName.pwszVal, -1), endpointGroup);
    temp->setCheckable(true);
    temp->setObjectName(QString::fromWCharArray(input, -1));
    actionList.push_back(temp);
    endpointGroup->addAction(temp);
    this->addAction(temp);
    PropVariantClear(&varName);
    SAFE_RELEASE(pProps);
    SAFE_RELEASE(pDevice)
}

void EndpointMenu::setNewAudioEndpoint(QAction* a)
{
    SIZE_T strSize = sizeof(LPWSTR) * (a->objectName().size() + 1);
    LPWSTR copy = (LPWSTR)CoTaskMemAlloc(strSize);
    a->objectName().toWCharArray(copy);
    pInterface->setAudioEndpoint(copy);
    CoTaskMemFree(copy);
}
