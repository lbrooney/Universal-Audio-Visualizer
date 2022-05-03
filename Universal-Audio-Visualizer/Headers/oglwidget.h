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
#include "Cylinder.h"

#define N 10000

class OGLWidget : public QOpenGLWidget, public QOpenGLExtraFunctions
{
public:
    OGLWidget(QWidget *parent = 0);
    ~OGLWidget();
double mag[N/2];
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void initShaders();
    HRESULT RecordAudioStream();
    HRESULT ProcessData(BYTE* pData, UINT32 NumFrames, BOOL* pDone);

private:
    QOpenGLShaderProgram m_program;
    std::vector<Shape*> objList;
};

#endif // OGLWIDGET_H
