#pragma once

#include <GL/gl3w.h>
#include <glm/glm.hpp>

class Application
{
public:
    Application();
    ~Application();

    void Draw(float time);

private:
    GLuint m_program;
    GLuint m_attrib_pos_a;
    GLuint m_attrib_pos_b;
    GLuint m_uniform_mix;

    unsigned int m_vertexBufferObject;
    unsigned int m_indexBufferObject;

    struct vertex {
        glm::vec2 pos_a;
        glm::vec2 pos_b;
    } __attribute__((packed));
};
