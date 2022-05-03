#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "Headers/Shape.h"

class Triangle : public Shape
{
public:
    Triangle(float r, float g, float b) : Shape(r, g, b)
    {
        Triangle::InitGeometry();
    }

    void InitGeometry()
    {
        QVector3D vertices[] = {
            QVector3D(-0.5f, -0.5f, 0.0f),
            QVector3D(0.5f, -0.5f, 0.0f),
            QVector3D(0.0f, 0.5f, 0.0f)
        };

        GLushort indices[] = {
            0, 1, 2
        };

        QVector3D normals[] = {
            QVector3D(0.0f, 0.0f, -1.0f),
            QVector3D(0.0f, 0.0f, -1.0f),
            QVector3D(0.0f, 0.0f, -1.0f)
        };

        indexCount = 3;

        arrayBuf.bind();
        arrayBuf.allocate(vertices, 3 * sizeof(QVector3D));

        indexBuf.bind();
        indexBuf.allocate(indices, indexCount * sizeof(GLushort));

        normalBuf.bind();
        normalBuf.allocate(normals, 3 * sizeof(QVector3D));
    }
};

#endif // TRIANGLE_H
