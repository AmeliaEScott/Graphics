#include "Application.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

GLuint CompileShader(const char* src, GLint type)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &src, NULL);

    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

    if (infoLen > 1)
    {
        printf("%s during shader compilation.\n ", compiled == GL_TRUE ? "Warning" : "Error");
        char* buf = new char[infoLen];
        glGetShaderInfoLog(shader, infoLen, NULL, buf);
        printf("Compilation log: %s\n", buf);
        delete[] buf;
    }

    return shader;
}

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
};

Application::Application()
{
    gl3wInit();

    const char* OpenGLversion = (const char*)glGetString(GL_VERSION);
    const char* GLSLversion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("OpenGL %s GLSL: %s", OpenGLversion, GLSLversion);


    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearDepth(1.0f);

    const char* vertex_shader_src = R"(
        attribute vec3 a_position;
        attribute vec3 a_normal;

        uniform mat4 u_transform;
        uniform mat4 u_viewProjection;

        varying vec3 v_normal;
        varying vec4 v_pos;

        void main()
        {
            v_normal = (u_transform * vec4(a_normal, 0.0)).xyz;
            v_pos = u_transform * vec4(a_position, 1.0);
            gl_Position = u_viewProjection * v_pos;
        }
    )";

    const char* fragment_shader_src = R"(
        uniform vec3 u_light_pos;
        uniform vec3 u_color;
        uniform float u_ambient;

        varying vec3 v_normal;
        varying vec4 v_pos;

        void main()
        {
            float diffuse = dot(normalize(v_normal), normalize(-v_pos.xyz));
            diffuse = max(diffuse, 0.0);
            gl_FragColor = vec4(u_color * (diffuse + u_ambient), 1.0);
        }
    )";

    int vertex_shader_handle = CompileShader(vertex_shader_src, GL_VERTEX_SHADER);
    int fragment_shader_handle = CompileShader(fragment_shader_src, GL_FRAGMENT_SHADER);

    m_program = glCreateProgram();

    glAttachShader(m_program, vertex_shader_handle);
    glAttachShader(m_program, fragment_shader_handle);

    glLinkProgram(m_program);

    int linked;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            char* buf = new char[infoLen];
            glGetProgramInfoLog(m_program, infoLen, NULL, buf);
            printf("Linking error: \n%s\n", buf);
            delete[] buf;
        }
    }

    glDetachShader(m_program, vertex_shader_handle);
    glDetachShader(m_program, fragment_shader_handle);

    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);

    m_attrib_pos = glGetAttribLocation(m_program, "a_position");
    m_attrib_normal = glGetAttribLocation(m_program, "a_normal");

    m_uniform_transform = glGetUniformLocation(m_program, "u_transform");
    m_uniform_viewProjection = glGetUniformLocation(m_program, "u_viewProjection");

    m_uniform_color = glGetUniformLocation(m_program, "u_color");
    m_uniform_ambient = glGetUniformLocation(m_program, "u_ambient");
    m_uniform_light_pos = glGetUniformLocation(m_program, "u_light_pos");

    m_cow.Load("../cow.obj");
    m_teapot.Load("../teapot.obj");
    m_teddy.Load("../teddy.obj");
}

void Object::Load(std::string path)
{
    glGenBuffers(1, &m_vertexBufferObject);
    glGenBuffers(1, &m_indexBufferObject);

    std::vector<Vertex> vertices;
    std::vector<int> indices;
    std::ifstream file(path);
    std::string str;
    Vertex v;
    v.pos = glm::vec3();
    while (std::getline(file, str))
    {
        if (strncmp(str.c_str(), "v ", 2) == 0)
        {
            sscanf(str.c_str(), "v %f %f %f", &v.pos.x, &v.pos.y, &v.pos.z);
            v.normal = glm::vec3(0);
            vertices.push_back(v);
        }
        else if (strncmp(str.c_str(), "f ", 2) == 0)
        {
            int tmp;
            int a, b, c, d;
            sscanf(str.c_str(), "f %d %d %d", &a, &b, &c);
            indices.push_back(a - 1);
            indices.push_back(b - 1);
            indices.push_back(c - 1);
        }
    }
    m_indexSize = indices.size();

    for (int f = 0; f < m_indexSize / 3; ++f)
    {
        int i = indices[3 * f + 0];
        int j = indices[3 * f + 1];
        int k = indices[3 * f + 2];
        glm::vec3 p1 = vertices[i].pos;
        glm::vec3 p2 = vertices[j].pos;
        glm::vec3 p3 = vertices[k].pos;
        glm::vec3 a = p1 - p3;
        glm::vec3 b = p2 - p3;
        glm::vec3 c = glm::cross(a, b);
        vertices[i].normal += c;
        vertices[j].normal += c;
        vertices[k].normal += c;
    }

    for (int i = 0; i < vertices.size(); ++i)
    {
        vertices[i].normal = glm::normalize(vertices[i].normal);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Object::Draw()
{
    glDrawElements(GL_TRIANGLES, m_indexSize, GL_UNSIGNED_INT, 0);
}

void Object::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
}

void Object::UnBind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Application::~Application()
{
    glDeleteProgram(m_program);
}

inline void* ToVoidPointer(int offset)
{
    size_t offset_ = static_cast<size_t>(offset);
    return reinterpret_cast<void*>(offset_);
}

void Application::Draw(float time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glUseProgram(m_program);

    glm::mat4 transform = glm::mat4(1.0);
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &transform[0][0]);

    glm::mat4 projection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, -200.0f, 200.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(1.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 viewProjection = projection * view;

    glUniformMatrix4fv(m_uniform_viewProjection, 1, GL_FALSE, &viewProjection[0][0]);

    glUniform3f(m_uniform_color, 1.0, 1.0, 0.0);
    glUniform1f(m_uniform_ambient, 1.0);

    DrawMesh(m_cow);

    glUniform3f(m_uniform_color, 0.0, 1.0, 0.0);
    glUniform1f(m_uniform_ambient, 0.2);

    transform = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -40.0));
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &transform[0][0]);

    DrawMesh(m_teapot);

    glUniform3f(m_uniform_color, 0.8, 0.5, 0.2);

    transform = glm::translate(glm::mat4(1.0), glm::vec3(40.0, 0.0, 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &transform[0][0]);

    DrawMesh(m_teddy);
}

void Application::DrawMesh(Object& object)
{
    object.Bind();

    glEnableVertexAttribArray(m_attrib_pos);
    glVertexAttribPointer(m_attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(0));

    glEnableVertexAttribArray(m_attrib_normal);
    glVertexAttribPointer(m_attrib_normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(sizeof(glm::vec3)));

    object.Draw();

    glDisableVertexAttribArray(m_attrib_pos);
    glDisableVertexAttribArray(m_attrib_normal);

    object.UnBind();
}
