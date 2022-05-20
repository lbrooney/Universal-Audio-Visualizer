#include "endpointmenu.h"
// https://stackoverflow.com/questions/9773822/how-to-fix-a-linker-error-with-pkey-device-friendlyname
#include <initguid.h>  // Put this in to get rid of linker errors.
#include <devpkey.h>  // Property keys defined here are now defined inline.
#include <Functiondiscoverykeys_devpkey.h>
#include <vector>
#include <mmdeviceapi.h>
#include "Audio/AudioMacros.h"
#include <QActionGroup>

EndpointMenu::EndpointMenu(QWidget *parent, AudioInterface *p)
    : QMenu{parent}
{
    pInterface = p;
    return;
}

EndpointMenu::EndpointMenu(const QString &title, QWidget *parent, AudioInterface *p)
    : QMenu{title, parent}
{
    pInterface = p;
    return;
}


EndpointMenu::~EndpointMenu()
{
    if(endpointGroup != nullptr)
    {
        delete endpointGroup;
    }
    return;
}

void EndpointMenu::showEvent(QShowEvent *event)
{
    this->clear();
    if(endpointGroup != nullptr)
    {
        delete endpointGroup;
    }
    endpointGroup = new QActionGroup(this);
    IMMDevice* pDevice = nullptr;
    const std::vector<LPWSTR> endpoints = pInterface->getEndpoints();
    for(auto it : endpoints)
    {
        pInterface->getEnumerator()->GetDevice(it, &pDevice);
        IPropertyStore* pProps = nullptr;
        PROPVARIANT varName;
        pDevice->OpenPropertyStore(
                    STGM_READ, &pProps);
        PropVariantInit(&varName);
        pProps->GetValue(
                    PKEY_Device_FriendlyName, &varName);
        this->addAction(QString::fromWCharArray(varName.pwszVal, -1));
        PropVariantClear(&varName);
        SAFE_RELEASE(pProps);
    }
    return;
}
