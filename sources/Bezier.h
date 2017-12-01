#ifndef OPENGLINTRO_BEZIER_H
#define OPENGLINTRO_BEZIER_H

#define TEXTURE_WIDTH (1 << 13)
#define TEXTURE_HEIGHT (1 << 13)
#define SQUARE_SIZE (1 << 8)

#include <string>
#include <functional>
#include <glm/vec3.hpp>
#include <GL/gl3w.h>
#include "Object.h"

class Bezier : public Object
{
public:
    void Load(float startu, float endu, int usteps, float startv, float endv, int vsteps, std::function<glm::vec3 (float, float)> func);

    void Draw();
    void Bind();
    void UnBind();

    GLuint textureHandle;
private:


    GLubyte texture[TEXTURE_HEIGHT][TEXTURE_WIDTH][3];
};


#endif //OPENGLINTRO_BEZIER_H
