#ifndef OGLWIDGET_H
#define OGLWIDGET_H
#include <random>
#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <gl/GLU.h>
#include <gl/GL.h>
#include <QVector3D>
#include <vector>
#include <QTimer>
#include <fftw3.h>
#include "Shape.h"
#include "Triangle.h"
#include "Square.h"
#include "Sphere.h"
#include "Circle.h"
#include "Cube.h"
#include "Cylinder.h"

#include <iostream>

class OGLWidget : public QOpenGLWidget, public QOpenGLExtraFunctions
{
public:
    OGLWidget(QWidget *parent = 0);
    ~OGLWidget();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void initShaders();

private:
    QOpenGLShaderProgram m_program;
    std::vector<Shape*> objList;
    int rotation;

};

#endif // OGLWIDGET_H
