#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <gl/GLU.h>
#include <gl/GL.h>


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
    unsigned int m_vao;
    unsigned int m_vbo;
    QOpenGLShaderProgram m_program;
};

#endif // OGLWIDGET_H
