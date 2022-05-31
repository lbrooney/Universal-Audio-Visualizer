#ifndef ENDPOINTMENU_H
#define ENDPOINTMENU_H

#include "audiosystem.h"
#include <QMenu>
#include <QActionGroup>

class EndpointMenu : public QMenu, IMMNotificationClient
{
    Q_OBJECT
private:
    IMMDeviceEnumerator *enumerator;
    QActionGroup        *endpointGroup;
    AudioSystem         *aSystem;
    QList<QAction *>    actionList;
    LONG _RefCount;

    STDMETHOD(OnDeviceStateChanged)     (LPCWSTR /*DeviceId*/, DWORD /*NewState*/);
    STDMETHOD(OnDeviceAdded)            (LPCWSTR /*DeviceId*/) { return S_OK; }
    STDMETHOD(OnDeviceRemoved)          (LPCWSTR /*DeviceId(*/) { return S_OK; }
    STDMETHOD(OnDefaultDeviceChanged)   (EDataFlow Flow, ERole Role, LPCWSTR NewDefaultDeviceId) { return S_OK; }
    STDMETHOD(OnPropertyValueChanged)   (LPCWSTR /*DeviceId*/, const PROPERTYKEY /*Key*/){return S_OK; }
    //
    //  IUnknown
    //
    STDMETHOD(QueryInterface)(REFIID iid, void **pvObject);

public:
    EndpointMenu(const QString &title, QWidget *parent = nullptr, AudioSystem *a = nullptr);
    virtual ~EndpointMenu();
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    void Shutdown();

private slots:
    void SetNewAudioEndpoint(QAction* a);
    void AddDevice(QString DeviceId);
    void RemoveDevice(QString DeviceId);

signals:
    void DeviceAdded(QString DeviceId);
    void DeviceRemoved(QString DeviceId);

};

#endif // ENDPOINTMENU_H
