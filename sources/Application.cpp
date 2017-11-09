#include "Application.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

/*
 * CS 470: Computer Graphics
 * Assignment 3
 * Timothy Scott
 *
 * This code is based on my code for Assignment 2. I have added the camera movement code in Application::Draw,
 * as well as in the two functions viewMatrix() and viewMatrixOrbit(). They are commented below.
 *
 * In free camera mode, WASD move around in the X and Z directions. To move in the Y direction (Up and down),
 * use the Q and E keys.
 *
 * In orbit mode, dragging the mouse changes the altitude above the XZ plane and the angle about the center,
 * and scrolling changes the radius.
 *
 * To switch between orbit and free mode, hit Shift.
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

    m_a_pressed = false;
    m_w_pressed = false;
    m_s_pressed = false;
    m_d_pressed = false;
    m_q_pressed = false;
    m_e_pressed = false;
    m_mouse_pressed = false;

    m_camera_pos = glm::vec3(0.0f, 0.0f, 40.0f);
    m_camera_yaw = 0.0f;
    m_camera_pitch = 0.0f;
    m_camera_altitude = 0.0f;
    m_camera_radius = 90.0f;
    m_delta_scroll = 0.0f;

    m_prev_mouse_pos = glm::vec3(0.0, 0.0, 0.0);
    m_current_mouse_pos = glm::vec3(0.0, 0.0, 0.0);

    m_orbit_mode = 1;
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

/**
 * Returns the view matrix for free camera mode.
 * @param eye Location of the camera
 * @param pitch Vertical angle of the camera, in radians
 * @param yaw Horizontal angle of the camera, in radians
 * @return The view matrix as a glm::mat4
 */
glm::mat4 viewMatrix( glm::vec3 eye, float pitch, float yaw )
{

    float translate[16] = {
            1.0f, 0.0f, 0.0f, eye[0],
            0.0f, 1.0f, 0.0f, eye[1],
            0.0f, 0.0f, 1.0f, eye[2],
            0.0f, 0.0f, 0.0f, 1.0f
    };

    float roty[16] = {
            cosf(-yaw), 0.0f, -sinf(-yaw), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            sinf(-yaw), 0.0f, cosf(-yaw), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };

    float rotx[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cosf(-pitch), sinf(-pitch), 0.0f,
            0.0f, -sinf(-pitch), cosf(-pitch), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f

    };

    // The matrices are transposed because GLM stores them in column-major form, but the float arrays above are in
    // row-major form.
    glm::mat4 t = glm::transpose(glm::make_mat4(translate));
    glm::mat4 ry = glm::transpose(glm::make_mat4(roty));
    glm::mat4 rx = glm::transpose(glm::make_mat4(rotx));

    return glm::inverse(t * (ry * rx));

}

/**
 * Returns the view matrix for orbit mode.
 * @param radius Distance from (0, 0)
 * @param yaw Rotation about (0, 0)
 * @param altitude Angle above the XZ Plane
 * @return View matrix as a glm::mat4
 */
glm::mat4 viewMatrixOrbit(float radius, float yaw, float altitude){
    float xzRadius = radius * cosf(altitude);
    glm::vec3 eye = glm::vec3(xzRadius * sinf(yaw), radius * sinf(altitude), xzRadius * cosf(yaw));
    return viewMatrix(eye, -altitude, yaw);
}

