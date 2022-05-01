#ifndef SQUARE_H
#define SQUARE_H
#include "Headers/Shape.h"

class Square : public Shape
{
public:
    Square(QVector3D color) : Shape(color)
    {
        Square::InitGeometry();
    }

    void InitGeometry()
    {
        QVector3D vertices[] = {
            QVector3D(0.5f, 0.5f, 0.0f),
            QVector3D(0.5f, -0.5f, 0.0f),
            QVector3D(-0.5f, -0.5f, 0.0f),
            QVector3D(-0.5f, 0.5f, 0.0f)
        };

        GLushort indices[] = {
            0, 1, 3,
            1, 2, 3
        };
        indexCount = 6;

        QVector3D normals[] = {
            QVector3D(0.0f, 0.0f, -1.0f),
            QVector3D(0.0f, 0.0f, -1.0f),
            QVector3D(0.0f, 0.0f, -1.0f),
            QVector3D(0.0f, 0.0f, -1.0f)
        };

        arrayBuf.bind();
        arrayBuf.allocate(vertices, 4 * sizeof(QVector3D));

        indexBuf.bind();
        indexBuf.allocate(indices, indexCount * sizeof(GLushort));

        normalBuf.bind();
        normalBuf.allocate(normals, 4 * sizeof(QVector3D));
    }
};

#endif // SQUARE_H
