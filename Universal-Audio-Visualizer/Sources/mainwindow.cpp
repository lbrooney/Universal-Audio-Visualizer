#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include "oglwidget.h"
#include "mainwindow.h"
#include <QGraphicsView>
#include <iostream>
#include <QObject>
#include <QDebug>
#include "slider.h"
#include "stdafx.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    pSystem = new (std::nothrow) AudioSystem();
    pSystem->Initialize();
    pSystem->Start();
    ui->setupUi(this);
    openGLWidget = new OGLWidget(ui->centralwidget, pSystem);
    openGLWidget->setObjectName(QString::fromUtf8("openGLWidget"));
    ui->verticalLayout->addWidget(openGLWidget);
    pEndpointMenu = new EndpointMenu("Audio Endpoints", menuBar(), pSystem);
    menuBar()->addMenu(pEndpointMenu);
}

MainWindow::~MainWindow()
{
    pSystem->Stop();
    delete openGLWidget;
    pEndpointMenu->Shutdown();
    SafeRelease(&pEndpointMenu);
    delete ui;
    pSystem->Shutdown();
    SafeRelease(&pSystem);
}

OGLWidget* MainWindow::getOGLWidget()
{
    return openGLWidget;
}


void MainWindow::on_actionFull_Screen_triggered()
{
    fullscreen();
}

void MainWindow::fullscreen()
{
    if(isFullscreen == true)
    {
        ui->menubar->show();
        setStyleSheet("");
        showNormal();
        isFullscreen = false;
    }
    else
    {
        setStyleSheet("background:transparent;");
        ui->menubar->hide();
        showFullScreen();
        isFullscreen = true;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *keyevent)
{
    switch(keyevent->key())
    {
        case Qt::Key_Escape: case Qt::Key_F11:
            if(isFullscreen == true)
            {
                fullscreen();
            }
            break;
        default:
            break;
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
    openGLWidget->loadPreset(1);

}

void MainWindow::on_actionPrism_triggered()
{
    openGLWidget->loadPreset(2);
}

void MainWindow::on_actionCube_triggered()
{
    openGLWidget->loadPreset(3);
}


void MainWindow::on_actionSphere_triggered()
{
    openGLWidget->loadPreset(4);
}


void MainWindow::on_actionWaveform_triggered()
{
    openGLWidget->loadPreset(0);
}

void MainWindow::on_actionSliders_triggered()
{
    Slider *window = new Slider(this, pSystem);
    window->show();
}

void MainWindow::on_actionRed_triggered()
{
    openGLWidget->rgbSelector=QVector3D(1,0,0);
}


void MainWindow::on_actionGreen_triggered()
{
    openGLWidget->rgbSelector=QVector3D(0,1,0);
}


void MainWindow::on_actionBlue_triggered()
{
    openGLWidget->rgbSelector=QVector3D(0,0,1);
}




