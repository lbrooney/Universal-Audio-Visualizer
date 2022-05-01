#include "oglwidget.h"
#include "Triangle.h"
#include "Square.h"
#include "Sphere.h"
#include "Circle.h"
#include "Cube.h"
#include "Cylinder.h"
#include <QVector3D>
#include <iostream>

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

OGLWidget::~OGLWidget()
{
    for(int i = objList.size() - 1; i >= 0; i--)
    {
        delete objList[i];
    }
}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();

    Triangle* t = new Triangle(QVector3D(1.0f, 0.0f, 0.0f));
    //t->SetRotation(-45, glm::vec3(0.0f, 1.0f, 0.0f));
    t->SetTranslation(glm::vec3(-0.5f, 0.5f, 0.0f));
    t->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

    Square* s = new Square(QVector3D(0.0f, 1.0f, 0.0f));
    s->SetTranslation(glm::vec3(-0.5f, -0.5f, 0.0f));
    s->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

    Circle* c = new Circle(QVector3D(0.0f, 0.0f, 1.0f));
    c->SetTranslation(glm::vec3(0.5f, -0.5f, 0.0f));
    c->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));


    //objList.push_back(t);
    //objList.push_back(s);
    //objList.push_back(c);

    Cube* cube = new Cube(QVector3D(1.0f, 1.0f, 0.0f));
    cube->SetRotation(15, glm::vec3(1.0f, 1.0f, 0.0f));
    cube->SetTranslation(glm::vec3(0.5f, 0.5f, 0.0f));
    cube->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

    Sphere* sphere = new Sphere(QVector3D(1.0f, 0.0f, 0.0f));
    //sphere->SetTranslation(glm::vec3(0.5f, 0.5f, 0.0f));
    sphere->SetScale(glm::vec3(0.25f, 0.25f, 0.25f));

    Cylinder* cyl = new Cylinder(QVector3D(0.0f, 1.0f, 1.0f), 3);
    cyl->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
    cyl->SetRotation(-45, glm::vec3(0.0f, 1.0f, 0.0f));

    //objList.push_back(cube);
    //objList.push_back((sphere));
    objList.push_back(cyl);

}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int u_lightColor = glGetUniformLocation(m_program.programId(), "u_lightColor");
    glUniform3f(u_lightColor, 1.0f, 1.0f, 1.0f);
    unsigned int u_lightDirection = glGetUniformLocation(m_program.programId(), "u_lightDirection");
    glUniform3f(u_lightDirection, 1.0f, 1.0f, -1.0f);

    for(int i = 0; i < objList.size(); i++)
    {
        objList[i]->DrawShape(&m_program);
    }

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
