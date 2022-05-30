#include "slider.h"
#include "ui_slider.h"
#include <QLabel>
#include <QGridLayout>
#include <QTimer>

const int DEFAULTSIZE = 30;

Slider::Slider(QWidget *parent, AudioSystem *p) :
    QDialog(parent),
    ui(new Ui::Slider),
    pSystem(p)
{
    ui->setupUi(this);

    setWindowTitle("Sliders");
    /*pInterface = ((MainWindow*)parent)->getAudioInterface();

    if(pInterface != nullptr) {
        pRecorder = pInterface->getRecorder();
    }
    else
    ui->textBrowser->setText(QString::number(pSystem->GetBPM()));
    // A clever way to save the state of the toggle button ;)
    if (openGLWidget->rgbSelector != QVector3D(1, 1, 1))
    {
        ui->checkBox->setCheckState(Qt::Checked);
    }
    volumeSetup();
    */
    openGLWidget = ((MainWindow*)parent)->getOGLWidget();
    scaleSetup();
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
       openGLWidget->rgbSelector = QVector3D(1, 1, 1); // Default color is white
    }
}

void Slider::on_checkBox_toggled(bool checked)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Slider::UpdateColor);
    timer->start();
}

