#pragma once
#include <string>
#include <glm/vec4.hpp>
#include <glm/detail/type_mat.hpp>
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

    glm::mat4 viewMatrixOrbit(float radius, float yaw, float altitude);

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

    int m_left_pressed;
    int m_right_pressed;
    int m_up_pressed;
    int m_down_pressed;
    int m_period_pressed;
    int m_comma_pressed;

    int m_orbit_mode;

    glm::vec3 m_prev_mouse_pos;
    glm::vec3 m_current_mouse_pos;

    float m_light_1_radius;
    float m_light_1_height;
    float m_light_1_angle;

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

    unsigned int m_uniform_camera_pos;

    unsigned int m_uniform_material_ambient;
    unsigned int m_uniform_material_diffuse;
    unsigned int m_uniform_material_specular;
    unsigned int m_uniform_material_shininess;

    unsigned int m_uniform_light_1_pos;
    unsigned int m_uniform_light_1_ambient;
    unsigned int m_uniform_light_1_diffuse;
    unsigned int m_uniform_light_1_specular;

    unsigned int m_uniform_light_2_pos;
    unsigned int m_uniform_light_2_ambient;
    unsigned int m_uniform_light_2_diffuse;
    unsigned int m_uniform_light_2_specular;
};
