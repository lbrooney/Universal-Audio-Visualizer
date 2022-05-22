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

private:
    AudioInterface* pInterface = nullptr;
    AudioRecorder* pRecorder = nullptr;
    Ui::Slider *ui;
    void volumeSetup();
};

#endif // SLIDER_H
