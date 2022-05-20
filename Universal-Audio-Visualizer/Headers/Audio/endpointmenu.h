#ifndef ENDPOINTMENU_H
#define ENDPOINTMENU_H

#include "Audio/audiointerface.h"
#include <QMenu>

class EndpointMenu : public QMenu
{
private:
    Q_OBJECT
    AudioInterface* pInterface = nullptr;

protected:
    void showEvent(QShowEvent *event) override;
public:
    explicit EndpointMenu(QWidget *parent = nullptr, AudioInterface *p = nullptr);
    explicit EndpointMenu(const QString &title, QWidget *parent = nullptr, AudioInterface *p = nullptr);
    virtual ~EndpointMenu();

signals:

};

#endif // ENDPOINTMENU_H
