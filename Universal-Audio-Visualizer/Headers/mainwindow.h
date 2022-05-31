#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBuffer>
#include <QKeyEvent>
#include <QtWidgets>
#include "oglwidget.h"
#include "endpointmenu.h"
#include "audiosystem.h"
#include "slider.h"

extern int choose_shape;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    void Fullscreen();
    bool checkToggled();
    bool isFullscreen = false;
    AudioSystem* pSystem = nullptr;
    EndpointMenu* pEndpointMenu = nullptr;
    OGLWidget* openGLWidget;
    Slider* sliderWindow = nullptr;
    bool wasSliderWindowShown = false;
    QPoint sliderWindowPos;
    void SetShapesFalse();

public:
    MainWindow(QWidget *parent = nullptr);
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

    void on_actionWhite_triggered();
};
#endif // MAINWINDOW_H
