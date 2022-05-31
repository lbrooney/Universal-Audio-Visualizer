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
#include <QObject>
#include <thread>
#include <atomic>

#include "Shapes/Shape.h"
#include "Shapes/Sphere.h"
#include "Shapes/Cube.h"
#include "Shapes/Prism.h"
#include "audiosystem.h"

const float DEFAULTINTENSITY = 0.5f;
const uint8_t SHAPEUPDATECYCLE = 10;
const uint8_t SPECTRUMUPDATECYCLE = 3;

class OGLWidget : public QOpenGLWidget, public QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    OGLWidget(QWidget *parent = nullptr, AudioSystem* p = nullptr);
    void loadPreset(int preset);
    void oglsetScale(float scale);
    ~OGLWidget();
    QVector3D rgbSelector;
    QVector3D determineColor(float bpm);
public slots:
    void playBeat();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void InitShaders();

private:
    AudioSystem* pSystem;

    int drawCycleCount = 0;
    double maxMagnitude = 10.0;
    bool showSpectrum = false;

    float scale = 0.3;
    QTimer* beatTimer;
    bool playBeatAnim = false;

    QOpenGLShaderProgram shaderProgram;
    std::vector<Shape*> objList;
    glm::mat4 perspectiveMatrix;
    glm::mat4 viewMatrix;

    void CreateSphere(float r, float g, float b, int frequency);
    void CreateCube(float r, float g, float b, int frequency);
    void CreatePrism(float r, float g, float b, int frequency);
};

#endif // OGLWIDGET_H
