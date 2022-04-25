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

    ~Square()
    {
        delete[] indices;
        delete[] vertices;
    }
    void CreateIndexArray()
    {
         indices = new unsigned int[6];
         indices[0] = 0;
         indices[1] = 1;
         indices[2] = 3;
         indices[3] = 1;
         indices[4] = 2;
         indices[5] = 3;
    }

    void CreateVertexArray()
    {
        vertices = new float[12];
        //vertex 0
        vertices[0] = 0.5f;
        vertices[1] = 0.5f;
        vertices[2] = 0.0f;

        //vertex 1
        vertices[3] = 0.5f;
        vertices[4] = -0.5f;
        vertices[5] = 0.0f;

        //vertex 2
        vertices[6] = -0.5f;
        vertices[7] = -0.5f;
        vertices[8] = 0.0f;

        //vertex 3
        vertices[9] = -0.5f;
        vertices[10] = 0.5f;
        vertices[11] = 0.0f;

    }

    int indexCount = 6;
    int vertexCount = 12;
};

#endif // SQUARE_H
