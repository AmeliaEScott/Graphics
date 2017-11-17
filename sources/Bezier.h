#ifndef OPENGLINTRO_BEZIER_H
#define OPENGLINTRO_BEZIER_H

#include <string>
#include <functional>
#include <glm/vec3.hpp>
#include "Object.h"

class Bezier : public Object
{
public:
    void Load(int height, int width, std::function<glm::vec3 (int, int)> func);
//private:
//    unsigned int m_vertexBufferObject;
//    unsigned int m_indexBufferObject;
//    unsigned int m_indexSize;
};


#endif //OPENGLINTRO_BEZIER_H
