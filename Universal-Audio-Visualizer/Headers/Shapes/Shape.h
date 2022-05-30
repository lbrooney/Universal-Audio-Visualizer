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

class Prism;

class Shape : public QOpenGLFunctions
{
public:
    Shape()
    {
        modelMatrix = glm::mat4(1.0f);
        indexBuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        color = QVector3D(1.0f, 1.0f, 1.0f);
        initializeOpenGLFunctions();

        arrayBuf.create();
        indexBuf.create();
        normalBuf.create();

        scale = glm::vec3(1.0f, 1.0f, 1.0f);
        rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        position = glm::vec3(0.0f, 0.0f, 0.0f);

        intensityScale = 1.0f;
    }

    Shape(float r, float g, float b)
    {
        modelMatrix = glm::mat4(1.0f);
        indexBuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        color = QVector3D(r, g, b);
        initializeOpenGLFunctions();

        arrayBuf.create();
        indexBuf.create();
        normalBuf.create();

        scale = glm::vec3(1.0f, 1.0f, 1.0f);
        rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        position = glm::vec3(0.0f, 0.0f, 0.0f);

        intensityScale = 1.0f;
    }

    Shape(const Shape &source)
    {
        initializeOpenGLFunctions();
        color = source.color;
        modelMatrix = source.modelMatrix;
        arrayBuf = source.arrayBuf;
        indexBuf = source.indexBuf;
        normalBuf = source.normalBuf;
        indexCount = source.indexCount;
        scale = source.scale;
        rotation = source.rotation;
        position = source.position;
        freqBin = source.freqBin;
        intensityScale = source.intensityScale;
        magnitude = source.magnitude;
    }

    virtual ~Shape()
    {
        arrayBuf.destroy();
        indexBuf.destroy();
        normalBuf.destroy();
    }

    /*virtual Shape& operator=(const Shape &source)
    {
        if(this == &source)
           return *this;

        initializeOpenGLFunctions();
        color = source.color;
        modelMatrix = source.modelMatrix;
        arrayBuf = source.arrayBuf;
        indexBuf = source.indexBuf;
        normalBuf = source.normalBuf;
        indexCount = source.indexCount;
        scale = source.scale;
        rotation = source.rotation;
        position = source.position;
        freqBin = source.freqBin;
        intensityScale = source.intensityScale;
        magnitude = source.magnitude;

        return *this;
    }
    virtual Shape& operator=(const Prism &source)
    {
        return *this;
    }*/

    virtual void InitGeometry() = 0;

    void DrawShape(QOpenGLShaderProgram *program)
    {
        glUseProgram(program->programId());

        indexBuf.bind();

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        modelMatrix = glm::scale(modelMatrix, scale);

        unsigned int u_ModelMatrix = glGetUniformLocation(program->programId(), "u_ModelMatrix");
        glUniformMatrix4fv(u_ModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        normalMatrix = glm::transpose(glm::inverse(modelMatrix));

        unsigned int u_NormalMatrix = glGetUniformLocation(program->programId(), "u_NormalMatrix");
        glUniformMatrix4fv(u_NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        unsigned int u_Color = glGetUniformLocation(program->programId(), "u_Color");
        glUniform3f(u_Color, color.x(), color.y(), color.z());

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
        this->position = glm::vec3(x, y, z);
    }
    void SetRotation(float x, float y, float z)
    {
        this->rotation = glm::vec3(x, y, z);
    }
    void SetScale(float s)
    {
        this->scale = glm::vec3(s, s, s);
    }
    void SetScale(float x, float y, float z)
    {
        this->scale = glm::vec3(x, y, z);
    }
    void SetColor(float r, float g, float b)
    {
        this->color = QVector3D(r, g, b);
    }
    void SetColor(QVector3D color)
    {
        this->color = color;
    }
    void AssignFrequencyBin(int freq, DWORD sampleRate, int blockSize)
    {
        float freqStep = sampleRate / (float)blockSize;
        this->freqBin = round(freq / freqStep);
    }

    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;
    QVector3D color;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 position;
    int freqBin = 0;
    float intensityScale;
    float magnitude = 0.0f;

protected:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    QOpenGLBuffer normalBuf;
    int indexCount;
};

#endif // SHAPE_H
