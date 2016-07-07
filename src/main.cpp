#include "Precompiled.h"
#include "Base/Timer.h"
#include "Base/Debug.h"
#include "Base/File.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"
#include "Game/GameApp.h"
#include "Game/Camera.h"
#include "Game/SkyBox.h"
#include <GLFW/glfw3.h>

#include "Base/Memory.h"
#include "Base/Sys.h"
#include "Base/ParallelJobQueue.h"

#include "Renderer/CommandBuffer.h"
#include "Game/Entry.h"


class GameApp : public GameAppBase
{
public:
    virtual ~GameApp() {}
   
    virtual bool Init();
    virtual void Shutdown() {}
    virtual void UpdateGameLogic();
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

void PrintStringJob(void *data)
{
    const char *str = (const char *)data;
    Sys_Printf("Thread %u: %s\n", GetThreadID(), str);
}

bool GameApp::Init()
{
    parallel_job_queue Queue;
    CreateParallelJobQueue(&Queue, 8);
    Sys_Sleep(1);

    SubmitJob(&Queue, &PrintStringJob, "String00");
    SubmitJob(&Queue, &PrintStringJob, "String01");
    SubmitJob(&Queue, &PrintStringJob, "String02");
    SubmitJob(&Queue, &PrintStringJob, "String03");
    SubmitJob(&Queue, &PrintStringJob, "String04");
    SubmitJob(&Queue, &PrintStringJob, "String05");
    SubmitJob(&Queue, &PrintStringJob, "String07");
    SubmitJob(&Queue, &PrintStringJob, "String08");
    SubmitJob(&Queue, &PrintStringJob, "String09");

    SubmitJob(&Queue, &PrintStringJob, "String10");
    SubmitJob(&Queue, &PrintStringJob, "String11");
    SubmitJob(&Queue, &PrintStringJob, "String12");
    SubmitJob(&Queue, &PrintStringJob, "String13");
    SubmitJob(&Queue, &PrintStringJob, "String14");
    SubmitJob(&Queue, &PrintStringJob, "String15");
    SubmitJob(&Queue, &PrintStringJob, "String16");
    SubmitJob(&Queue, &PrintStringJob, "String17");
    SubmitJob(&Queue, &PrintStringJob, "String18");
    SubmitJob(&Queue, &PrintStringJob, "String19");
    
    WaitForQueueToFinish(&Queue);

    program = renderer::CreateShaderProgramFromFiles(renderDevice, "assets/shaders/blinn-phong-bump.vert", "assets/shaders/blinn-phong-bump.frag");
    RETURN_FALSE_IF(!program);
    skyboxProgram = renderer::CreateShaderProgramFromFiles(renderDevice, "assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    RETURN_FALSE_IF(!skyboxProgram);
    debugNormalProgram = renderer::CreateShaderProgramFromFiles(renderDevice,
                                                                "assets/shaders/debug-normals.vert",
                                                                "assets/shaders/debug-normals.geom",
                                                                "assets/shaders/debug-normals.frag");
    RETURN_FALSE_IF(!debugNormalProgram);
    debugNormalProgram->SetFloat(debugNormalProgram->GetParameterLocation("debugLineLength"), 0.2f);
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

    RETURN_FALSE_IF(!CreateSkyBoxSurface(renderDevice, skyboxSurface, skyboxFilenames));

    camera.SetPerspectiveMatrix(45.0f, 16.0f / 10.0f, 0.1f, 1000.0f);
    //model = renderer::LoadOBJModel(renderDevice, "assets/crytek-sponza/sponza.obj");
    cameraControl.SetWalkSpeed(8.5f);
    model = renderer::LoadOBJModel(renderDevice, "assets/Street environment_V01.obj");
    RETURN_FALSE_IF(!model);

    // move controls
    BindKey(GLFW_KEY_W,     [=](void) { cameraControl.MoveForward((float)frameTimer); });
    BindKey(GLFW_KEY_S,     [=](void) { cameraControl.MoveForward((float)-frameTimer); });
    BindKey(GLFW_KEY_A,     [=](void) { cameraControl.MoveRight((float)-frameTimer); });
    BindKey(GLFW_KEY_D,     [=](void) { cameraControl.MoveRight((float)frameTimer); });
    BindKey(GLFW_KEY_SPACE, [=](void) { cameraControl.MoveUp((float)frameTimer); });
    BindKey(GLFW_KEY_C,     [=](void) { cameraControl.MoveUp((float)-frameTimer); });
    BindMouseMove([=](const MouseStateInfo &mouseState) { if (mouseState.isGrabbed) { cameraControl.RotateLookAtDirection(mouseState.offset); } });

    // sampler state control
    modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(SamplerFilter_Anisotropic));
    BindKey(GLFW_KEY_1, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(SamplerFilter_Point)); });
    BindKey(GLFW_KEY_2, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(SamplerFilter_Bilinear)); });
    BindKey(GLFW_KEY_3, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(SamplerFilter_Trilinear)); });
    BindKey(GLFW_KEY_4, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(SamplerFilter_Anisotropic)); });
    BindKey(GLFW_KEY_5, [=](void) { modelSampler = renderDevice->CreateSamplerState(renderer::SamplerStateInitializer(SamplerFilter_Point, SamplerWrap_Repeat, SamplerWrap_Repeat, SamplerWrap_Repeat, 0, 3)); });

    return true;
}

void GameApp::UpdateGameLogic()
{
    static char titleBuffer[64] = {};
    _snprintf_s(titleBuffer, sizeof(titleBuffer), "CybEngine | FrameTime: %.0fms", frameTimer * HiPerformanceTimer::MsPerSecond);
    UpdateWindowTitle(titleBuffer);

    cameraControl.UpdateCameraView(&camera);
}

void GameApp::Render()
{
    renderDevice->Clear(Clear_Color | Clear_Depth, glm::vec4(0.125f, 0.188f, 0.250f, 1.0f));

    if (modelSampler)
    {
        renderDevice->SetSamplerState(0, modelSampler);
    }

    // render model
    renderDevice->SetShaderProgram(program);
    model->Render(renderDevice, &camera);

    // render model tangent-space vectors
    //renderDevice->SetShaderProgram(debugNormalProgram);
    //model->Render(renderDevice, &camera);

    // render skybox
    renderDevice->SetShaderProgram(skyboxProgram);
    renderDevice->Render(&skyboxSurface, &camera);
}

void GameTestRender(render_command_buffer *RenderCommands)
{
    command_entry_test1 *test1 = PushRenderCommand(RenderCommands, command_entry_test1);
    test1->A = 1;
    test1->B = 2;
    test1->C = 3;
    test1->D = 4;

    command_entry_test2 *test2 = PushRenderCommand(RenderCommands, command_entry_test2);
    test2->A = 5.5f;
    test2->B = 6.6f;
    test2->C = 7.7f;
    test2->D = 8.8f;
}

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    game_entry_params Params = {};
    Params.PermanentStorageSize = Megabytes(256);
    Params.RenderCommandStorageSize = Megabytes(64);
    Params.TransientStorageSize = Gigabytes(1);
    Params.RenderCallback = GameTestRender;
    GameEntryFunction(&Params);

    return RunGameApplication(new GameApp, 1280, 720, "CybEngine");
}