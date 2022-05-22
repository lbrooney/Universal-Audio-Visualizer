#ifndef SLIDER_H
#define SLIDER_H

#include <QWidget>

namespace Ui {
class Slider;
}

class Slider : public QWidget
{
    Q_OBJECT

public:
    explicit Slider(QWidget *parent = nullptr);
    ~Slider();

private:
    Ui::Slider *ui;
};

#endif // SLIDER_H
