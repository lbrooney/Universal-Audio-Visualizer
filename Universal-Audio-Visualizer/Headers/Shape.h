#ifndef SHAPE_H
#define SHAPE_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <gl/GLU.h>
#include <gl/GL.h>
#include <lib/glm/glm/glm.hpp>
#include <lib/glm/glm/gtc/matrix_transform.hpp>
#include <lib/glm/glm/gtc/type_ptr.hpp>
#include <QVector3D>
#include <iostream>

class Shape : public QOpenGLFunctions
{
public:
    Shape(QVector3D color)
    {
        m_ModelMatrix = glm::mat4(1.0f);
        indexBuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        m_Color = color;
        initializeOpenGLFunctions();

        arrayBuf.create();
        indexBuf.create();
        normalBuf.create();

        m_Scale = QVector3D(1.0f, 1.0f, 1.0f);
        //m_Rotation = QVector3D(1.0f, 1.0f, 1.0f);
        m_Position = QVector3D(0.0f, 0.0f, 0.0f);

        scaleFactor = 1.05f;
    }
    virtual ~Shape()
    {
        arrayBuf.destroy();
        indexBuf.destroy();
        normalBuf.destroy();
    }

    virtual void InitGeometry() = 0;
    void DrawShape(QOpenGLShaderProgram *program)
    {
        glUseProgram(program->programId());

        indexBuf.bind();

        unsigned int u_ModelMatrix = glGetUniformLocation(program->programId(), "u_ModelMatrix");
        glUniformMatrix4fv(u_ModelMatrix, 1, GL_FALSE, glm::value_ptr(m_ModelMatrix));

        m_NormalMatrix = glm::transpose(glm::inverse(m_ModelMatrix));

        unsigned int u_NormalMatrix = glGetUniformLocation(program->programId(), "u_NormalMatrix");
        glUniformMatrix4fv(u_NormalMatrix, 1, GL_FALSE, glm::value_ptr(m_NormalMatrix));

        unsigned int u_Color = glGetUniformLocation(program->programId(), "u_Color");
        glUniform3f(u_Color, m_Color.x(), m_Color.y(), m_Color.z());

        arrayBuf.bind();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        normalBuf.bind();
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
    }

    void SetTranslation(glm::vec3 translation)
    {
        m_Position.setX(m_Position.x() + translation.x);
        m_Position.setY(m_Position.y() + translation.y);
        m_Position.setZ(m_Position.z() + translation.z);
        m_ModelMatrix = glm::translate(m_ModelMatrix, translation);
    }
    void SetRotation(float rotDegree, glm::vec3 axis)
    {
        m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(rotDegree), axis);
    }
    void SetScale(glm::vec3 scale)
    {
        m_Scale.setX(scale.x * m_Scale.x());
        m_Scale.setY(scale.y * m_Scale.y());
        m_Scale.setZ(scale.z * m_Scale.z());
        m_ModelMatrix = glm::scale(m_ModelMatrix, scale);
    }

    glm::mat4 m_ModelMatrix;
    glm::mat4 m_NormalMatrix;
    QVector3D m_Color;
    //QVector3D m_Rotation;
    QVector3D m_Scale;
    QVector3D m_Position;
    float scaleFactor;

protected:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    QOpenGLBuffer normalBuf;

    int indexCount;
};

#endif // SHAPE_H
