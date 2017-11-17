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
        attribute vec3 a_uv;

        uniform mat4 u_transform;
        uniform mat4 u_viewProjection;

        varying vec4 v_normal;
        varying vec4 v_pos;
        varying vec3 v_uv;

        void main()
        {
            v_normal = vec4(normalize((u_transform * vec4(a_normal, 0.0)).xyz), 1.0);
            v_pos = u_transform * vec4(a_position, 1.0);
            v_uv = a_uv;
            gl_Position = u_viewProjection * v_pos;
        }
    )";

    const char* fragment_shader_src = R"(
        uniform vec4 u_camera_pos;

        uniform vec4 u_material_ambient;
        uniform vec4 u_material_diffuse;
        uniform vec4 u_material_specular;
        uniform float u_material_shininess;

        uniform vec4 u_light_1_pos;
        uniform vec4 u_light_1_ambient;
        uniform vec4 u_light_1_diffuse;
        uniform vec4 u_light_1_specular;

        uniform vec4 u_light_2_pos;
        uniform vec4 u_light_2_ambient;
        uniform vec4 u_light_2_diffuse;
        uniform vec4 u_light_2_specular;

        varying vec4 v_normal;
        varying vec4 v_pos;
        varying vec3 v_uv;

        void main()
        {
            vec3 V = normalize(u_camera_pos.xyz - v_pos.xyz);
            vec3 L1 = normalize(u_light_1_pos.xyz - v_pos.xyz);
            vec3 L2 = normalize(u_light_2_pos.xyz - v_pos.xyz);
            vec3 R1 = (2.0 * dot(L1, v_normal.xyz) * v_normal.xyz) - L1;
            vec3 R2 = (2.0 * dot(L2, v_normal.xyz) * v_normal.xyz) - L2;

            vec3 ambient = u_material_ambient.xyz * (u_light_1_ambient.xyz + u_light_2_ambient.xyz);
            vec3 light1diffuse = u_material_diffuse.xyz * max(dot(L1, v_normal.xyz), 0.0) * u_light_1_diffuse.xyz;
            vec3 light1specular = u_light_1_specular.xyz * u_material_specular.xyz * pow(max(dot(R1, V), 0.0), u_material_shininess);
            vec3 light2diffuse = u_material_diffuse.xyz * max(dot(L2, v_normal.xyz), 0.0) * u_light_2_diffuse.xyz;
            vec3 light2specular = u_light_2_specular.xyz * u_material_specular.xyz * pow(max(dot(R2, V), 0.0), u_material_shininess);
            gl_FragColor = vec4(ambient + light1diffuse + light1specular + light2diffuse + light2specular, 1.0);
            //gl_FragColor = v_normal;
            //gl_FragColor = vec4(v_uv.x / 20.0, v_uv.y / 20.0, 0.0, 1.0);
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
    m_attrib_uv = glGetAttribLocation(m_program, "a_uv");

    m_uniform_transform = glGetUniformLocation(m_program, "u_transform");
    m_uniform_viewProjection = glGetUniformLocation(m_program, "u_viewProjection");

    m_uniform_material_ambient = glGetUniformLocation(m_program, "u_material_ambient");
    m_uniform_material_diffuse = glGetUniformLocation(m_program, "u_material_diffuse");
    m_uniform_material_specular = glGetUniformLocation(m_program, "u_material_specular");
    m_uniform_material_shininess = glGetUniformLocation(m_program, "u_material_shininess");

    m_uniform_light_1_pos = glGetUniformLocation(m_program, "u_light_1_pos");
    m_uniform_light_1_ambient  = glGetUniformLocation(m_program, "u_light_1_ambient");
    m_uniform_light_1_diffuse = glGetUniformLocation(m_program, "u_light_1_diffuse");
    m_uniform_light_1_specular = glGetUniformLocation(m_program, "u_light_1_specular");

    m_uniform_light_2_pos = glGetUniformLocation(m_program, "u_light_2_pos");
    m_uniform_light_2_ambient = glGetUniformLocation(m_program, "u_light_2_ambient");
    m_uniform_light_2_diffuse = glGetUniformLocation(m_program, "u_light_2_diffuse");
    m_uniform_light_2_specular = glGetUniformLocation(m_program, "u_light_2_specular");

    m_uniform_camera_pos = glGetUniformLocation(m_program, "u_camera_pos");

    m_cow.Load("../cow.obj");
    m_teapot.Load("../sphere.obj");
    m_teddy.Load("../teddy.obj");
    m_bezier.Load(20, 20, [](int x, int y) -> glm::vec3{
        return glm::vec3(x, 0.05 * ((x - 10) * (x - 10) - (y - 10) * (y - 10)), y);
    });

    m_a_pressed = false;
    m_w_pressed = false;
    m_s_pressed = false;
    m_d_pressed = false;
    m_q_pressed = false;
    m_e_pressed = false;
    m_mouse_pressed = false;

    m_left_pressed = false;
    m_right_pressed = false;
    m_up_pressed = false;
    m_down_pressed = false;
    m_period_pressed = false;
    m_comma_pressed = false;

    m_camera_pos = glm::vec3(0.0f, 0.0f, 40.0f);
    m_camera_yaw = 0.0f;
    m_camera_pitch = 0.0f;
    m_camera_altitude = 0.0f;
    m_camera_radius = 90.0f;
    m_delta_scroll = 0.0f;

    m_prev_mouse_pos = glm::vec3(0.0, 0.0, 0.0);
    m_current_mouse_pos = glm::vec3(0.0, 0.0, 0.0);

    m_orbit_mode = 1;

    m_light_1_radius = 0.0f;
    m_light_1_angle = 0.0f;
    m_light_1_height = 60.0f;

    m_light_1_on = 1;
    m_light_2_on = 1;

    m_current_material = 0;
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
glm::mat4 Application::viewMatrixOrbit(float radius, float yaw, float altitude){
    float xzRadius = radius * cosf(altitude);
    glm::vec3 eye = glm::vec3(xzRadius * sinf(yaw), radius * sinf(altitude), xzRadius * cosf(yaw));
    m_camera_pos = eye;
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
    }else if(keycode == 262){  // Right arrow
        m_right_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == 263){  // Left arrow
        m_left_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == 264){  // Down arrow
        m_down_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == 265){  // Up arrow
        m_up_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == '.'){
        m_period_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == ','){
        m_comma_pressed = (event == GLFW_PRESS || event == GLFW_REPEAT);
    }else if(keycode == '1' && event == GLFW_PRESS){
        m_light_1_on = !m_light_1_on;
    }else if(keycode == '2' && event == GLFW_PRESS){
        m_light_2_on = !m_light_2_on;
    }else if(keycode == ']' && event == GLFW_PRESS){
        m_current_material = (m_current_material + 1) % MATERIALS;
    }else if(keycode == '[' && event == GLFW_PRESS){
        m_current_material = (m_current_material - 1 + MATERIALS) % MATERIALS;
    }
    else{
        printf("Key event: %d\n", keycode);
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

    float lightSpeed = 20.0f;  // NOTE: reduced from 3.0x10^8
    float lightAngleSpeed = 1.5f;

    if(m_up_pressed){
        m_light_1_height += lightSpeed * deltatime;
    }
    if(m_down_pressed){
        m_light_1_height -= lightSpeed * deltatime;
    }
    if(m_left_pressed){
        m_light_1_angle += lightAngleSpeed * deltatime;
    }
    if(m_right_pressed){
        m_light_1_angle -= lightAngleSpeed * deltatime;
    }
    if(m_comma_pressed){
        m_light_1_radius -= lightSpeed * deltatime;
    }
    if(m_period_pressed){
        m_light_1_radius += lightSpeed * deltatime;
    }

    if(m_light_1_radius < 0.0f){
        m_light_1_radius = 0.0f;
    }

    glm::vec3 light1pos = glm::vec3(m_light_1_radius * cosf(m_light_1_angle), m_light_1_height, m_light_1_radius * sinf(m_light_1_angle));

    //printf("Camera pos: %.3f, %.3f, %.3f\n", m_camera_pos.x, m_camera_pos.y, m_camera_pos.z);
    glUniform4f(m_uniform_camera_pos, m_camera_pos.x, m_camera_pos.y, m_camera_pos.z, 1.0);

    Material m = m_materials[m_current_material];

    glUniform4f(m_uniform_material_ambient, m.material_ambient[0], m.material_ambient[1], m.material_ambient[2], m.material_ambient[3]);
    glUniform4f(m_uniform_material_diffuse, m.material_diffuse[0], m.material_diffuse[1], m.material_diffuse[2], m.material_diffuse[3]);
    glUniform4f(m_uniform_material_specular, m.material_specular[0], m.material_specular[1], m.material_specular[2], m.material_specular[3]);
    glUniform1f(m_uniform_material_shininess, m.shininess);

    glUniform4f(m_uniform_light_1_pos, light1pos.x, light1pos.y, light1pos.z, 1.0);
    if(m_light_1_on) {
        glUniform4f(m_uniform_light_1_ambient, m.light_1_ambient[0], m.light_1_ambient[1], m.light_1_ambient[2], m.light_1_ambient[3]);
        glUniform4f(m_uniform_light_1_diffuse, m.light_1_diffuse[0], m.light_1_diffuse[1], m.light_1_diffuse[2], m.light_1_diffuse[3]);
        glUniform4f(m_uniform_light_1_specular, m.light_1_specular[0], m.light_1_specular[1], m.light_1_specular[2], m.light_1_specular[3]);
    }else{
        glUniform4f(m_uniform_light_1_ambient, 0.0, 0.0, 0.0, 1.0);
        glUniform4f(m_uniform_light_1_diffuse, 0.0, 0.0, 0.0, 1.0);
        glUniform4f(m_uniform_light_1_specular, 0.0, 0.0, 0.0, 1.0);
    }

    glUniform4f(m_uniform_light_2_pos, m_camera_pos.x, m_camera_pos.y, m_camera_pos.z, 1.0);
    if(m_light_2_on) {
        glUniform4f(m_uniform_light_2_ambient, m.light_2_ambient[0], m.light_2_ambient[1], m.light_2_ambient[2], m.light_2_ambient[3]);
        glUniform4f(m_uniform_light_2_diffuse, m.light_2_diffuse[0], m.light_2_diffuse[1], m.light_2_diffuse[2], m.light_2_diffuse[3]);
        glUniform4f(m_uniform_light_2_specular, m.light_2_specular[0], m.light_2_specular[1], m.light_2_specular[2], m.light_2_specular[3]);
    }else{
        glUniform4f(m_uniform_light_2_ambient, 0.0, 0.0, 0.0, 1.0);
        glUniform4f(m_uniform_light_2_diffuse, 0.0, 0.0, 0.0, 1.0);
        glUniform4f(m_uniform_light_2_specular, 0.0, 0.0, 0.0, 1.0);
    }


    //glm::mat4 transform = glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 transform = glm::mat4(1.0f);
    glm::mat4 cowscale = glm::scale(transform, glm::vec3(6.0, 6.0, 6.0));
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &cowscale[0][0]);

    DrawMesh(m_cow);


    transform = glm::translate(transform, glm::vec3(0.0, 0.0, -40.0));
    transform = glm::rotate(transform, 0.0f, glm::vec3(0.0, 1.0, 0.0));
    transform = glm::scale(transform, glm::vec3(6.0, 6.0, 6.0));
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &transform[0][0]);

    DrawMesh(m_teapot);

    transform = glm::rotate(transform, 0.0f, glm::vec3(0.0, 1.0, 0.0));
    transform = glm::translate(transform, glm::vec3(10.0, 0.0, 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
    transform = glm::rotate(transform, -3.14159f / 2.0f, glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &transform[0][0]);

    DrawMesh(m_teddy);

    transform = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 40.0, 0.0));
    glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &transform[0][0]);
    DrawMesh(m_bezier);
}

void Application::DrawMesh(Object& object)
{
    object.Bind();

    glEnableVertexAttribArray(m_attrib_pos);
    glVertexAttribPointer(m_attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(0));

    glEnableVertexAttribArray(m_attrib_normal);
    glVertexAttribPointer(m_attrib_normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(offsetof(Vertex, normal)));

    glEnableVertexAttribArray(m_attrib_uv);
    glVertexAttribPointer(m_attrib_uv, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(offsetof(Vertex, uv)));

    object.Draw();

    glDisableVertexAttribArray(m_attrib_pos);
    glDisableVertexAttribArray(m_attrib_normal);

    object.UnBind();
}
