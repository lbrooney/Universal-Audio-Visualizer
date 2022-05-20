#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBuffer>
#include <fftw3.h>

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

private:
    Ui::MainWindow *ui;
    void fullscreen();
    bool isFullscreen = false;
};
#endif // MAINWINDOW_H
