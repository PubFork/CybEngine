#include "stdafx.h"
#include "GameApp.h"
#include "Base/Debug.h"
#include "Base/Timer.h"
#include "Base/Profiler.h"
#include <Windows.h>
#include <GLFW/glfw3.h>

void GameAppBase::SetupWindow(uint32_t width, uint32_t height, const char *title)
{
    // initialize glfw
    glfwSetErrorCallback([](int, const char *msg) { throw FatalException(msg); });
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 64);
    
    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwWindow = window;
    glfwSwapInterval(0);

    // initialize render device
    renderDevice = renderer::CreateRenderDeviceGL();
    renderDevice->Init();
}

void GameAppBase::UpdateWindowTitle(const char *title)
{
    GLFWwindow *window = (GLFWwindow*)glfwWindow;
    glfwSetWindowTitle(window, title);
}

void GameAppBase::MainLoop()
{
    GLFWwindow *window = (GLFWwindow*)glfwWindow;

    while (!glfwWindowShouldClose(window))
    {
        double timerStart = HiPerformanceTimer::GetSeconds();
        Render();

        double timerEnd = HiPerformanceTimer::GetSeconds();
        frameTimer = timerEnd - timerStart;
        timer += frameTimer;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int RunGameApplication(std::unique_ptr<GameAppBase> application, uint32_t width, uint32_t height, const char *title)
{
    int returnValue = 0;

    try
    {
        application->SetupWindow(width, height, title);
        THROW_FATAL_COND(!application->Init(), "Failed to initialize application...");
        application->Shutdown();
        application->MainLoop();
    } catch (const std::exception &e)
    {
        DEBUG_LOG_TEXT("*** Error: %s", e.what());
        MessageBox(0, e.what(), 0, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
        returnValue = 1;
    }

    application->Shutdown();
    glfwTerminate();
    globalProfiler->PrintToDebug();
    SaveDebugLogToFile("debuglog.txt");
    return returnValue;
}