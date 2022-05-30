#include "slider.h"
#include "ui_slider.h"
#include <QLabel>
#include <QGridLayout>
#include <QTimer>
#include "Audio/audiorecorder.h"

#define DEFAULT 30

extern bool redChecked;
extern bool blueChecked;
extern bool greenChecked;
extern bool selectedColor;

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
    // A clever way to save the state of the toggle button ;)
    if (openGLWidget->rgb_selector != QVector3D(1, 1, 1) && selectedColor == false)
    {
        ui->checkBox->setCheckState(Qt::Checked);
    }
    volumeSetup();
    scaleSetup();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Slider::UpdateText);
    timer->start();
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

void Slider::UpdateText()
{
    ui->textBrowser->setText(QString::number(pRecorder->GetBPM()));
}

void Slider::UpdateColor()
{
    // (Off -> On)
    if (ui->checkBox->isChecked())
    {
        openGLWidget->rgb_selector = openGLWidget->determineColor(pInterface->getRecorder()->bpm);
    }
    // (On -> Off)
    else
    {
       if (redChecked)
       {
           openGLWidget->rgb_selector = QVector3D(1, 0, 0);
       }
       else if (blueChecked)
       {
           openGLWidget->rgb_selector = QVector3D(0, 0, 1);
       }
       else if (greenChecked)
       {
           openGLWidget->rgb_selector = QVector3D(0, 1, 0);
       }
       else
       {
           openGLWidget->rgb_selector = QVector3D(1, 1, 1); // Default color is white
       }
    }
}

void Slider::on_checkBox_toggled(bool checked)
{
    selectedColor = (checked) ? false : true;
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Slider::UpdateColor);
    timer->start();
}

