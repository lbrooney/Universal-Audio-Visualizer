#include "slider.h"
#include "ui_slider.h"
#include <QLabel>
#include <QGridLayout>

const int DEFAULTSIZE = 30;

Slider::Slider(QWidget *parent, AudioSystem *p) :
    QDialog(parent),
    ui(new Ui::Slider),
    pSystem(p)
{
    ui->setupUi(this);
    setWindowTitle("Sliders");
    openGLWidget = ((MainWindow*)parent)->getOGLWidget();
    ui->textBrowser->setText(QString::number(pSystem->GetBPM()));
    volumeSetup();
    scaleSetup();
}

void Slider::scaleSetup()
{
    auto* ptr = ui->scaleSlider;
    ptr->setRange(0, 100);
    ptr->setValue(DEFAULTSIZE);
    ptr->setTracking(true);
}

void Slider::volumeSetup()
{
    auto *ptr = ui->volumeSlider;
    float volume = pSystem->GetVolume() * 100;
    ptr->setRange(0, 100);
    ptr->setValue(volume);
    ptr->setTracking(true);
}

Slider::~Slider()
{
    delete ui;
}

void Slider::on_volumeSlider_sliderMoved(int position)
{
    float vol = float(position)/100;
    //std::cout << "pos" << position << std::endl;
    float result = pSystem->SetVolume(vol);
    if(result == -1)
    {
        //std::cout << "error with set volume" << std::endl;
    }
}

void Slider::on_scaleSlider_sliderMoved(int position)
{
    float scale = float(position)/100;
    openGLWidget->oglsetScale(scale);
}

void Slider::on_pushButton_clicked()
{
    ui->textBrowser->setText(QString::number(pSystem->GetBPM()));
}

