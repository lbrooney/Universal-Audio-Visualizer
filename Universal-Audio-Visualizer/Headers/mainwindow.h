#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBuffer>
#include <fftw3.h>
#include "oglwidget.h"
#include "endpointmenu.h"
#include "Audio/audiointerface.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
private:
    Q_OBJECT
    AudioInterface* pInterface = nullptr;
    EndpointMenu* pEndpointMenu = nullptr;
    OGLWidget* openGLWidget;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
