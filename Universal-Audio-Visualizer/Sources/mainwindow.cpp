#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>

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
