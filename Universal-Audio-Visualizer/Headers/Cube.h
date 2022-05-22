#ifndef CUBE_H
#define CUBE_H
#include "Shape.h"

class Cube : public Shape
{
public:
    Cube() : Shape()
    {
        Cube::InitGeometry();
        indexCount = 36;
    }

    Cube(float r, float g, float b) : Shape(r, g, b)
    {
        Cube::InitGeometry();
        indexCount = 36;
    }

    Cube(const Cube &c) : Shape(c)
    {
        indexCount = 36;
        Cube::InitGeometry();
    }

    void InitGeometry()
    {
        QVector3D vertices[] = {
            //front
            QVector3D(0.5f, 0.5f, 0.5f),    //v0
            QVector3D(-0.5f, 0.5f, 0.5f),   //v1
            QVector3D(-0.5f, -0.5f, 0.5f),  //v2
            QVector3D(0.5f, -0.5f, 0.5f),   //v3
            //right
            QVector3D(0.5f, 0.5f, -0.5f),   //v4
            QVector3D(0.5f, 0.5f, 0.5f),    //v5
            QVector3D(0.5f, -0.5f, 0.5f),   //v6
            QVector3D(0.5f, -0.5f, -0.5f),  //v7
            //back
            QVector3D(-0.5f, 0.5f, -0.5f),  //v8
            QVector3D(0.5f, 0.5f, -0.5f),   //v9
            QVector3D(0.5f, -0.5f, -0.5f),  //v10
            QVector3D(-0.5f, -0.5f, -0.5f), //v11
            //left
            QVector3D(-0.5f, 0.5f, 0.5f),   //v12
            QVector3D(-0.5f, 0.5f, -0.5f),  //v13
            QVector3D(-0.5f, -0.5f, -0.5f), //v14
            QVector3D(-0.5f, -0.5f, 0.5f),  //v15
            //top
            QVector3D(0.5f, 0.5f, -0.5f),   //v16
            QVector3D(-0.5f, 0.5f, -0.5f),  //v17
            QVector3D(-0.5f, 0.5f, 0.5f),   //v18
            QVector3D(0.5f, 0.5f, 0.5f),    //v19
            //bottom
            QVector3D(0.5f, -0.5f, 0.5f),   //v20
            QVector3D(-0.5f, -0.5f, 0.5f),  //v21
            QVector3D(-0.5f, -0.5f, -0.5f), //v22
            QVector3D(0.5f, -0.5f, -0.5f),  //v23
        };

        GLushort indices[] = {
             0, 1, 3, 3, 1, 2,//front
             4, 5, 7, 7, 5, 6,//right
             8, 9, 11, 11, 9, 10,//back
             12, 13, 15, 15, 13, 14,//left
             16, 17, 19, 19, 17, 18,//top
             20, 21, 23, 23, 21, 22//bottom
        };

        QVector3D normals[] = {
            //front
            QVector3D(0.0f, 0.0f, 1.0f),  //v0
            QVector3D(0.0f, 0.0f, 1.0f),  //v1
            QVector3D(0.0f, 0.0f, 1.0f),  //v2
            QVector3D(0.0f, 0.0f, 1.0f),  //v3
            //right
            QVector3D(1.0f, 0.0f, 0.0f),  //v4
            QVector3D(1.0f, 0.0f, 0.0f),  //v5
            QVector3D(1.0f, 0.0f, 0.0f),  //v6
            QVector3D(1.0f, 0.0f, 0.0f),  //v7
            //back
            QVector3D(0.0f, 0.0f, -1.0f), //v8
            QVector3D(.0f, 0.0f, -1.0f),  //v9
            QVector3D(.0f, 0.0f, -1.0f),  //v10
            QVector3D(.0f, 0.0f, -1.0f),  //v11
            //left
            QVector3D(-1.0f, 0.0f, 0.0f), //v12
            QVector3D(-1.0f, 0.0f, 0.0f), //v13
            QVector3D(-1.0f, 0.0f, 0.0f), //v14
            QVector3D(-1.0f, 0.0f, 0.0f), //v15
            //top
            QVector3D(0.0f, 1.0f, 0.0f),  //v16
            QVector3D(0.0f, 1.0f, 0.0f),  //v17
            QVector3D(0.0f, 1.0f, 0.0f),  //v18
            QVector3D(0.0f, 1.0f, 0.0f),  //v19
            //bottom
            QVector3D(0.0f, -1.0f, 0.0f), //v20
            QVector3D(0.0f, -1.0f, 0.0f), //v21
            QVector3D(0.0f, -1.0f, 0.0f), //v22
            QVector3D(0.0f, -1.0f, 0.0f), //v23
        };

        arrayBuf.bind();
        arrayBuf.allocate(vertices, 24 * sizeof(QVector3D));

        indexBuf.bind();
        indexBuf.allocate(indices, 36 * sizeof(GLushort));

        normalBuf.bind();
        normalBuf.allocate(normals, 24 * sizeof(QVector3D));
    }
};

#endif // CUBE_H