void Application::KeyEvent(int keycode, int event){
    if(keycode == 'Q'){
        m_q_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == 'W'){
        m_w_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == 'E'){
        m_e_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == 'A'){
        m_a_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == 'S'){
        m_s_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == 'D'){
        m_d_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if((keycode == 340 || keycode == 344) && event == GLFW_PRESS){  // Left shift and right shift keys
        m_orbit_mode = !m_orbit_mode;
    }
}

void Application::MouseButtonEvent(int buttoncode, int event) {
    if(buttoncode == GLFW_MOUSE_BUTTON_1){
        m_mouse_pressed = (event == GLFW_PRESS);
    }
}

void Application::MouseMoveEvent(double x, double y) {
    m_current_mouse_pos = glm::vec3(x, y, 0.0);
}

void Application::MouseScrollEvent(double x, double y) {
    m_delta_scroll += y;
}

void Application::Draw(float time, float deltatime)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glUseProgram(m_program);



    /* #################################
     * #### BEGIN ASSIGNMENT 3 CODE ####
     * #################################
     */

    glm::mat4 projection = glm::perspectiveRH(1.0f, 1.0f, 0.1f, 10000.0f);


    glm::vec3 mousedelta = m_current_mouse_pos - m_prev_mouse_pos;
    m_prev_mouse_pos = m_current_mouse_pos;

    float deltaScroll = m_delta_scroll;
    m_delta_scroll = 0.0f;

    glm::mat4 view;
    if(!m_orbit_mode) {
        float speed = 20.0f;
        float mousespeed = 0.07f;
        glm::vec4 delta = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        if (m_w_pressed) {
            delta.z -= speed * deltatime;
        }
        if (m_s_pressed) {
            delta.z += speed * deltatime;
        }
        if (m_a_pressed) {
            delta.x -= speed * deltatime;
        }
        if (m_d_pressed) {
            delta.x += speed * deltatime;
        }
        if (m_q_pressed) {
            delta.y -= speed * deltatime;
        }
        if (m_e_pressed) {
            delta.y += speed * deltatime;
        }
        // This rotates the delta vector so that it aligns with the camera.
        delta = glm::rotate(glm::mat4(1.0f), m_camera_yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * delta;

        m_camera_pos += glm::vec3(delta.x, delta.y, delta.z);

        float d_yaw = 0.0f;
        float d_pitch = 0.0f;
        if (m_mouse_pressed) {
            d_yaw -= mousedelta.x * mousespeed * deltatime;
            d_pitch -= mousedelta.y * mousespeed * deltatime;
        }
        m_camera_pitch += d_pitch;
        m_camera_yaw += d_yaw;

        if (m_camera_pitch < -3.14159f / 2.0f) {
            m_camera_pitch = -3.14159f / 2.0f;
        } else if (m_camera_pitch > 3.14159f / 2.0f) {
            m_camera_pitch = 3.14159f / 2.0f;
        }

        view = viewMatrix(m_camera_pos, m_camera_pitch, m_camera_yaw);
    }else{
        float yawspeed = 0.4f;
        float altitudespeed = 0.3f;
        float radiusspeed = 10.0f;

        float d_yaw = 0.0f;
        float d_altitude = 0.0f;
        if(m_mouse_pressed){
            d_yaw -= mousedelta.x * yawspeed * deltatime;
            d_altitude += mousedelta.y * altitudespeed * deltatime;
        }

        m_camera_yaw += d_yaw;
        m_camera_altitude += d_altitude;
        if(m_camera_altitude > 3.14159f / 2.0f){
            m_camera_altitude = 3.14159f / 2.0f;
        }else if(m_camera_altitude < -3.14159f / 2.0f){
            m_camera_altitude = -3.14159f / 2.0f;
        }

        float d_radius = deltaScroll * radiusspeed * deltatime;
        m_camera_radius -= d_radius;
        if(m_camera_radius < 0.0f){
            m_camera_radius = 0.0f;
        }

        view = viewMatrixOrbit(m_camera_radius, m_camera_yaw, m_camera_altitude);
    }
    glm::mat4 viewProjection = projection * view;

    glUniformMatrix4fv(m_uniform_viewProjection, 1, GL_FALSE, &viewProjection[0][0]);

    /* #################################
     * ##### END ASSIGNMENT 3 CODE #####
     * #################################
     *
     * The rest of this code is just for rendering the scene. It is the same as it was in Assignment 3,
     * but with the movement removed (so it is easier to tell how the camera is moving).
     */

    glm::mat4 transform = glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 cowscale = glm::scale(transform, glm::vec3(3.0, 3.0, 3.0));
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &cowscale[0][0]);

    glUniform3f(m_uniform_color, 1.0, 1.0, 0.0);
    glUniform1f(m_uniform_ambient, 1.0);

    DrawMesh(m_cow);

    glUniform3f(m_uniform_color, 0.0, 1.0, 0.0);
    glUniform1f(m_uniform_ambient, 0.2);

    transform = glm::translate(transform, glm::vec3(0.0, 0.0, -40.0));
    transform = glm::rotate(transform, 0.0f, glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &transform[0][0]);

    DrawMesh(m_teapot);

    glUniform3f(m_uniform_color, 0.8, 0.5, 0.2);

    transform = glm::rotate(transform, 0.0f, glm::vec3(0.0, 1.0, 0.0));
    transform = glm::translate(transform, glm::vec3(10.0, 0.0, 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
    transform = glm::rotate(transform, -3.14159f / 2.0f, glm::vec3(0.0, 1.0, 0.0));
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
