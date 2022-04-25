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

    ~Triangle()
    {
        delete[] indices;
        delete[] vertices;
    }
    void CreateIndexArray()
    {
         indices = new unsigned int[3];
         indices[0] = 0;
         indices[1] = 1;
         indices[2] = 2;
    }

    void CreateVertexArray()
    {
        vertices = new float[9];
        vertices[0] = -0.5f;
        vertices[1] = -0.5f;
        vertices[2] = 0.0f;
        vertices[3] = 0.5f;
        vertices[4] = -0.5f;
        vertices[5] = 0.0f;
        vertices[6] = 0.0f;
        vertices[7] = 0.5f;
        vertices[8] = 0.0f;
    }

    int indexCount = 3;
    int vertexCount = 9;
};

#endif // TRIANGLE_H
