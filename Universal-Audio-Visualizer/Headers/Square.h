#ifndef SQUARE_H
#define SQUARE_H
#include "Headers/Shape.h"

class Square : public Shape
{
public:
    Square()
    {
        Square::CreateVertexArray();
        Square::CreateIndexArray();
    }

    void CreateIndexArray()
    {
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(3);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(3);
    }

    void CreateVertexArray()
    {
        vertices.push_back(QVector3D(0.5f, 0.5f, 0.0f));
        vertices.push_back(QVector3D(0.5f, -0.5f, 0.0f));
        vertices.push_back(QVector3D(-0.5f, -0.5f, 0.0f));
        vertices.push_back(QVector3D(-0.5f, 0.5f, 0.0f));
    }
};

#endif // SQUARE_H
