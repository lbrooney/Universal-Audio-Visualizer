#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include "oglwidget.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionFull_Screen_triggered()
{
    fullscreen();
}

void MainWindow::fullscreen()
{
    if(isFullscreen == false)
    {
        setWindowState(Qt::WindowFullScreen);
        isFullscreen = true;
    }
    else
    {
        showNormal();
        isFullscreen = false;
    }
}

void MainWindow::on_actionClose_triggered()
{
    close();
}


void MainWindow::on_actionRestart_triggered()
{
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}


void MainWindow::on_actionAll_shapes_triggered()
{
    ui->openGLWidget->loadPreset(1);

}

void MainWindow::on_actionPrism_triggered()
{
    ui->openGLWidget->loadPreset(2);
}

void MainWindow::on_actionCube_triggered()
{
    ui->openGLWidget->loadPreset(3);
}


void MainWindow::on_actionSphere_triggered()
{
    ui->openGLWidget->loadPreset(4);
}


void MainWindow::on_actionWaveform_triggered()
{
    ui->openGLWidget->loadPreset(0);
}

