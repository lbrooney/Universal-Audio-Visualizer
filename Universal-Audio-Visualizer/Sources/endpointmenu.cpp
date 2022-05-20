#include "Audio/endpointmenu.h"

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
    return;
}

void EndpointMenu::showEvent(QShowEvent *event)
{
    this->clear();
    this->addAction("YES");
    this->addAction("NO");
}
