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
    Fullscreen();
}

void MainWindow::Fullscreen()
{
    if(isFullscreen == true)
    {
        if(wasSliderWindowShown)
        {
            sliderWindow->move(sliderWindowPos);
            sliderWindow->show();
            wasSliderWindowShown = false;
        }
        ui->menubar->show();
        setStyleSheet("");
        showNormal();
        this->setFocus();
        isFullscreen = false;
    }
    else
    {
        setStyleSheet("background:transparent;");
        ui->menubar->hide();
        if(sliderWindow->isVisible())
        {
            sliderWindowPos = sliderWindow->pos();
            sliderWindow->close();
            wasSliderWindowShown = true;
        }
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
                Fullscreen();
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
    SetShapesFalse();
    openGLWidget->LoadPreset(1);
}

void MainWindow::on_actionPrism_triggered()
{
    auto* sphere = ui->actionSphere;
    auto* cube = ui->actionCube;
    auto* prism = ui->actionPrism;

    if(sphere->isChecked() and cube->isChecked())
    {
        if(!prism->isChecked())
        {
            //Load sphere and cube
            openGLWidget->LoadPreset(7);
        }
        else
        {
            //All shapes
            openGLWidget->LoadPreset(1);
        }
    }
    else if(sphere->isChecked())
    {
        if(!prism->isChecked())
        {
            // Load sphere
            openGLWidget->LoadPreset(4);
        }
        else
        {
            //Sphere and prism
            openGLWidget->LoadPreset(5);
        }
    }
    else if(cube->isChecked())
    {
        if(!prism->isChecked())
        {
            // Load sphere
            openGLWidget->LoadPreset(3);
        }
        else
        {
            //Cube and prism
            openGLWidget->LoadPreset(6);
        }
    }
    else
    {
        if(!prism->isChecked())
        {
            //Nothing is checked
            on_actionWaveform_triggered();
        }
        else
        {
            //Prism only
            openGLWidget->LoadPreset(2);
        }
    }
}

void MainWindow::on_actionCube_triggered()
{
    auto* sphere = ui->actionSphere;
    auto* prism = ui->actionPrism;
    auto* cube = ui->actionCube;

    if(sphere->isChecked() and prism->isChecked())
    {
        if(!cube->isChecked())
        {
            // Load sphere and prism
            openGLWidget->LoadPreset(5);
        }
        else
        {
            //All shapes
            openGLWidget->LoadPreset(1);
        }
    }
    else if(sphere->isChecked())
    {
        if(!cube->isChecked())
        {
            // Load sphere
            openGLWidget->LoadPreset(4);
        }
        else
        {
            // Load cube and sphere
            openGLWidget->LoadPreset(7);
        }
    }
    else if(prism->isChecked())
    {
        if(!cube->isChecked())
        {
            // Load prism
            openGLWidget->LoadPreset(2);
        }
        else
        {
            //Load prism and cube
            openGLWidget->LoadPreset(6);
        }
    }
    else
    {
        if(!cube->isChecked())
        {
            //Nothing is checked
            on_actionWaveform_triggered();
        }
        else
        {
            //Cube only
            openGLWidget->LoadPreset(3);
        }
    }
}

void MainWindow::on_actionSphere_triggered()
{
    auto* sphere = ui->actionSphere;
    auto* cube = ui->actionCube;
    auto* prism = ui->actionPrism;

    if(prism->isChecked() and cube->isChecked())
    {
        if(!sphere->isChecked())
        {
            // Prism and Cube
            openGLWidget->LoadPreset(6);
        }
        else
        {
            //All shapes
            openGLWidget->LoadPreset(1);
        }
    }
    else if(prism->isChecked())
    {
        if(!sphere->isChecked())
        {
            //Load prism
            openGLWidget->LoadPreset(2);
        }
        else
        {
            //Sphere and prism
            openGLWidget->LoadPreset(5);
        }
    }
    else if(cube->isChecked())
    {
        if(!sphere->isChecked())
        {
            //Load cube
            openGLWidget->LoadPreset(3);
        }
        else
        {
            //sphere and cube
            openGLWidget->LoadPreset(7);
        }
    }
    else
    {
        if(!sphere->isChecked())
        {
            //Nothing is checked
            on_actionWaveform_triggered();
        }
        else
        {
            //Sphere only
            openGLWidget->LoadPreset(4);
        }
    }
}

void MainWindow::SetShapesFalse()
{
    ui->actionSphere->setChecked(false);
    ui->actionCube->setChecked(false);
    ui->actionPrism->setChecked(false);
}

void MainWindow::on_actionWaveform_triggered()
{
    SetShapesFalse();
    openGLWidget->LoadPreset(0);
}

void MainWindow::on_actionSliders_triggered()
{
    sliderWindow->move(this->pos());
    sliderWindow->show();
}

bool MainWindow::checkToggled()
{
    if(openGLWidget->rgbSelector != QVector3D(1, 1, 1) && selectedColor == false)
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
    if(checkToggled())
    {
        return;
    }
    openGLWidget->rgbSelector = QVector3D(1, 0, 0);
    redChecked = true;
    blueChecked = false;
    greenChecked = false;
    selectedColor = true;
}

void MainWindow::on_actionGreen_triggered()
{
    if(checkToggled())
    {
        return;
    }
    openGLWidget->rgbSelector = QVector3D(0, 1, 0);
    redChecked = false;
    blueChecked = false;
    greenChecked = true;
    selectedColor = true;
}

void MainWindow::on_actionBlue_triggered()
{
    if(checkToggled())
    {
        return;
    }
    openGLWidget->rgbSelector = QVector3D(0, 0, 1);
    redChecked = false;
    blueChecked = true;
    greenChecked = false;
    selectedColor = true;
}

void MainWindow::on_actionWhite_triggered()
{
    if(checkToggled())
    {
        return;
    }
    openGLWidget->rgbSelector = QVector3D(1, 1, 1);
    redChecked = false;
    blueChecked = false;
    greenChecked = false;
    selectedColor = true;
}

