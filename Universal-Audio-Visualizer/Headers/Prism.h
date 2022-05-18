#ifndef PRISM_H
#define PRISM_H
#include "Headers/Shape.h"
#include <vector>

class Prism : public Shape
{
public:
    Prism() : Shape()
    {
        m_SideCount = 3;
        Prism::InitGeometry();
    }

    Prism(float r, float g, float b, int sideCount) : Shape(r, g, b)
    {
        m_SideCount = sideCount;
        Prism::InitGeometry();
    }

    /*Prism(const Prism &p) : Shape(p)
    {
        m_SideCount = p.m_SideCount;
        Prism::InitGeometry();
    }

    virtual Prism& operator=(const Prism& source) override
    {
        if(this == &source)
           return *this;

        m_SideCount = source.m_SideCount;
        Prism::InitGeometry();

        const Shape* base = this;

        Shape::operator=(*base);

        return *this;
    }*/

    void InitGeometry()
    {
        int vertexCount = (m_SideCount * 6 + 2) * 3;
        float* vertices = new float[vertexCount];
        memset(vertices, 0, sizeof(float) * vertexCount);
        //calculate inner angle of cylinder
        float angle = 2 * 3.1416f / m_SideCount;
        int j = 0;
        //get vertex coords for the sides
        for (int i = 0; i < m_SideCount * 4; i += 4) {
            //vertex 1
            vertices[i * 3] = float(cos(angle * j));
            vertices[i * 3 + 1] = float(sin(angle * j));
            vertices[i * 3 + 2] = 0.0f;

            //vertex 2
            vertices[(i + 1) * 3] = float(cos(angle * (j + 1)));
            vertices[(i + 1) * 3 + 1] = float(sin(angle * (j + 1)));
            vertices[(i + 1) * 3 + 2] = 0.0f;

            //vertex 3
            vertices[(i + 2) * 3] = float(cos(angle * (j + 1)));
            vertices[(i + 2) * 3 + 1] = float(sin(angle * (j + 1)));
            vertices[(i + 2) * 3 + 2] = 1.0f;

            //vertex 4
            vertices[(i + 3) * 3] = float(cos(angle * j));
            vertices[(i + 3) * 3 + 1] = float(sin(angle * j));
            vertices[(i + 3) * 3 + 2] = 1.0f;

            j++;
        }

        //get vertex coords for the caps
        float zVal = 0.0f;
        j = 0;
        for (int i = 4 * m_SideCount; i < vertexCount / 3 - 2; i++) {
            if (j >= m_SideCount)
                zVal = 1.0f;
            vertices[i * 3] = float(cos(angle * j));
            vertices[i * 3 + 1] = float(sin(angle * j));
            vertices[i * 3 + 2] = zVal;

            j++;
        }
        //second to last coord will be center for bottom cap, last coord center for top cap
        vertices[vertexCount - 1] = 1.0f;

        indexCount = m_SideCount * 12;
        GLushort* indices = new GLushort[indexCount];
        j = 0;
        int i = 0;
        //indices for the side polygons
        for (i = 0; i < indexCount / 2; i += 6) {
            //polygon 1
            indices[i] = j;
            indices[i + 1] = j + 2;
            indices[i + 2] = j + 3;

            //polygon 2
            indices[i + 3] = j;
            indices[i + 4] = j + 1;
            indices[i + 5] = j + 2;
            j += 4;
        }
        //bottom cap
        for (i = indexCount / 2; i < indexCount * 3 / 4; i += 3) {
            indices[i] = j;
            indices[i + 1] = vertexCount / 3 - 2;
            indices[i + 2] = j + 1;
            j++;
        }
        //fix last index for the cap
        indices[indexCount * 3 / 4 - 1] = 4 * m_SideCount;

        //top cap
        for (i = indexCount * 3 / 4; i < indexCount; i += 3) {
            indices[i] = j;
            indices[i + 1] = vertexCount / 3 - 1;
            indices[i + 2] = j + 1;
            j++;
        }
        //fix last index for the cap
        indices[indexCount - 1] = 4 * m_SideCount + m_SideCount;


        float* normals = new float[vertexCount];
        memset(normals, 0, sizeof(float) * vertexCount);
        j = 0;
        i = 0;

        //side normals
        //calculates the normal for one polygon and assigns it to the vertices
        //that make up that polygon
        for (i = 0; i < m_SideCount * 4; i += 4) {
            QVector3D a = QVector3D(
                        vertices[(i + 1) * 3] - vertices[i * 3],
                    vertices[(i + 1) * 3 + 1] - vertices[i * 3 + 1],
                    vertices[(i + 1) * 3 + 2] - vertices[i * 3 + 2]
                    );
            QVector3D b = QVector3D(
                        vertices[(i + 2) * 3] - vertices[i * 3],
                    vertices[(i + 2) * 3 + 1] - vertices[i * 3 + 1],
                    vertices[(i + 2) * 3 + 2] - vertices[i * 3 + 2]
                    );

            QVector3D c = QVector3D::crossProduct(a, b);
            for (int k = 0; k < 4; k++) {
                normals[j] = c.x();
                normals[j + 1] = c.y();
                normals[j + 2] = c.z();
                j += 3;
            }
        }
        //cap normals
        for (i = 0; i < m_SideCount * 2; i++) {
            normals[j] = 0.0f;
            normals[j + 1] = 0.0f;
            if (i < m_SideCount)
                normals[j + 2] = -1.0f;
            else
                normals[j + 2] = 1.0f;
            j += 3;
        }

        //normals for center vertices
        normals[vertexCount - 4] = -1.0f;
        normals[vertexCount - 1] = 1.0f;

        arrayBuf.bind();
        arrayBuf.allocate(vertices, vertexCount * sizeof(float));

        indexBuf.bind();
        indexBuf.allocate(indices, indexCount * sizeof(GLushort));

        normalBuf.bind();
        normalBuf.allocate(normals, vertexCount * sizeof(float));

        delete[] vertices;
        delete[] indices;
        delete[] normals;
    }

private:
    int m_SideCount;
};
#endif // PRISM_H
