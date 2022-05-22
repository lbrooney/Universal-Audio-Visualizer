#ifndef ENDPOINTMENU_H
#define ENDPOINTMENU_H

#include "Audio/audiointerface.h"
#include <QMenu>
#include <QActionGroup>

class EndpointMenu : public QMenu
{
private:
    Q_OBJECT
    QActionGroup* endpointGroup = nullptr;
    AudioInterface* pInterface = nullptr;
    QList<QAction *> actionList;

    void addEndpointAction(LPWSTR input = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
public:
    EndpointMenu(const QString &title, QWidget *parent = nullptr, AudioInterface *p = nullptr);
    virtual ~EndpointMenu();

signals:

private slots:
    void setNewAudioEndpoint(QAction* a);

};

#endif // ENDPOINTMENU_H
