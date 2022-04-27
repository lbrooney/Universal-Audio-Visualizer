#ifndef SHAPE_H
#define SHAPE_H

#include <QOpenGLFunctions>
#include <lib/glm/glm/glm.hpp>
#include <lib/glm/glm/gtc/matrix_transform.hpp>
#include <lib/glm/glm/gtc/type_ptr.hpp>
#include <vector>
#include <QVector3D>

class Shape
{
public:
    Shape()
    {
        transformMatrix = glm::mat4(1.0f);
    }

    virtual void CreateIndexArray() = 0;
    virtual void CreateVertexArray() = 0;

    void SetTranslation(glm::vec3 translation)
    {
        transformMatrix = glm::translate(transformMatrix, translation);
    }
    void SetRotation(float rotDegree, glm::vec3 axis)
    {
        transformMatrix = glm::rotate(transformMatrix, glm::radians(rotDegree), axis);
    }
    void SetScale(glm::vec3 scale)
    {
        transformMatrix = glm::scale(transformMatrix, scale);
    }
    std::vector<unsigned int> indices;
    std::vector<QVector3D> vertices;
    glm::mat4 transformMatrix;
};

#endif // SHAPE_H
