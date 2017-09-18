#include "Application.h"
#include <stdio.h>

#define NUM_VERTICES 10

/*
 * CS 470: Computer Graphics
 * Assignment 1
 * Timothy Scott
 *
 * My additions have been commented below
 */

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

Application::Application()
{
    gl3wInit();

    const char* OpenGLversion = (const char*)glGetString(GL_VERSION);
    const char* GLSLversion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("OpenGL %s GLSL: %s\n", OpenGLversion, GLSLversion);


    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    /*
     * Added second position attribute, mix uniform, and mixing between positions
     */
    const char* vertex_shader_src = R"(
        attribute vec2 pos_a;
        attribute vec2 pos_b;

        uniform float mix;

        void main()
        {
            gl_Position = vec4(mix * pos_a + (1.0 - mix) * pos_b, 0.0, 1.0);
        }
    )";

    const char* fragment_shader_src = R"(
        void main()
        {
            gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
        }
    )";

    GLuint vertex_shader_handle = CompileShader(vertex_shader_src, GL_VERTEX_SHADER);
    GLuint fragment_shader_handle = CompileShader(fragment_shader_src, GL_FRAGMENT_SHADER);

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

    m_attrib_pos_a = glGetAttribLocation(m_program, "pos_a");
    m_attrib_pos_b = glGetAttribLocation(m_program, "pos_b");
    m_uniform_mix = glGetUniformLocation(m_program, "mix");

    glGenBuffers(1, &m_vertexBufferObject);
    glGenBuffers(1, &m_indexBufferObject);

    /*
     * Added the following code to initialize positions
     */
    vertex vertices[NUM_VERTICES + 2];
    // First vertex is the center of the circle, at (0, 0)
    vertices[0] = {glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0)};
    // Iterate until i = NUM_VERTICES because the last vertex should overlap
    // the first one on the outside of the circle
    for(int i = 0; i <= NUM_VERTICES; i++){
        // Uniformly spread theta around the circle
        float theta = ((float) i / NUM_VERTICES) * 2 * 3.14159f;
        float mix = (i % 2) == 0 ? 1.0f : 0.25f;
        vertices[i + 1] = {
                glm::vec2(std::cos(theta), std::sin(theta)),
                glm::vec2(mix * std::cos(theta), mix * std::sin(theta))
        };
    }

    short indices[NUM_VERTICES + 2] = {0};
    for(short i = 0; i < NUM_VERTICES + 2; i++){
        indices[i] = i;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, (NUM_VERTICES + 2) * sizeof(vertex), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (NUM_VERTICES + 2) * sizeof(short), indices, GL_STATIC_DRAW);
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
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_program);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);

    glEnableVertexAttribArray(m_attrib_pos_a);
    glVertexAttribPointer(m_attrib_pos_a, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), ToVoidPointer(offsetof(vertex, pos_a)));
    // Added position b attribute array
    glEnableVertexAttribArray(m_attrib_pos_b);
    glVertexAttribPointer(m_attrib_pos_b, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), ToVoidPointer(offsetof(vertex, pos_b)));

    // Uniform for determining what part of the cycle we are in
    glUniform1f(m_uniform_mix, (std::sin(time) + 1) / 2.0f);

    glDrawElements(GL_TRIANGLE_FAN, NUM_VERTICES + 2, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(m_attrib_pos_a);
    glDisableVertexAttribArray(m_attrib_pos_b);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
