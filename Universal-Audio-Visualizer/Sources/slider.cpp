#include "slider.h"
#include "ui_slider.h"
#include <QLabel>
#include <QGridLayout>
#include <QTimer>

const int DEFAULTSIZE = 30;

extern bool redChecked;
extern bool blueChecked;
extern bool greenChecked;
extern bool selectedColor;

Slider::Slider(QWidget *parent, AudioSystem *p, OGLWidget *ogl) :
    QDialog(parent),
    ui(new Ui::Slider),
    pSystem(p),
    openGLWidget(ogl)
{
    ui->setupUi(this);

    setWindowTitle("Sliders");
    scaleSetup();
    volumeSetup();

    if (openGLWidget->rgbSelector != QVector3D(1, 1, 1) && selectedColor == false)
    {
        ui->checkBox->setCheckState(Qt::Checked);
    }
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Slider::UpdateText);
    timer->start();
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

void Slider::UpdateText()
{
    ui->textBrowser->setText(QString::number(pSystem->GetBPM()));
}

void Slider::UpdateColor()
{
    // (Off -> On)
    if (ui->checkBox->isChecked())
    {
        openGLWidget->rgbSelector = openGLWidget->determineColor(pSystem->GetBPM());
    }
    // (On -> Off)
    else
    {
       if (redChecked)
       {
           openGLWidget->rgbSelector = QVector3D(1, 0, 0);
       }
       else if (blueChecked)
       {
           openGLWidget->rgbSelector = QVector3D(0, 0, 1);
       }
       else if (greenChecked)
       {
           openGLWidget->rgbSelector = QVector3D(0, 1, 0);
       }
       else
       {
           openGLWidget->rgbSelector = QVector3D(1, 1, 1); // Default color is white
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

