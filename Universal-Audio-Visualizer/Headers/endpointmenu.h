#ifndef ENDPOINTMENU_H
#define ENDPOINTMENU_H

#include "audiosystem.h"
#include <QMenu>
#include <QActionGroup>

class EndpointMenu : public QMenu, IMMNotificationClient
{
private:
    Q_OBJECT
    IMMDeviceEnumerator* pEnumerator;
    QActionGroup* endpointGroup = nullptr;
    AudioSystem* pSystem = nullptr;
    QList<QAction *> actionList;
    LONG _RefCount;

    STDMETHOD(OnDeviceStateChanged) (LPCWSTR /*DeviceId*/, DWORD /*NewState*/);
    STDMETHOD(OnDeviceAdded) (LPCWSTR /*DeviceId*/) { return S_OK; }
    STDMETHOD(OnDeviceRemoved) (LPCWSTR /*DeviceId(*/) { return S_OK; }
    STDMETHOD(OnDefaultDeviceChanged) (EDataFlow Flow, ERole Role, LPCWSTR NewDefaultDeviceId) { return S_OK; }
    STDMETHOD(OnPropertyValueChanged) (LPCWSTR /*DeviceId*/, const PROPERTYKEY /*Key*/){return S_OK; }
    //
    //  IUnknown
    //
    STDMETHOD(QueryInterface)(REFIID iid, void **pvObject);

    void addEndpointAction(LPWSTR input = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
public:
    EndpointMenu(const QString &title, QWidget *parent = nullptr, AudioSystem *p = nullptr);
    virtual ~EndpointMenu();
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    void Shutdown();

signals:

private slots:
    void setNewAudioEndpoint(QAction* a);

};

#endif // ENDPOINTMENU_H
