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
    Shape(float r, float g, float b)
    {
        m_ModelMatrix = glm::mat4(1.0f);
        indexBuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        m_Color = QVector3D(r, g, b);
        initializeOpenGLFunctions();

        arrayBuf.create();
        indexBuf.create();
        normalBuf.create();

        m_Scale = glm::vec3(1.0f, 1.0f, 1.0f);
        m_Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    Shape(const Shape &source)
    {
        initializeOpenGLFunctions();
        m_Color = source.m_Color;
        m_ModelMatrix = source.m_ModelMatrix;
        arrayBuf = source.arrayBuf;
        indexBuf = source.indexBuf;
        normalBuf = source.normalBuf;
        indexCount = source.indexCount;
        m_Scale = source.m_Scale;
        m_Rotation = source.m_Rotation;
        m_Position = source.m_Position;
        freqBin = source.freqBin;
        std::cout << "Copy constructor called\n";
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

        m_ModelMatrix = glm::mat4(1.0f);
        m_ModelMatrix = glm::translate(m_ModelMatrix, m_Position);
        m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(m_Rotation.x), glm::vec3(1, 0, 0));
        m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
        m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
        m_ModelMatrix = glm::scale(m_ModelMatrix, m_Scale);

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

    void SetTranslation(float x, float y, float z)
    {
        m_Position = glm::vec3(x, y, z);
    }
    void SetRotation(float x, float y, float z)
    {
        m_Rotation = glm::vec3(x, y, z);
    }
    void SetScale(float x, float y, float z)
    {
        m_Scale = glm::vec3(x, y, z);
    }
    void SetColor(float r, float g, float b)
    {
        m_Color = QVector3D(r, g, b);
    }
    void AssignFrequencyBin(int freq, DWORD sampleRate, int blockSize)
    {
        float freqStep = sampleRate / (float)blockSize;
        freqBin = round(freq / freqStep);
    }

    glm::mat4 m_ModelMatrix;
    glm::mat4 m_NormalMatrix;
    QVector3D m_Color;
    glm::vec3 m_Rotation;
    glm::vec3 m_Scale;
    glm::vec3 m_Position;
    int freqBin = 0;

protected:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    QOpenGLBuffer normalBuf;
    int indexCount;
};

#endif // SHAPE_H
