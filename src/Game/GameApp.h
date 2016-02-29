#pragma once

#include "Renderer/RenderDevice.h"

class GameAppBase
{
public:
    virtual ~GameAppBase() {}

    void SetupWindow(uint32_t width, uint32_t height, const char *title);
    void UpdateWindowTitle(const char *title);
    void MainLoop();

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Render() = 0;

protected:
    std::shared_ptr<renderer::RenderDevice> renderDevice;
    double timer;
    double frameTimer;

private:
    void *glfwWindow;
};

int RunGameApplication(std::unique_ptr<GameAppBase> application, uint32_t width, uint32_t height, const char *title);