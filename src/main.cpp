#include "stdafx.h"
#include "Base/Timer.h"
#include "Base/Profiler.h"
#include "Renderer/Model.h"
#include "Game/GameApp.h"
#include <GLFW/glfw3.h>

class CameraBase
{
public:
    CameraBase() :
        fov(45.0f),
        aspect(16.0f / 9.0f),
        zNear(0.1f), 
        zFar(1000.0f)
    { 
        UpdateProjectionMatrix(fov, aspect, zNear, zFar);
        UpdateViewMatrix(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void UpdateProjectionMatrix(float Hfov, float aspectRatio, float near, float far)
    {
        fov = Hfov;
        aspect = aspectRatio;
        zNear = near;
        zFar = far;
        projectionMatrix = glm::perspective(fov, aspect, zNear, zFar);
    }

    void UpdateViewMatrix(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
    {
        viewMatrix = glm::lookAt(position, target, up);
    }

    const glm::mat4 &GetProjectionMatrix() const { return projectionMatrix; }
    const glm::mat4 &GetViewMatrix() const { return viewMatrix; }

private:
    float fov;
    float aspect;
    float zNear;
    float zFar;
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
};

class PlayerBase
{
public:
    PlayerBase()
    {
        position = glm::vec3(0.0f, 2.0f, 0.0f);
        target = glm::vec3(0.0f, 2.0f, 1.0f);
        moveSpeed = 5.8;
    }

    void MoveForward(double dt)
    {
        double moveUnits = moveSpeed * dt;
        glm::dvec3 dir = glm::normalize(target - position);
        position += dir * moveUnits;
        target += dir * moveUnits;
    }

    void MoveRight(double dt) 
    {
        double moveUnits = moveSpeed * dt;
        glm::dvec3 dir = glm::cross(glm::normalize(target - position), glm::vec3(0.0f, 1.0f, 0.0f));
        position += dir * moveUnits;
        target += dir * moveUnits;
    }

    void MoveUp(double dt)
    {
        double moveUnits = moveSpeed * dt;
        position.y += moveUnits;
        target.y += moveUnits;
    }

    void UpdateCamera()
    {
        camera.UpdateViewMatrix(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    const CameraBase *GetCamera() const { return &camera; }
    
private:
    double moveSpeed;
    glm::vec3 position;
    glm::vec3 target;
    CameraBase camera;
};

class GameApp : public GameAppBase
{
public:
    virtual ~GameApp() {}
   
    virtual bool Init();
    virtual void Shutdown() {}
    virtual void Render();

private:
    PlayerBase player;
    //glm::mat4 modelMatrix;
    std::shared_ptr<renderer::Model> model;
};

bool GameApp::Init()
{
    renderDevice->SetProjection(player.GetCamera()->GetProjectionMatrix());
    /*
    viewMatrix = glm::lookAt(
        glm::vec3(0, 0, -3),   // position
        glm::vec3(0, 0, 1),    // target
        glm::vec3(0, 1, 0));   // up
    */
    model = renderer::Model::LoadOBJ(renderDevice, "assets/Street environment_V01.obj");

    BindKey(GLFW_KEY_W,     [=](void) { player.MoveForward(frameTimer); });
    BindKey(GLFW_KEY_S,     [=](void) { player.MoveForward(-frameTimer); });
    BindKey(GLFW_KEY_A,     [=](void) { player.MoveRight(-frameTimer); });
    BindKey(GLFW_KEY_D,     [=](void) { player.MoveRight(frameTimer); });
    BindKey(GLFW_KEY_SPACE, [=](void) { player.MoveUp(frameTimer); });
    BindKey(GLFW_KEY_C,     [=](void) { player.MoveUp(-frameTimer); });

    return true;
}

void GameApp::Render()
{
    {
        SCOOPED_PROFILE_EVENT("Update_Frame");
        static char titleBuffer[64] = {};
        _snprintf_s(titleBuffer, sizeof(titleBuffer), "CybEngine | FrameTime: %.0fms", frameTimer * HiPerformanceTimer::MsPerSecond);
        UpdateWindowTitle(titleBuffer);

        //player.MoveForward(frameTimer);
        player.UpdateCamera();
        //modelMatrix = glm::rotate(glm::mat4(1.0f), (float)(timer*0.4), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    {
        SCOOPED_PROFILE_EVENT("Clear_Screen");
        renderDevice->Clear(renderer::Clear_All, glm::vec4(0.125f, 0.188f, 0.250f, 1.0f));
    }

    {
        SCOOPED_PROFILE_EVENT("Draw_Surfaces");
        model->Render(renderDevice, player.GetCamera()->GetViewMatrix());
    }
}

int main()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    return RunGameApplication(std::make_unique<GameApp>(), 1280, 720, "CybEngine");
}