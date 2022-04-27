#include "oglwidget.h"
#include "Triangle.h"
#include "Square.h"
#include "Circle.h"
#include <QVector3D>
#include <iostream>

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

OGLWidget::~OGLWidget()
{
    /*for(int i = objList.size() - 1; i >= 0; i--)
    {
        delete objList[i];
    }*/
}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();

    Triangle* t = new Triangle();
    t->SetTranslation(glm::vec3(0.5f, 0.5f, 0.0f));
    t->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

    Square* s = new Square();
    s->SetTranslation(glm::vec3(-0.5f, 0.5f, 0.0f));
    s->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

    Circle* c = new Circle();
    c->SetTranslation(glm::vec3(0.0f, -0.5f, 0.0f));
    c->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));


    objList.push_back(t);
    objList.push_back(s);
    objList.push_back(c);



    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_program.programId());
    glBindVertexArray(VAO);

    for(int i = 0; i < objList.size(); i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, objList[i]->vertices.size() * sizeof(QVector3D), objList[i]->vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, objList[i]->indices.size() * sizeof(unsigned int), objList[i]->indices.data(), GL_STATIC_DRAW);

        unsigned int transformLoc = glGetUniformLocation(m_program.programId(), "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(objList[i]->transformMatrix));

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawElements(GL_TRIANGLES, objList[i]->indices.size(), GL_UNSIGNED_INT, 0);
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
