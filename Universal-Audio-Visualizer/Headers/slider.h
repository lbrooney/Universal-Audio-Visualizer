#ifndef SLIDER_H
#define SLIDER_H

#include <QDialog>
#include "audiosystem.h"
#include "oglwidget.h"


namespace Ui {
class Slider;
}

class Slider : public QDialog
{
    Q_OBJECT

public:
    explicit Slider(QWidget *parent = nullptr, AudioSystem *p = nullptr, OGLWidget *ogl = nullptr);
    ~Slider();

private slots:
    void on_volumeSlider_sliderMoved(int position);

    void on_scaleSlider_sliderMoved(int position);

    void on_checkBox_toggled(bool checked);

private:
    OGLWidget* openGLWidget = nullptr;
    AudioSystem *pSystem = nullptr;
    Ui::Slider *ui;
    void VolumeSetup();
    void ScaleSetup();
    void UpdateText();
    void UpdateColor();
};

#endif // SLIDER_H
