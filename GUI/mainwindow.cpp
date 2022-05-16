#include "mainwindow.h"
#include "preferences.h"

#include "./ui_mainwindow.h"
#include "./ui_preferences.h"

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


void MainWindow::on_actionPreferences_triggered()
{
    Preferences *p = new Preferences();
    p->show();
}

