#pragma once
#include <string>
#include <glm/vec4.hpp>
#include <glm/detail/type_mat.hpp>
#include "Object.h"
#include "Bezier.h"

#define MATERIALS 4

typedef struct Material {
    glm::vec4 material_ambient;
    glm::vec4 material_diffuse;
    glm::vec4 material_specular;
    float shininess;

    glm::vec4 light_1_ambient;
    glm::vec4 light_1_diffuse;
    glm::vec4 light_1_specular;

    glm::vec4 light_2_ambient;
    glm::vec4 light_2_diffuse;
    glm::vec4 light_2_specular;
} Material;

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
    Bezier m_bezier;

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

    int m_light_1_on;
    int m_light_2_on;

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

    Material m_materials[MATERIALS] = {
            {       // From the handout
                    glm::vec4(0.6, 0.2, 0.2, 1.0),
                    glm::vec4(0.9, 0.1, 0.1, 1.0),
                    glm::vec4(0.8, 0.8, 0.8, 1.0),
                    80.0f,
                    glm::vec4(0.2, 0.2, 0.2, 1.0),
                    glm::vec4(0.6, 0.6, 0.6, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
                    glm::vec4(0.2, 0.2, 0.2, 1.0),
                    glm::vec4(0.6, 0.6, 0.6, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
            },
            {       // Blue, non-shiny material
                    glm::vec4(0.01, 0.01, 0.7, 1.0),
                    glm::vec4(0.05, 0.05, 0.95, 1.0),
                    glm::vec4(0.05, 0.05, 0.1, 1.0),
                    2.0f,
                    glm::vec4(0.2, 0.2, 0.2, 1.0),
                    glm::vec4(0.6, 0.6, 0.6, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
                    glm::vec4(0.1, 0.1, 0.1, 1.0),
                    glm::vec4(0.4, 0.4, 0.4, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
            },
            {       // Copper
                    glm::vec4(0.55, 0.3, 0.1, 1.0),
                    glm::vec4(0.55, 0.3, 0.1, 1.0),
                    glm::vec4(0.55, 0.3, 0.1, 1.0),
                    300.0f,
                    glm::vec4(0.2, 0.2, 0.2, 1.0),
                    glm::vec4(0.6, 0.6, 0.6, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
                    glm::vec4(0.2, 0.2, 0.2, 1.0),
                    glm::vec4(0.6, 0.6, 0.6, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
            },
            {       // Crazy debug material
                    glm::vec4(1.0, 0.0, 0.0, 1.0),
                    glm::vec4(0.0, 1.0, 0.0, 1.0),
                    glm::vec4(0.0, 0.0, 1.0, 1.0),
                    50.0f,
                    glm::vec4(0.2, 0.2, 0.2, 1.0),
                    glm::vec4(0.6, 0.6, 0.6, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
                    glm::vec4(0.2, 0.2, 0.2, 1.0),
                    glm::vec4(0.6, 0.6, 0.6, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
            }
    };

    int m_current_material;

    unsigned int m_program;
    unsigned int m_attrib_pos;
    unsigned int m_attrib_normal;
    unsigned int m_attrib_uv;
    unsigned int m_uniform_transform;
    unsigned int m_uniform_viewProjection;

    unsigned int m_uniform_camera_pos;
    unsigned int m_uniform_texture;
    unsigned int m_uniform_use_texture;

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
