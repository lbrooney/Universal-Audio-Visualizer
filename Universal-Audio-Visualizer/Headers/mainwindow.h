#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBuffer>
#include <fftw3.h>
#include <QKeyEvent>
#include <QtWidgets>
#include "oglwidget.h"
#include "endpointmenu.h"
#include "audiosystem.h"

extern int choose_shape;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
private:
    Q_OBJECT
    Ui::MainWindow *ui;
    void fullscreen();
    bool isFullscreen = false;
    AudioSystem* pSystem = nullptr;
    EndpointMenu* pEndpointMenu = nullptr;
    OGLWidget* openGLWidget;

#ifdef QT_DEBUG
    QMenu* debug = nullptr;
#endif

public:
    MainWindow(QWidget *parent = nullptr);
    OGLWidget* getOGLWidget();
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *);

private slots:
    void on_actionFull_Screen_triggered();

    void on_actionClose_triggered();

    void on_actionRestart_triggered();

    void on_actionAll_shapes_triggered();

    void on_actionCube_triggered();

    void on_actionPrism_triggered();

    void on_actionSphere_triggered();

    void on_actionWaveform_triggered();

    void on_actionSliders_triggered();

    void on_actionRed_triggered();

    void on_actionGreen_triggered();

    void on_actionBlue_triggered();

    void set_shapes_false();

};
#endif // MAINWINDOW_H
