#pragma once

#include "Renderer/RenderDevice.h"

struct MouseStateInfo
{
    glm::vec2 position;
    glm::vec2 offset;

    bool button[8];
    bool inClientArea;
    bool isGrabbed;
};

struct GLFWCallbackPointerData
{
    MouseStateInfo mouseState;
    bool keyState[348];                                             // GLFW_KEY_LAST
    std::function<void(const MouseStateInfo &)> mouseMoveCallback;
    std::unordered_map<int, std::function<void(void)>> keyBinds;
};

class GameAppBase
{
public:
    GameAppBase();
    virtual ~GameAppBase() {}

    void SetupWindow(uint32_t width, uint32_t height, const char *title);
    void UpdateWindowTitle(const char *title);
    bool SetMouseCursor(const char *filename, int xHot, int yHot);
    void MainLoop();

    void BindKey(int key, std::function<void(void)> fun);
    void BindMouseMove(std::function<void(const MouseStateInfo &)> callback);

    std::shared_ptr<renderer::IRenderDevice> GetRenderDevice() { return renderDevice; }

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Render() = 0;

protected:
    std::shared_ptr<renderer::IRenderDevice> renderDevice;
    double timer;
    double frameTimer;

private:
    void *glfwWindow;
    GLFWCallbackPointerData callbackData;
};

int RunGameApplication(std::unique_ptr<GameAppBase> application, uint32_t width, uint32_t height, const char *title);