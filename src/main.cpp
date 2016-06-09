#include "Precompiled.h"
#include "Base/Timer.h"
#include "Base/Profiler.h"
#include "Base/Debug.h"
#include "Base/File.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"
#include "Game/GameApp.h"
#include "Game/Camera.h"
#include "Game/SkyBox.h"
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
    std::shared_ptr<renderer::ISamplerState> modelSampler;
    std::shared_ptr<renderer::Model> model;
    renderer::Surface skyboxSurface;

    std::shared_ptr<renderer::IShaderProgram> program;
    std::shared_ptr<renderer::IShaderProgram> skyboxProgram;
    std::shared_ptr<renderer::IShaderProgram> debugNormalProgram;
};

bool GameApp::Init()
{
    program = renderer::CreateShaderProgramFromFiles(renderDevice, "assets/shaders/blinn-phong.vert", "assets/shaders/blinn-phong-bump.frag");
    THROW_FATAL_COND(!program, "Fatal: Failed to create shader program!");
    skyboxProgram = renderer::CreateShaderProgramFromFiles(renderDevice, "assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    THROW_FATAL_COND(!skyboxProgram, "Fatal: Failed to create shader program!");
    debugNormalProgram = renderer::CreateShaderProgramFromFiles(renderDevice,
                                                                "assets/shaders/debug-normals.vert",
                                                                "assets/shaders/debug-normals.geom",
                                                                "assets/shaders/debug-normals.frag");
    THROW_FATAL_COND(!debugNormalProgram, "Fatal: Failed to create shader program!");
    renderDevice->SetShaderProgram(program);

    const char *skyboxFilenames[] = 
    {
        "assets/nv_sky/right.jpg",
        "assets/nv_sky/left.jpg",
        "assets/nv_sky/bottom.jpg",
        "assets/nv_sky/top.jpg",
        "assets/nv_sky/back.jpg",
        "assets/nv_sky/front.jpg",
    };

    CreateSkyBoxSurface(renderDevice, skyboxSurface, skyboxFilenames);

    camera.SetPerspectiveMatrix(45.0f, 16.0f / 10.0f, 0.1f, 1000.0f);
    model = renderer::Model::LoadOBJ(renderDevice, "assets/crytek-sponza/sponza.obj");
    //model = renderer::Model::LoadOBJ(renderDevice, "assets/Street environment_V01.obj");

    // move controls
    BindKey(GLFW_KEY_W,     [=](void) { cameraControl.MoveForward((float)frameTimer); });
    BindKey(GLFW_KEY_S,     [=](void) { cameraControl.MoveForward((float)-frameTimer); });
    BindKey(GLFW_KEY_A,     [=](void) { cameraControl.MoveRight((float)-frameTimer); });
    BindKey(GLFW_KEY_D,     [=](void) { cameraControl.MoveRight((float)frameTimer); });
    BindKey(GLFW_KEY_SPACE, [=](void) { cameraControl.MoveUp((float)frameTimer); });
    BindKey(GLFW_KEY_C,     [=](void) { cameraControl.MoveUp((float)-frameTimer); });
    BindMouseMove([=](const MouseStateInfo &mouseState) { if (mouseState.isGrabbed) { cameraControl.RotateLookAtDirection(mouseState.offset); } });

    // sampler state control
    modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Anisotropic));
    BindKey(GLFW_KEY_1, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Point)); });
    BindKey(GLFW_KEY_2, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Bilinear)); });
    BindKey(GLFW_KEY_3, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Trilinear)); });
    BindKey(GLFW_KEY_4, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Anisotropic)); });
    BindKey(GLFW_KEY_5, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(renderer::SamplerFilter_Point, renderer::SamplerWrap_Repeat, renderer::SamplerWrap_Repeat, renderer::SamplerWrap_Repeat, 0, 3)); });

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
        renderDevice->SetShaderProgram(skyboxProgram);
        renderDevice->Render(&skyboxSurface, &camera);

        if (modelSampler)
        {
            renderDevice->SetSamplerState(0, modelSampler);
        }
        renderDevice->SetShaderProgram(program);
        model->Render(renderDevice, &camera);

        // DEBUG DRAW NORMALS
        //renderDevice->SetShaderProgram(debugNormalProgram);
        //model->Render(renderDevice, &camera);
    }
}

int main()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    return RunGameApplication(std::make_unique<GameApp>(), 1280, 720, "CybEngine");
}