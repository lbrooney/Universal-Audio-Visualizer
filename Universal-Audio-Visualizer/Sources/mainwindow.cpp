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

bool redChecked = false;
bool blueChecked = false;
bool greenChecked = false;
bool selectedColor = false;

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

#ifdef QT_DEBUG
    debug = new QMenu("debug", this);
    menuBar()->addMenu(debug);
#endif

}

MainWindow::~MainWindow()
{
    delete openGLWidget;
    delete pEndpointMenu;
    delete ui;
    delete pInterface;
}

AudioInterface* MainWindow::getAudioInterface()
{
    return pInterface;
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
    Slider *window = new Slider(this);
    window->show();
}

bool MainWindow::checkToggled()
{
    if (openGLWidget->rgb_selector != QVector3D(1, 1, 1) && selectedColor == false)
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
    openGLWidget->rgb_selector = QVector3D(1, 0, 0);
    redChecked = true;
    blueChecked = false;
    greenChecked = false;
    selectedColor = true;
}

void MainWindow::on_actionGreen_triggered()
{
    if (checkToggled()) return;
    openGLWidget->rgb_selector = QVector3D(0, 1, 0);
    redChecked = false;
    blueChecked = false;
    greenChecked = true;
    selectedColor = true;
}

void MainWindow::on_actionBlue_triggered()
{
    if (checkToggled()) return;
    openGLWidget->rgb_selector = QVector3D(0, 0, 1);
    redChecked = false;
    blueChecked = true;
    greenChecked = false;
    selectedColor = true;
}

void MainWindow::on_actionWhite_triggered()
{
    if (checkToggled()) return;
    openGLWidget->rgb_selector = QVector3D(1, 1, 1);
    redChecked = false;
    blueChecked = false;
    greenChecked = false;
    selectedColor = true;
}

