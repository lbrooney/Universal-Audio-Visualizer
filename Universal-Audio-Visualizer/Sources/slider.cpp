#include "slider.h"
#include "ui_slider.h"
#include <QLabel>
#include <QGridLayout>
#include "Audio/audiorecorder.h"

#define DEFAULT 30

extern double myTempo;

Slider::Slider(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Slider)
{
    ui->setupUi(this);
    setWindowTitle("Sliders");
    pInterface = ((MainWindow*)parent)->getAudioInterface();
    openGLWidget = ((MainWindow*)parent)->getOGLWidget();
    if(pInterface != nullptr) {
        pRecorder = pInterface->getRecorder();
    }
    else
    {
        std::cout << "ERROR" << std::endl;
    }

    ui->textBrowser->setText(QString::number(myTempo));
    volumeSetup();
    scaleSetup();
}

void Slider::scaleSetup()
{
    auto* ptr = ui->scaleSlider;
    ptr->setRange(0, 100);
    ptr->setValue(DEFAULT);
    ptr->setTracking(true);
}

void Slider::volumeSetup()
{
    auto *ptr = ui->volumeSlider;
    float volume = pRecorder->GetVolume() * 100;
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
    float result = pRecorder->SetVolume(vol);
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
    ui->textBrowser->setText(QString::number(myTempo));
}
