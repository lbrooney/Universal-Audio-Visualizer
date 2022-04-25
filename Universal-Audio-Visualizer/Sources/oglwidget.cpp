#include "oglwidget.h"
#include "Triangle.h"
#include "Square.h"
#include <QVector3D>
#include <iostream>

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
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();

    Triangle t = Triangle();
    Square s = Square();

    //s.SetTranslation(glm::vec3(-0.5f, 0.5f, 0.0f));

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, s.vertexCount * sizeof(float), s.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, s.indexCount * sizeof(unsigned int), s.indices, GL_STATIC_DRAW);

    unsigned int transformLoc = glGetUniformLocation(m_program.programId(), "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(s.transformMatrix));
}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_program.programId());
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
