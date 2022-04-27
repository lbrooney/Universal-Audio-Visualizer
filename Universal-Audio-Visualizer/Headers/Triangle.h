#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "Headers/Shape.h"

class Triangle : public Shape
{
public:
    Triangle()
    {
        Triangle::CreateVertexArray();
        Triangle::CreateIndexArray();
    }

    void CreateIndexArray()
    {
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(3);
    }

    void CreateVertexArray()
    {
        vertices.push_back(QVector3D(-0.5f, -0.5f, 0.0f));
        vertices.push_back(QVector3D(0.5f, -0.5f, 0.0f));
        vertices.push_back(QVector3D(0.0f, 0.5f, 0.0f));
    }
};

#endif // TRIANGLE_H
