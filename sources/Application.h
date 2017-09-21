#pragma once
#include <string>

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

class Application
{
public:
    Application();
    ~Application();

    void Draw(float time);

private:
    void DrawMesh(Object& object);

    Object m_cow;
    Object m_teapot;
    Object m_teddy;

    unsigned int m_program;
    unsigned int m_attrib_pos;
    unsigned int m_attrib_normal;
    unsigned int m_uniform_transform;
    unsigned int m_uniform_viewProjection;
    unsigned int m_uniform_color;
    unsigned int m_uniform_ambient;
    unsigned int m_uniform_light_pos;
};
