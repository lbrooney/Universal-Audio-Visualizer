#ifndef ENDPOINTMENU_H
#define ENDPOINTMENU_H

#include "audiosystem.h"
#include <QMenu>
#include <QActionGroup>

class EndpointMenu : public QMenu, IAudioSessionEvents, IMMNotificationClient
{
private:
    Q_OBJECT
    QActionGroup* endpointGroup = nullptr;
    AudioSystem* pSystem = nullptr;
    QList<QAction *> actionList;

    void addEndpointAction(LPWSTR input = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
public:
    EndpointMenu(const QString &title, QWidget *parent = nullptr, AudioSystem *p = nullptr);
    virtual ~EndpointMenu();

signals:

private slots:
    void setNewAudioEndpoint(QAction* a);

};

#endif // ENDPOINTMENU_H
