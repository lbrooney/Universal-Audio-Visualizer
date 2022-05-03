#ifndef CIRCLE_H
#define CIRCLE_H
#include "Headers/Shape.h"
#include <vector>

class Circle : public Shape
{
public:
    Circle(float r, float g, float b) : Shape(r, g, b)
    {
        Circle::InitGeometry();
    }

    void InitGeometry()
    {
        int segmentCount = 50;
        indexCount = segmentCount * 3;
        QVector3D* vertices = new QVector3D[segmentCount];
        QVector3D* normals = new QVector3D[segmentCount];
        GLushort* indices = new GLushort[indexCount];

        //calculate vertices
        float angle = 2 * 3.1416f / segmentCount;
        float radius = 0.5f;
        vertices[0] = QVector3D(0, 0, 0);
        for(int i = 0; i < segmentCount; i++)
        {
            float x = radius * cos(i * angle);
            float y = radius * sin(i * angle);
            vertices[i] = QVector3D(x, y, 0.0f);
            normals[i] = QVector3D(0.0f, 0.0f, -1.0f);
        }

        //calculate indices
        int counter = 1;
        for(int i = 0; i < indexCount; i += 3)
        {
            indices[i] = 0;
            indices[i + 1] = counter++;
            indices[i + 2] = counter;
        }

        arrayBuf.bind();
        arrayBuf.allocate(vertices, segmentCount * sizeof(QVector3D));

        indexBuf.bind();
        indexBuf.allocate(indices, indexCount * sizeof(GLushort));

        normalBuf.bind();
        normalBuf.allocate(normals, segmentCount * sizeof(QVector3D));

        delete[] vertices;
        delete[] indices;
        delete[] normals;
    }
};
#endif // CIRCLE_H
