#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include "oglwidget.h"
#include "mainwindow.h"
#include <QGraphicsView>
#include <iostream>
#include <QObject>
#include <QDebug>
#include "stdafx.h"

bool redChecked = false;
bool blueChecked = false;
bool greenChecked = false;
bool selectedColor = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    pSystem = new (std::nothrow) AudioSystem();
    pSystem->Initialize();
    pSystem->Start();
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Icons/img/icon.png"));
    setWindowTitle("Universal Audio Visualizer");
    openGLWidget = new OGLWidget(ui->centralwidget, pSystem);
    openGLWidget->setObjectName(QString::fromUtf8("openGLWidget"));
    ui->verticalLayout->addWidget(openGLWidget);
    pEndpointMenu = new EndpointMenu("Audio Endpoints", menuBar(), pSystem);
    menuBar()->addMenu(pEndpointMenu);
    sliderWindow = new Slider(this, pSystem, openGLWidget);
}

MainWindow::~MainWindow()
{
    pSystem->Stop();
    delete sliderWindow;
    delete openGLWidget;
    pEndpointMenu->Shutdown();
    SafeRelease(&pEndpointMenu);
    delete ui;
    pSystem->Shutdown();
    SafeRelease(&pSystem);
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
    set_shapes_false();
    openGLWidget->loadPreset(1);
}

void MainWindow::on_actionPrism_triggered()
{
    auto* sphere_ptr = ui->actionSphere;
    auto* cube_ptr = ui->actionCube;
    auto* prism_ptr = ui->actionPrism;

    if(sphere_ptr->isChecked() and cube_ptr->isChecked())
    {
        if(!prism_ptr->isChecked())
        {
            //Load sphere and cube
            openGLWidget->loadPreset(7);
        }
        else
        {
            //All shapes
            openGLWidget->loadPreset(1);
        }
    }
    else if(sphere_ptr->isChecked())
    {
        if(!prism_ptr->isChecked())
        {
            // Load sphere
            openGLWidget->loadPreset(4);
        }
        else
        {
            //Sphere and prism
            openGLWidget->loadPreset(5);
        }
    }
    else if(cube_ptr->isChecked())
    {
        if(!prism_ptr->isChecked())
        {
            // Load sphere
            openGLWidget->loadPreset(3);
        }
        else
        {
            //Cube and prism
            openGLWidget->loadPreset(6);
        }
    }
    else
    {
        if(!prism_ptr->isChecked())
        {
            //Nothing is checked
            on_actionWaveform_triggered();
        }
        else
        {
            //Prism only
            openGLWidget->loadPreset(2);
        }
    }
}

void MainWindow::on_actionCube_triggered()
{
    auto* sphere_ptr = ui->actionSphere;
    auto* prism_ptr = ui->actionPrism;
    auto* cube_ptr = ui->actionCube;
    if(sphere_ptr->isChecked() and prism_ptr->isChecked())
    {
        if(!cube_ptr->isChecked())
        {
            // Load sphere and prism
            openGLWidget->loadPreset(5);
        }
        else
        {
            //All shapes
            openGLWidget->loadPreset(1);
        }
    }
    else if(sphere_ptr->isChecked())
    {
        if(!cube_ptr->isChecked())
        {
            // Load sphere
            openGLWidget->loadPreset(4);
        }
        else
        {
            // Load cube and sphere
            openGLWidget->loadPreset(7);
        }
    }
    else if(prism_ptr->isChecked())
    {
        if(!cube_ptr->isChecked())
        {
            // Load prism
            openGLWidget->loadPreset(2);
        }
        else
        {
            //Load prism and cube
            openGLWidget->loadPreset(6);
        }
    }
    else
    {
        if(!cube_ptr->isChecked())
        {
            //Nothing is checked
            on_actionWaveform_triggered();
        }
        else
        {
            //Cube only
            openGLWidget->loadPreset(3);
        }
    }
}

void MainWindow::on_actionSphere_triggered()
{
    auto* sphere_ptr = ui->actionSphere;
    auto* cube_ptr = ui->actionCube;
    auto* prism_ptr = ui->actionPrism;

    if(prism_ptr->isChecked() and cube_ptr->isChecked())
    {
        if(!sphere_ptr->isChecked())
        {
            // Prism and Cube
            openGLWidget->loadPreset(6);
        }
        else
        {
            //All shapes
            openGLWidget->loadPreset(1);
        }
    }
    else if(prism_ptr->isChecked())
    {
        if(!sphere_ptr->isChecked())
        {
            //Load prism
            openGLWidget->loadPreset(2);
        }
        else
        {
            //Sphere and prism
            openGLWidget->loadPreset(5);
        }
    }
    else if(cube_ptr->isChecked())
    {
        if(!sphere_ptr->isChecked())
        {
            //Load cube
            openGLWidget->loadPreset(3);
        }
        else
        {
            //sphere and cube
            openGLWidget->loadPreset(7);
        }
    }
    else
    {
        if(!sphere_ptr->isChecked())
        {
            //Nothing is checked
            on_actionWaveform_triggered();
        }
        else
        {
            //Sphere only
            openGLWidget->loadPreset(4);
        }
    }
}

void MainWindow::set_shapes_false() {
    ui->actionSphere->setChecked(false);
    ui->actionCube->setChecked(false);
    ui->actionPrism->setChecked(false);
}

void MainWindow::on_actionWaveform_triggered()
{
    set_shapes_false();
    openGLWidget->loadPreset(0);
}

void MainWindow::on_actionSliders_triggered()
{
    sliderWindow->show();
}

bool MainWindow::checkToggled()
{
    if (openGLWidget->rgbSelector != QVector3D(1, 1, 1) && selectedColor == false)
    {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", "Turn off the tempo changes color button!");
        messageBox.setFixedSize(500, 200);
        return 1;
    }
    return 0;
}

void MainWindow::on_actionRed_triggered()
{
    if (checkToggled()) return;
    openGLWidget->rgbSelector = QVector3D(1, 0, 0);
    redChecked = true;
    blueChecked = false;
    greenChecked = false;
    selectedColor = true;
}

void MainWindow::on_actionGreen_triggered()
{
    if (checkToggled()) return;
    openGLWidget->rgbSelector = QVector3D(0, 1, 0);
    redChecked = false;
    blueChecked = false;
    greenChecked = true;
    selectedColor = true;
}

void MainWindow::on_actionBlue_triggered()
{
    if (checkToggled()) return;
    openGLWidget->rgbSelector = QVector3D(0, 0, 1);
    redChecked = false;
    blueChecked = true;
    greenChecked = false;
    selectedColor = true;
}

void MainWindow::on_actionWhite_triggered()
{
    if (checkToggled()) return;
    openGLWidget->rgbSelector = QVector3D(1, 1, 1);
    redChecked = false;
    blueChecked = false;
    greenChecked = false;
    selectedColor = true;
}

