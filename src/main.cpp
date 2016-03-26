#include "stdafx.h"
#include "Base/Timer.h"
#include "Base/Profiler.h"
#include "Base/Debug.h"
#include "Renderer/Model.h"
#include "Game/GameApp.h"
#include "Game/Camera.h"
#include <GLFW/glfw3.h>




class GameApp : public GameAppBase
{
public:
    virtual ~GameApp() {}
   
    virtual bool Init();
    virtual void Shutdown() {}
    virtual void Render();

private:
    BaseCamera camera;
    CameraController cameraControl;
    std::shared_ptr<renderer::Model> model;
};

bool GameApp::Init()
{
    DEBUG_LOG_TEXT("test=%x %x, asdasd=%x %x", COMPILE_TIME_CRC32_STR("test"), COMPILE_TIME_CRC32_STR("test"), COMPILE_TIME_CRC32_STR("asdasd"), COMPILE_TIME_CRC32_STR("asdasd"));
    camera.SetPerspectiveMatrix(45.0f, 16.0f / 10.0f, 0.1f, 1000.0f);
    renderDevice->SetProjection(camera.GetProjMatrix());
    model = renderer::Model::LoadOBJ(renderDevice, "assets/Street environment_V01.obj");

    // move controls
    BindKey(GLFW_KEY_W,     [=](void) { cameraControl.MoveForward((float)frameTimer); });
    BindKey(GLFW_KEY_S,     [=](void) { cameraControl.MoveForward((float)-frameTimer); });
    BindKey(GLFW_KEY_A,     [=](void) { cameraControl.MoveRight((float)-frameTimer); });
    BindKey(GLFW_KEY_D,     [=](void) { cameraControl.MoveRight((float)frameTimer); });
    BindKey(GLFW_KEY_SPACE, [=](void) { cameraControl.MoveUp((float)frameTimer); });
    BindKey(GLFW_KEY_C,     [=](void) { cameraControl.MoveUp((float)-frameTimer); });
    BindMouseMove([=](const MouseStateInfo &mouseState) { if (mouseState.isGrabbed) { cameraControl.RotateLookAtDirection(mouseState.offset); } });

    // image filter control
    BindKey(GLFW_KEY_1,     [=](void) { renderDevice->SetImageFilterMode(renderer::ImageFilter_Nearest); });
    BindKey(GLFW_KEY_2,     [=](void) { renderDevice->SetImageFilterMode(renderer::ImageFilter_Linear); });
    BindKey(GLFW_KEY_3,     [=](void) { renderDevice->SetImageFilterMode(renderer::ImageFilter_Anisotropic); });

    return true;
}

void GameApp::Render()
{
    {
        SCOOPED_PROFILE_EVENT("Update");
        static char titleBuffer[64] = {};
        _snprintf_s(titleBuffer, sizeof(titleBuffer), "CybEngine | FrameTime: %.0fms", frameTimer * HiPerformanceTimer::MsPerSecond);
        UpdateWindowTitle(titleBuffer);
        cameraControl.UpdateCameraView(&camera);
    }

    {
        SCOOPED_PROFILE_EVENT("Clear");
        renderDevice->Clear(renderer::Clear_All, glm::vec4(0.125f, 0.188f, 0.250f, 1.0f));
    }

    {
        SCOOPED_PROFILE_EVENT("Draw");
        model->Render(renderDevice, camera.GetViewMatrix());
    }
}

int main()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    return RunGameApplication(std::make_unique<GameApp>(), 1280, 720, "CybEngine");
}