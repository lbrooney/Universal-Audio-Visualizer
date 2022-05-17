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
#include "AudioRecorder.h"

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
    void loadPreset(int preset);

private:
    QOpenGLShaderProgram m_program;
    std::vector<Shape*> objList;
    AudioRecorder* m_Recorder;

    std::thread recording_thread;
    std::atomic_bool exit_recording_thread_flag = false;
    int drawCycleCount = 0;

    glm::mat4 m_PerspectiveMatrix;
    glm::mat4 m_ViewMatrix;
};

#endif // OGLWIDGET_H
