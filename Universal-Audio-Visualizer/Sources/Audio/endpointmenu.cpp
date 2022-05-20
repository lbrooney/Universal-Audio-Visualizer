#include "Audio/endpointmenu.h"

EndpointMenu::EndpointMenu(QWidget *parent, AudioCommons *c)
    : QMenu{parent}
{
    common = c;
    return;
}

EndpointMenu::EndpointMenu(const QString &title, QWidget *parent, AudioCommons *c)
    : QMenu{title, parent}
{
    common = c;
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
