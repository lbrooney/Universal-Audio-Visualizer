#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <gl/GLU.h>
#include <gl/GL.h>
#include "Shape.h"
#include <vector>


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
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    QOpenGLShaderProgram m_program;
    std::vector<Shape*> objList;
};

#endif // OGLWIDGET_H
