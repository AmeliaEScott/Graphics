#include "Application.h"
#include <stdio.h>

#define NUM_VERTICES 20
#define MIX 0.1f
#define ROTATION_SPEED 0.05f

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

//https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
glm::vec3 hsv2rgb(glm::vec3 in)
{
    float       hh, p, q, t, ff;
    long        i;
    glm::vec3   out;

    if(in.y <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.z;
        out.g = in.z;
        out.b = in.z;
        return out;
    }
    hh = in.x;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.z * (1.0f - in.y);
    q = in.z * (1.0f - (in.y * ff));
    t = in.z * (1.0f - (in.y * (1.0f - ff)));

    switch(i) {
        case 0:
            out.r = in.z;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = in.z;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = in.z;
            out.b = t;
            break;

        case 3:
            out.r = p;
            out.g = q;
            out.b = in.z;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = in.z;
            break;
        case 5:
        default:
            out.r = in.z;
            out.g = p;
            out.b = q;
            break;
    }
    return out;
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
        attribute vec3 color;
        varying vec3 rainbow;

        uniform float mix;
        uniform float rotation;

        void main()
        {
            vec2 point = mix * pos_a + (1.0 - mix) * pos_b;
            point = mat2(cos(rotation), -sin(rotation), sin(rotation), cos(rotation)) * point;
            gl_Position = vec4(point, 0.0, 1.0);
            rainbow = color;
        }
    )";

    const char* fragment_shader_src = R"(
        varying vec3 rainbow;

        void main()
        {
            gl_FragColor = vec4(rainbow, 1.0);
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
    m_attrib_color = glGetAttribLocation(m_program, "color");
    m_uniform_mix = glGetUniformLocation(m_program, "mix");
    m_uniform_rotation = glGetUniformLocation(m_program, "rotation");

    glGenBuffers(1, &m_vertexBufferObject);
    glGenBuffers(1, &m_indexBufferObject);

    /*
     * Added the following code to initialize positions
     */
    vertex vertices[NUM_VERTICES + 2];
    // First vertex is the center of the circle, at (0, 0)
    vertices[0] = {glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), glm::vec3(0.0, 0.0, 0.0)};
    // Iterate until i = NUM_VERTICES because the last vertex should overlap
    // the first one on the outside of the circle
    for(int i = 0; i <= NUM_VERTICES; i++){
        // Uniformly spread theta around the circle
        float theta = ((float) i / NUM_VERTICES) * 2 * 3.14159f;
        float mix = (i % 2) == 0 ? 1.0f : MIX;
        glm::vec3 rgb = hsv2rgb(glm::vec3(theta * 180.0f / 3.14159f, 1.0f, 1.0f));
        vertices[i + 1] = {
                glm::vec2(std::cos(theta), std::sin(theta)),
                glm::vec2(mix * std::cos(theta), mix * std::sin(theta)),
                rgb
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
    glEnableVertexAttribArray(m_attrib_color);
    glVertexAttribPointer(m_attrib_color, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), ToVoidPointer(offsetof(vertex, color)));

    // Uniform for determining what part of the cycle we are in
    glUniform1f(m_uniform_mix, (std::sin(time) + 1) / 2.0f);
    glUniform1f(m_uniform_rotation, ROTATION_SPEED * time);

    glDrawElements(GL_TRIANGLE_FAN, NUM_VERTICES + 2, GL_UNSIGNED_SHORT, ToVoidPointer(0));

    glDisableVertexAttribArray(m_attrib_pos_a);
    glDisableVertexAttribArray(m_attrib_pos_b);
    glDisableVertexAttribArray(m_attrib_color);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
