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
#include "Audio/audiointerface.h"

const float DEFAULTINTENSITY = 0.5f;

class OGLWidget : public QOpenGLWidget, public QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    OGLWidget(QWidget *parent = nullptr, AudioInterface* p = nullptr);
    void loadPreset(int preset);
    void oglsetScale(float scale);
    ~OGLWidget();

public slots:
    void playBeat();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void initShaders();
    QVector3D determineColor(float bpm);

private:
    AudioInterface* pInterface;
    AudioRecorder* pRecorder;

    QTimer* beatTimer;
    bool playBeatAnim = false;
    int drawCycleCount = 0;
    float maxMagnitude = 10.0f;
    bool displayWaveform = false;
    float defaultScale = 0.3;

    QOpenGLShaderProgram shaderProgram;
    std::vector<Shape*> objList;
    glm::mat4 perspectiveMatrix;
    glm::mat4 viewMatrix;

    void createSphere(float r, float g, float b);
    void createCube(float r, float g, float b);
    void createPrism(float r, float g, float b);
};

#endif // OGLWIDGET_H
