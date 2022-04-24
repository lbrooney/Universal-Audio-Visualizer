#include "oglwidget.h"
#include <QVector3D>

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

OGLWidget::~OGLWidget()
{

}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5f,0.5f,0.5f,1.0f);

    initShaders();

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
        };

    GLushort indices[] = {
        0, 1, 2
    };

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_program.programId());
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void OGLWidget::resizeGL(int w, int h)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(0, 0, w, h);
}

void OGLWidget::initShaders()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vertex.glsl");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fragment.glsl");
    m_program.link();
    m_program.bind();
}
