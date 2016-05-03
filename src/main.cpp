#include "Precompiled.h"
#include "Base/Timer.h"
#include "Base/Profiler.h"
#include "Base/Debug.h"
#include "Base/File.h"
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
    std::shared_ptr<renderer::ISamplerState> samplerState;
    BaseCamera camera;
    CameraController cameraControl;
    std::shared_ptr<renderer::Model> model;
};

bool GameApp::Init()
{
    auto program = renderer::CreateShaderProgramFromFiles(renderDevice, "assets/shaders/standard.vert", "assets/shaders/standard.frag");
    renderDevice->SetShaderProgram(program);

    camera.SetPerspectiveMatrix(45.0f, 16.0f / 10.0f, 0.1f, 1000.0f);
    model = renderer::Model::LoadOBJ(renderDevice, "assets/Street environment_V01.obj");
    //model = renderer::Model::LoadOBJ(renderDevice, "assets/capsule.obj");

    // move controls
    BindKey(GLFW_KEY_W,     [=](void) { cameraControl.MoveForward((float)frameTimer); });
    BindKey(GLFW_KEY_S,     [=](void) { cameraControl.MoveForward((float)-frameTimer); });
    BindKey(GLFW_KEY_A,     [=](void) { cameraControl.MoveRight((float)-frameTimer); });
    BindKey(GLFW_KEY_D,     [=](void) { cameraControl.MoveRight((float)frameTimer); });
    BindKey(GLFW_KEY_SPACE, [=](void) { cameraControl.MoveUp((float)frameTimer); });
    BindKey(GLFW_KEY_C,     [=](void) { cameraControl.MoveUp((float)-frameTimer); });
    BindMouseMove([=](const MouseStateInfo &mouseState) { if (mouseState.isGrabbed) { cameraControl.RotateLookAtDirection(mouseState.offset); } });

    // sampler state control
    BindKey(GLFW_KEY_1, [=](void) { renderDevice->SetSamplerState(0, renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Point))); });
    BindKey(GLFW_KEY_2, [=](void) { renderDevice->SetSamplerState(0, renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Bilinear))); });
    BindKey(GLFW_KEY_3, [=](void) { renderDevice->SetSamplerState(0, renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Trilinear))); });
    BindKey(GLFW_KEY_4, [=](void) { renderDevice->SetSamplerState(0, renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Anisotropic, renderer::SamplerWrap_Repeat, renderer::SamplerWrap_Repeat, 16))); });
    BindKey(GLFW_KEY_5, [=](void) { renderDevice->SetSamplerState(0, renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Point, renderer::SamplerWrap_Repeat, renderer::SamplerWrap_Repeat, 0, 3))); });

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
        renderDevice->Clear(renderer::Clear_Color | renderer::Clear_Depth, glm::vec4(0.125f, 0.188f, 0.250f, 1.0f));
    }

    {
        SCOOPED_PROFILE_EVENT("Draw");
        model->Render(renderDevice, &camera);
    }
}

int main()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    return RunGameApplication(std::make_unique<GameApp>(), 1280, 720, "CybEngine");
}