#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBuffer>
#include <fftw3.h>

extern int choose_shape;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionFull_Screen_triggered();

    void on_actionClose_triggered();

    void on_actionRestart_triggered();

    void on_actionAll_shapes_triggered();

    void on_actionCube_triggered();

    void on_actionPrism_triggered();

    void on_actionSphere_triggered();

    void on_actionWaveform_triggered();

private:
    Ui::MainWindow *ui;
    void fullscreen();
    bool isFullscreen = false;
};
#endif // MAINWINDOW_H
