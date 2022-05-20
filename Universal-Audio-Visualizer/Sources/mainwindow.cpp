#include "./ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    pInterface = new AudioInterface();
    ui->setupUi(this);
    openGLWidget = new OGLWidget(ui->centralwidget, pInterface);
    openGLWidget->setObjectName(QString::fromUtf8("openGLWidget"));
    openGLWidget->setGeometry(QRect(190, 20, 400, 400));
    pEndpointMenu = new EndpointMenu("Audio Endpoints", menuBar(), pInterface);
    menuBar()->addMenu(pEndpointMenu);
}

MainWindow::~MainWindow()
{
    delete pInterface;
    delete ui;
}


