#include "slider.h"
#include "ui_slider.h"
#include <QLabel>
#include <QGridLayout>
#include "Audio/audiorecorder.h"

#define DEFAULT 0.3;

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
    volumeSetup();
}

void Slider::scaleSetup()
{
    auto* ptr = ui->scaleSlider;
    float base = DEFAULT;
    ptr->setRange(0, 100);
    ptr->setValue(base * 100);
    ptr->setTracking(true);
}

void Slider::volumeSetup()
{
    auto *ptr = ui->volumeSlider;
    float volume = pRecorder->GetVolume() * 100;
    //std::cout << "line 28 " << volume << std::endl;
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
