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
    std::cout << "main window delete start" << std::endl;;
    delete openGLWidget;
    std::cout << "ogl deleted" << std::endl;;
    delete pEndpointMenu;
    std::cout << "endpoint menu deleted" << std::endl;;
    delete ui;
    std::cout << "ui deleted" << std::endl;
    std::cout << "shit you crashing dawg" << std::endl;
    delete pInterface;
    std::cout << "audio interface deleted" << std::endl << " | main window delete end" << std::endl;
}


