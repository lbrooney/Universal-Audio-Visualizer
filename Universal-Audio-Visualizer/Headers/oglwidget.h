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

#include <thread>
#include <atomic>

#include "Shape.h"
#include "Sphere.h"
#include "Cube.h"
#include "Prism.h"
#include "Triangle.h"
#include "Square.h"
#include "Circle.h"
#include "audiosystem.h"

const float DEFAULTINTENSITY = 0.5f;

class OGLWidget : public QOpenGLWidget, public QOpenGLExtraFunctions
{
public:
    OGLWidget(QWidget *parent = nullptr, AudioSystem* p = nullptr);
    void loadPreset(int preset);
    void oglsetScale(float scale);
    ~OGLWidget();
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void initShaders();
    QVector3D determineColor(float bpm);

private:
    AudioSystem* pSystem;

    QOpenGLShaderProgram mProgram;
    std::vector<Shape*> objList;

    int drawCycleCount = 0;
    double maxMagnitude = 10.0;
    bool showSpectrum = false;
    float scale = 0.3;

    glm::mat4 m_PerspectiveMatrix;
    glm::mat4 m_ViewMatrix;
    void createSphere(float r, float g, float b);
    void createCube(float r, float g, float b);
    void createPrism(float r, float g, float b);
};

#endif // OGLWIDGET_H
