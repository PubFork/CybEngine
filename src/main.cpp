#include "stdafx.h"
#include "Base/Timer.h"
#include "Renderer/Model.h"
#include "Game/GameApp.h"

class GameApp : public GameAppBase
{
public:
    virtual ~GameApp() {}
   
    virtual bool Init();
    virtual void Shutdown() {}
    virtual void Render();

private:
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
    std::shared_ptr<renderer::Model> model;
};

bool GameApp::Init()
{
    renderDevice->SetProjection(glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f));
    viewMatrix = glm::lookAt(
        glm::vec3(0, 0, -3),   // position
        glm::vec3(0, 0, 1),    // target
        glm::vec3(0, 1, 0));   // up
    model = renderer::Model::LoadOBJ(renderDevice, "assets/Street environment_V01.obj");
    return true;
}

void GameApp::Render()
{
    static char titleBuffer[64];
    _snprintf_s(titleBuffer, sizeof(titleBuffer), "CybEngine | FrameTime: %.0fms", frameTimer * HiPerformanceTimer::MsPerSecond);
    UpdateWindowTitle(titleBuffer);

    modelMatrix = glm::rotate(glm::mat4(1.0f), (float)(timer*0.4), glm::vec3(0.0f, 1.0f, 0.0f));
    renderDevice->Clear(renderer::Clear_All, glm::vec4(0.125f, 0.188f, 0.250f, 1.0f));
    model->Render(renderDevice, viewMatrix * modelMatrix);
}

int main()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    return RunGameApplication(std::make_unique<GameApp>(), 1024, 768, "CybEngine");
}