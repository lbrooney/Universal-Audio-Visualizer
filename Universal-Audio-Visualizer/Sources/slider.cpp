#include "slider.h"
#include "ui_slider.h"

Slider::Slider(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Slider)
{
    ui->setupUi(this);
}

Slider::~Slider()
{
    delete ui;
}
