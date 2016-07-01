#include "Precompiled.h"
#include "Game/GameApp.h"
#include "Base/Debug.h"
#include "Base/File.h"
#include "Base/Timer.h"
#include "Base/Sys.h"
#include "Renderer/stb_image.h"
#include "Renderer/Texture.h"
#include <GLFW/glfw3.h>

static void KeyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    GLFWCallbackPointerData *data = (GLFWCallbackPointerData*)glfwGetWindowUserPointer(window);

    switch (action)
    {
    case GLFW_PRESS: data->keyState[key] = true; break;
    case GLFW_RELEASE: data->keyState[key] = false; break;
    }
}

static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    GLFWCallbackPointerData *data = (GLFWCallbackPointerData*)glfwGetWindowUserPointer(window);
    MouseStateInfo &state = data->mouseState;
    
    const glm::vec2 mousePos(xpos, ypos);
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

bool GameAppBase::SetupWindow(uint32_t width, uint32_t height, const char *title)
{
    // initialize glfw
    glfwSetErrorCallback([](int, const char *msg) { Sys_ErrorPrintf("glfw: %s\n", msg); });
    RETURN_FALSE_IF(glfwInit() != GL_TRUE);

    /*
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    int numVideoModes = 0;
    const GLFWvidmode *videoModes = glfwGetVideoModes(monitor, &numVideoModes);
    for (int i = 0; i < numVideoModes; i++)
    {
        const GLFWvidmode *mode = &videoModes[i];
        DEBUG_LOG_TEXT("%d: %dx%d@%dhz", i, mode->width, mode->height, mode->refreshRate);
    }
    */

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_SAMPLES, 64);
    
    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwWindow = window;
    glfwSwapInterval(0);
    
    // setup glfw input callbacks
    memset(&callbackData.mouseState, 0, sizeof(callbackData.mouseState));
    memset(&callbackData.keyState, 0, sizeof(callbackData.keyState));

    glfwSetWindowUserPointer(window, &callbackData);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // initialize the renderer
    renderDevice = renderer::CreateRenderDevice();
    RETURN_FALSE_IF(!renderDevice->Init());

    // load custom cursor (if avialable)
    SetMouseCursor("assets/cursor.png", 0, 0);

    return true;
}

void GameAppBase::UpdateWindowTitle(const char *title)
{
    GLFWwindow *window = (GLFWwindow*)glfwWindow;
    glfwSetWindowTitle(window, title);
}

bool GameAppBase::SetMouseCursor(const char *filename, int xHot, int yHot)
{
    SysFile textureFile(filename, FileOpen_Read);
    if (textureFile.IsValid())
    {
        int bpp;
        GLFWimage image;

        const stbi_uc *rawTextureBuffer = new stbi_uc[textureFile.GetLength()];
        textureFile.Read((uint8_t *)rawTextureBuffer, textureFile.GetLength());
        image.pixels = stbi_load_from_memory(rawTextureBuffer, (int)textureFile.GetLength(), &image.width, &image.height, &bpp, 4);
        delete[] rawTextureBuffer;

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
        const double timerStart = HiPerformanceTimer::GetSeconds();

        {
            TIMED_NAMED_BLOCK("UpdateGameLogic");
            UpdateGameLogic();
        }
        
        {
            TIMED_NAMED_BLOCK("RenderFrame");
            Render();
            glfwSwapBuffers(window);
        }

        // process key bindings
        glfwPollEvents();
        for (auto keyBinding : callbackData.keyBinds)
        {
            if (callbackData.keyState[keyBinding.first])
            {
                keyBinding.second();
            }
        }

        frameTimer = HiPerformanceTimer::GetSeconds() - timerStart;
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

static bool InitializeApplication(GameAppBase *application, uint32_t width, uint32_t height, const char *title)
{
    TIMED_NAMED_BLOCK("Initialization");
    RETURN_FALSE_IF(!application->SetupWindow(width, height, title));
    renderer::globalTextureCache->Initialize(application->GetRenderDevice());
    RETURN_FALSE_IF(!application->Init());
    
    return true;
}

int RunGameApplication(GameAppBase *application, uint32_t width, uint32_t height, const char *title)
{
    Sys_ProcessorInfo cpuInfo;
    Sys_GetProcessorInfo(&cpuInfo);

    DebugPrintf("CPU: %s, with %d cores and %d logical processors\n", 
                   cpuInfo.cpuString, 
                   cpuInfo.numCores, 
                   cpuInfo.numLogicalProcessors);

    const std::vector<std::string> gpuList = Sys_GetGraphicCardList();
    uint16_t gpuNum = 0;
    for (const auto &gpu : gpuList)
    {
        DebugPrintf("GPU(%d): %s\n", gpuNum, gpu.c_str());
        gpuNum++;
    }

    int returnValue = EXIT_FAILURE;
    if (InitializeApplication(application, width, height, title))
    {
        application->MainLoop();
        returnValue = EXIT_SUCCESS;
    } 

    renderer::globalTextureCache->Destroy();
    application->Shutdown();
    delete application;
    glfwTerminate();
    if (returnValue != EXIT_FAILURE)
    {
        DebugLogPerformanceCounters(DebugPerformenceRecord::staticRecords);
    }

    SaveDebugLogToFile("debuglog.txt");
    return returnValue;
}