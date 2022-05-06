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
#include <lib/glm/glm/glm.hpp>
#include <lib/glm/glm/gtc/matrix_transform.hpp>
#include <lib/glm/glm/gtc/type_ptr.hpp>
#include <QVector3D>
#include <vector>
#include <QTimer>
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <time.h>
#include <iostream>

#include <fftw3.h>
#include "Shape.h"
#include "Sphere.h"
#include "Cube.h"
#include "Triangle.h"
#include "Square.h"
#include "Circle.h"


#define N 1024

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
    void RecordAudioStream();
    void ProcessData(BYTE* pData);

private:
    QOpenGLShaderProgram m_program;
    std::vector<Shape*> objList;
    double mag[N/2];
    fftw_complex* in;
    fftw_complex* out;
    fftw_plan p;

    glm::mat4 m_PerspectiveMatrix;
    glm::mat4 m_ViewMatrix;
};

#endif // OGLWIDGET_H
