#ifndef CIRCLE_H
#define CIRCLE_H
#include "Headers/Shape.h"

class Circle : public Shape
{
public:
    Circle()
    {
        Circle::CreateVertexArray();
        Circle::CreateIndexArray();
    }

    void CreateIndexArray()
    {
        for(int i = 1; i < vertices.size(); i++)
        {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }

    void CreateVertexArray()
    {
        int segmentCount = 50;
        float angle = 2 * 3.1416f / segmentCount;
        float radius = 0.5f;
        vertices.push_back(QVector3D(0, 0, 0));
        for(int i = 0; i <= segmentCount; i++)
        {
            float x = radius * cos(i * angle);
            float y = radius * sin(i * angle);
            vertices.push_back((QVector3D(x, y, 0.0f)));
        }
    }
};
#endif // CIRCLE_H
