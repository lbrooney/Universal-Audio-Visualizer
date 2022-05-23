#ifndef SLIDER_H
#define SLIDER_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class Slider;
}

class Slider : public QDialog
{
    Q_OBJECT

public:
    explicit Slider(QWidget *parent = nullptr);
    ~Slider();

private slots:
    void on_volumeSlider_sliderMoved(int position);

    void on_scaleSlider_sliderMoved(int position);

    void on_pushButton_clicked();

private:
    OGLWidget* openGLWidget = nullptr;
    AudioInterface* pInterface = nullptr;
    AudioRecorder* pRecorder = nullptr;
    Ui::Slider *ui;
    void volumeSetup();
    void scaleSetup();
};

#endif // SLIDER_H