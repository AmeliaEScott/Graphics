#ifndef OPENGLINTRO_OBJECT_H
#define OPENGLINTRO_OBJECT_H

#include <string>
#include <glm/vec3.hpp>

class Object
{
public:
    void Load(std::string path);

    void Bind();
    void Draw();
    void UnBind();
private:
    unsigned int m_vertexBufferObject;
    unsigned int m_indexBufferObject;
    unsigned int m_indexSize;
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
};


#endif //OPENGLINTRO_OBJECT_H
