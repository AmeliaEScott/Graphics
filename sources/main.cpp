#include "Application.h"

#include <GLFW/glfw3.h>
#include <memory>
#include <chrono>
#include <thread>
#include "imgui.h"
#include "examples/opengl3_example/imgui_impl_glfw_gl3.h"


Application* app;

/*
 * NOTE: Use Shift key to switch between free camera and orbit mode.
 * See Application.cpp for more information.
 *
 * The framerate can be adjusted (to make sure the speed is consistent) by editing the sleep command on line
 * 75 of this file.
 */

int main()
{
    GLFWwindow* window;

    /* Initialize the library */
    glfwInit();

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RED_BITS, 8);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    ImGui_ImplGlfwGL3_Init(window, false);

    {
        //std::shared_ptr<Application> app = std::make_shared<Application>();
        app = new Application();

        glfwSetKeyCallback(window, [](GLFWwindow* thewindow, int keycode, int scancode, int event, int modifiers){
            app->KeyEvent(keycode, event);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* thewindow, int buttoncode, int event, int modifiers){
            app->MouseButtonEvent(buttoncode, event);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* thewindow, double x, double y){
            app->MouseMoveEvent(x, y);
        });

        glfwSetScrollCallback(window, [](GLFWwindow* thewindow, double x, double y){
            app->MouseScrollEvent(x, y);
        });

        auto prev = std::chrono::steady_clock::now();
        auto start = prev;

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            auto current_timestamp = std::chrono::steady_clock::now();

            ImGui_ImplGlfwGL3_NewFrame();

            std::chrono::duration<float> time = (current_timestamp - start);
            std::chrono::duration<float> delta_time = (current_timestamp - prev);
            prev = current_timestamp;

            app->Draw(time.count(), delta_time.count());
            //ImGui::SetNextWindowPos(ImVec2(0.0, 0.0), ImGuiCond_FirstUseEver);
            ImGui::ShowTestWindow();
            ImGui::Render();

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();

            std::this_thread::sleep_for(std::chrono::milliseconds(15));

        }
        ImGui_ImplGlfwGL3_Shutdown();
    }

    glfwTerminate();
    return 0;
}
