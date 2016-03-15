#include "stdafx.h"
#include "GameApp.h"
#include "Base/Debug.h"
#include "Base/Timer.h"
#include "Base/FileUtils.h"
#include "Base/Profiler.h"
#include "Base/SysInfo.h"
#include "Renderer/stb_image.h"
#include <Windows.h>
#include <GLFW/glfw3.h>

static void KeyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    GLFWCallbackPointerData *data = (GLFWCallbackPointerData*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS)
    {
        data->keyState[key] = true;
    }

    if (action == GLFW_RELEASE)
    {
        data->keyState[key] = false;
    }
}

static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    GLFWCallbackPointerData *data = (GLFWCallbackPointerData*)glfwGetWindowUserPointer(window);
    MouseStateInfo &state = data->mouseState;
    
    glm::vec2 mousePos(xpos, ypos);
    state.offset = mousePos - state.position;
    state.position = mousePos;

    if (data->mouseMoveCallback)
    {
        data->mouseMoveCallback(state);
    }
}

static void CursorEnterCallback(GLFWwindow *window, int entered)
{
    GLFWCallbackPointerData *data = (GLFWCallbackPointerData*)glfwGetWindowUserPointer(window);
    MouseStateInfo &state = data->mouseState;
    state.inClientArea = entered > 0 ? true : false;
}

static void MouseButtonCallback(GLFWwindow *window, int button, int action, int)
{
    GLFWCallbackPointerData *data = (GLFWCallbackPointerData*)glfwGetWindowUserPointer(window);
    MouseStateInfo &state = data->mouseState;
    state.button[button] = action > 0 ? true : false;

    // grab/release mouse with right-button
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action > 0)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            state.isGrabbed = true;
        } else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            state.isGrabbed = false;
        }
    }
}

GameAppBase::GameAppBase() :
    timer(0.0),
    frameTimer(0.0)
{
    memset(callbackData.keyState, 0, sizeof(callbackData.keyState));
}

void GameAppBase::SetupWindow(uint32_t width, uint32_t height, const char *title)
{
    // initialize glfw
    glfwSetErrorCallback([](int, const char *msg) { throw FatalException(msg); });
    THROW_FATAL_COND(glfwInit() != GL_TRUE, "glfwInit() failed");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 64);
    
    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwWindow = window;
    glfwSwapInterval(1);
    
    // setup glfw input callbacks
    memset(&callbackData.mouseState, 0, sizeof(callbackData.mouseState));
    memset(&callbackData.keyState, 0, sizeof(callbackData.keyState));

    glfwSetWindowUserPointer(window, &callbackData);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // initialize render device
    renderDevice = renderer::CreateRenderDeviceGL();
    renderDevice->Init();

    // load custom cursor (if avialable)
    SetMouseCursor("assets/cursor.png", 0, 0);
}

void GameAppBase::UpdateWindowTitle(const char *title)
{
    GLFWwindow *window = (GLFWwindow*)glfwWindow;
    glfwSetWindowTitle(window, title);
}

bool GameAppBase::SetMouseCursor(const char *filename, int xHot, int yHot)
{
    FileReader file(filename);
    if (file.IsOpen())
    {
        int bpp;
        GLFWimage image;
        image.pixels = stbi_load_from_memory((const stbi_uc*)file.RawBuffer(), (int)file.Size(), &image.width, &image.height, &bpp, 4);

        GLFWcursor *cursor = glfwCreateCursor(&image, xHot, yHot);
        glfwSetCursor((GLFWwindow*)glfwWindow, cursor);
        stbi_image_free(image.pixels);
        return true;
    }

    return false;
}

void GameAppBase::MainLoop()
{
    GLFWwindow *window = (GLFWwindow*)glfwWindow;

    while (!glfwWindowShouldClose(window))
    {
        SCOOPED_PROFILE_EVENT("MainLoop");
        double timerStart = HiPerformanceTimer::GetSeconds();
        
        {
            SCOOPED_PROFILE_EVENT("Render");
            Render();
        }
        
        {
            SCOOPED_PROFILE_EVENT("SwapBuffers");
            glfwSwapBuffers(window);
        }

        {
            SCOOPED_PROFILE_EVENT("PollEvents");
            glfwPollEvents();

            // process key bindings
            for (auto keyBinding : callbackData.keyBinds)
            {
                if (callbackData.keyState[keyBinding.first])
                {
                    keyBinding.second();
                }
            }
        }

        double timerEnd = HiPerformanceTimer::GetSeconds();
        frameTimer = timerEnd - timerStart;
        timer += frameTimer;
    }
}

void GameAppBase::BindKey(int key, std::function<void(void)> fun)
{
    callbackData.keyBinds[key] = fun;
}

void GameAppBase::BindMouseMove(std::function<void(const MouseStateInfo &)> callback)
{
    callbackData.mouseMoveCallback = callback;
}

int RunGameApplication(std::unique_ptr<GameAppBase> application, uint32_t width, uint32_t height, const char *title)
{
    int returnValue = 0;

    DEBUG_LOG_TEXT("CPU: %s", GetProcessorInfo().c_str());
    std::vector<std::string> gpuList = GetGraphicCardList();
    uint16_t gpuNum = 0;
    for (const auto &gpu : gpuList)
    {
        DEBUG_LOG_TEXT("GPU(%d): %s", gpuNum, gpu.c_str());
        gpuNum++;
    }

    try
    {
        {
            SCOOPED_PROFILE_EVENT("Init");
            application->SetupWindow(width, height, title);
            THROW_FATAL_COND(!application->Init(), "Failed to initialize application...");
        }
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