#ifndef ENDPOINTMENU_H
#define ENDPOINTMENU_H

#include "Audio/audiointerface.h"
#include <QMenu>
#include <QAction>

class EndpointMenu : public QMenu
{
private:
    Q_OBJECT
    AudioInterface* pInterface = nullptr;
    QActionGroup* group = nullptr;

protected:
    void showEvent(QShowEvent *event) override;
public:
    EndpointMenu(QWidget *parent = nullptr, AudioInterface *p = nullptr);
    EndpointMenu(const QString &title, QWidget *parent = nullptr, AudioInterface *p = nullptr);
    virtual ~EndpointMenu();

signals:

};

#endif // ENDPOINTMENU_H
