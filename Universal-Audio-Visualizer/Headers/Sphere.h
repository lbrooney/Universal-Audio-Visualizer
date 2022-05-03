#ifndef SPHERE_H
#define SPHERE_H
#include "Headers/Shape.h"
#include <vector>

class Sphere : public Shape
{
public:
    Sphere(float r, float g, float b) : Shape(r, g, b)
    {
        Sphere::InitGeometry();
    }

    void InitGeometry()
    {
        int segmentCount = 50;
        indexCount = segmentCount * 3;
        std::vector<QVector3D> vertices;
        std::vector<GLushort> indices;

        // Generate vertices
        for (int j = 0; j <= segmentCount; j++)
        {
            float aj = j * 3.1416f / segmentCount;
            float sj = sin(aj);
            float cj = cos(aj);
                for (int i = 0; i <= segmentCount; i++)
                {
                    float ai = i * 2 * 3.1416f / segmentCount;
                    float si = sin(ai);
                    float ci = cos(ai);

                    vertices.push_back(QVector3D(si * sj / 2, cj / 2, ci * sj / 2));
                }
        }

        // Generate indices
        for (int j = 0; j < segmentCount; j++)
        {
            for (int i = 0; i < segmentCount; i++)
            {
                int p1 = j * (segmentCount + 1) + i;
                int p2 = p1 + (segmentCount + 1);

                indices.push_back(p1);
                indices.push_back(p2);
                indices.push_back(p1 + 1);

                indices.push_back(p1 + 1);
                indices.push_back(p2);
                indices.push_back(p2 + 1);
            }
        }
        indexCount = indices.size();

        arrayBuf.bind();
        arrayBuf.allocate(vertices.data(), vertices.size() * sizeof(QVector3D));

        indexBuf.bind();
        indexBuf.allocate(indices.data(), indices.size() * sizeof(GLushort));

        normalBuf.bind();
        normalBuf.allocate(vertices.data(), vertices.size() * sizeof(QVector3D));
    }
};
#endif // SPHERE_H
