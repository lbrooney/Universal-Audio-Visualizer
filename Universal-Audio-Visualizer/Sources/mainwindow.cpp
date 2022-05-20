#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include "oglwidget.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    pInterface = new AudioInterface();
    ui->setupUi(this);
    openGLWidget = new OGLWidget(ui->centralwidget, pInterface);
    openGLWidget->setObjectName(QString::fromUtf8("openGLWidget"));
    ui->verticalLayout->addWidget(openGLWidget);
    pEndpointMenu = new EndpointMenu("Audio Endpoints", menuBar(), pInterface);
    menuBar()->addMenu(pEndpointMenu);
}

MainWindow::~MainWindow()
{
    delete openGLWidget;
    delete pEndpointMenu;
    delete ui;
    delete pInterface;
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
