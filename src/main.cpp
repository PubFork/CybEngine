#include "stdafx.h"
#include <Windows.h>
#include <GLFW/glfw3.h>

#include "core/Log.h"
#include "core/Timer.h"
#include "renderer/RenderDevice.h"
#include "engine/Model.h"

GLFWwindow *OpenWindow(uint32_t width, uint32_t height, const char *title)
{
    static bool isInitialized = false;

    if (!isInitialized)
    {
        glfwSetErrorCallback([](int, const char *msg) { throw core::FatalException(msg); });
        glfwInit();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        glfwWindowHint(GLFW_SAMPLES, 64);
        isInitialized = true;
    }

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    return window;
}

int main()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    int returnValue = 0;

    try
    {
        GLFWwindow *window = OpenWindow(1024, 760, "Cyb engine test");
        auto device = renderer::CreateRenderDeviceGL();

        engine::Model model;
        auto litShader = device->CreateShaderSet({ device->LoadBuiltinShader(renderer::Shader_Vertex, renderer::VShader_MVP), device->LoadBuiltinShader(renderer::Shader_Fragment, renderer::FShader_LitGouraud) });
        model.LoadOBJ(device, litShader, "assets/Porsche_911_GT2.obj");

        device->SetProjection(glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f));
        glm::mat4 view = glm::lookAt(
            glm::vec3(0, 0, -3),    // position
            glm::vec3(0, 0, 0),     // target
            glm::vec3(0, 1, 0));    // up

        char titleBuffer[64];
        double startTime = core::Timer::GetSeconds();
        double previousFrametime = startTime;

        //device->SetFillMode(renderer::Fill_Wire);

        while (!glfwWindowShouldClose(window))
        {
            double currentTime = core::Timer::GetSeconds() - startTime;
            double frametime = currentTime - previousFrametime;
            previousFrametime = currentTime;

            _snprintf_s(titleBuffer, sizeof(titleBuffer), "FrameTime: %.0fms", frametime * core::Timer::MsPerSecond);
            glfwSetWindowTitle(window, titleBuffer);

            device->Clear(renderer::Clear_All, 0xff203040);

            glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), (float)(currentTime*0.2), glm::vec3(0.0f, 1.0f, 0.0f));
            model.Render(device, view * rotate);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    } catch (const std::exception &e)
    {
        DEBUG_LOG_TEXT("*** Error: %s", e.what());
        MessageBox(0, e.what(), 0, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
        returnValue = 1;
    }

    glfwTerminate();
    core::LogSaveToFile("debuglog.txt");
    return returnValue;;
}