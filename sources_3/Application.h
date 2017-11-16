#pragma once
#include <string>
#include <glm/vec4.hpp>
#include "Object.h"


class Application
{
public:
    Application();
    ~Application();

    void Draw(float time, float deltatime);

    void KeyEvent(int keycode, int event);
    void MouseButtonEvent(int buttoncode, int event);
    void MouseMoveEvent(double x, double y);
    void MouseScrollEvent(double x, double y);

private:
    void DrawMesh(Object& object);

    Object m_cow;
    Object m_teapot;
    Object m_teddy;

    int m_w_pressed;
    int m_a_pressed;
    int m_s_pressed;
    int m_d_pressed;
    int m_q_pressed;
    int m_e_pressed;
    int m_mouse_pressed;
    float m_delta_scroll;

    int m_orbit_mode;

    glm::vec3 m_prev_mouse_pos;
    glm::vec3 m_current_mouse_pos;

    glm::vec3 m_camera_pos;
    float m_camera_yaw;
    float m_camera_pitch;
    float m_camera_altitude;
    float m_camera_radius;

    unsigned int m_program;
    unsigned int m_attrib_pos;
    unsigned int m_attrib_normal;
    unsigned int m_uniform_transform;
    unsigned int m_uniform_viewProjection;
    unsigned int m_uniform_color;
    unsigned int m_uniform_ambient;
    unsigned int m_uniform_light_pos;
};
