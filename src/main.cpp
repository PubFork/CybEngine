#include "stdafx.h"
#include "Base/Timer.h"
#include "Base/Profiler.h"
#include "Base/Debug.h"
#include "Renderer/Model.h"
#include "Game/GameApp.h"
#include <GLFW/glfw3.h>

class BaseCamera
{
public:
    void SetViewMatrix(const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up)
    {
        viewMatrix = glm::lookAt(pos, target, up);
    }

    void SetPerspectiveMatrix(float fov, float aspect, float zNear, float zFar)
    {
        verticalFOV = fov;
        aspectRatio = aspect;
        nearZClip = zNear;
        farZClip = zFar;
        UpdateProjectionMatrix();
    }

    const glm::mat4 &GetViewMatrix() const { return viewMatrix; }
    const glm::mat4 &GetProjMatrix() const { return projMatrix; }

private:
    void UpdateProjectionMatrix()
    {
        projMatrix = glm::tweakedInfinitePerspective(verticalFOV, aspectRatio, nearZClip);
    }

private:
    float verticalFOV;
    float aspectRatio;
    float nearZClip;
    float farZClip;

    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
};

class CameraController
{
public:
    CameraController()
    {
        walkSpeed = 8.2f;
        mouseSensitivity = 0.134f;
        position = glm::vec3(0.0f, 0.0f, 0.0f);
        yaw = 0.0f;
        pitch = 0.0f;
        UpdateDirectionVector();
    }

    void MoveForward(float dt)
    {
        float moveUnits = walkSpeed * dt;
        position += direction * moveUnits;
    }

    void MoveRight(float dt)
    {
        double moveUnits = walkSpeed * dt;
        glm::dvec3 dir = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f));
        position += dir * moveUnits;
    }

    void MoveUp(float dt)
    {
        float moveUnits = walkSpeed * dt;
        position.y += (float)moveUnits;
    }

    void RotateDirection(const glm::vec2 mouseOffset)
    {
        yaw += mouseOffset.x * mouseSensitivity;
        pitch = Clamp(pitch - mouseOffset.y * mouseSensitivity, -89.0f, 89.0f);
        UpdateDirectionVector();
    }

    void UpdateCameraView(BaseCamera *camera)
    {
        camera->SetViewMatrix(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    }

private:
    void UpdateDirectionVector()
    {
        direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        direction.y = sin(glm::radians(pitch));
        direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        direction = glm::normalize(glm::vec3(direction));
    }

    float walkSpeed;
    float mouseSensitivity;
    
    glm::vec3 position;
    glm::vec3 direction;
    float yaw;
    float pitch;
};

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
    camera.SetPerspectiveMatrix(45.0f, 16.0f / 10.0f, 0.1f, 1000.0f);
    renderDevice->SetProjection(camera.GetProjMatrix());

    model = renderer::Model::LoadOBJ(renderDevice, "assets/Street environment_V01.obj");

    BindKey(GLFW_KEY_W,     [=](void) { cameraControl.MoveForward((float)frameTimer); });
    BindKey(GLFW_KEY_S,     [=](void) { cameraControl.MoveForward((float)-frameTimer); });
    BindKey(GLFW_KEY_A,     [=](void) { cameraControl.MoveRight((float)-frameTimer); });
    BindKey(GLFW_KEY_D,     [=](void) { cameraControl.MoveRight((float)frameTimer); });
    BindKey(GLFW_KEY_SPACE, [=](void) { cameraControl.MoveUp((float)frameTimer); });
    BindKey(GLFW_KEY_C,     [=](void) { cameraControl.MoveUp((float)-frameTimer); });
    BindMouseMove([=](const MouseStateInfo &mouseState) { if (mouseState.isGrabbed) { cameraControl.RotateDirection(mouseState.offset); } });

    return true;
}

void GameApp::Render()
{
    {
        SCOOPED_PROFILE_EVENT("Update_Frame");
        static char titleBuffer[64] = {};
        _snprintf_s(titleBuffer, sizeof(titleBuffer), "CybEngine | FrameTime: %.0fms", frameTimer * HiPerformanceTimer::MsPerSecond);
        UpdateWindowTitle(titleBuffer);
        cameraControl.UpdateCameraView(&camera);
    }

    {
        SCOOPED_PROFILE_EVENT("Clear_Screen");
        renderDevice->Clear(renderer::Clear_All, glm::vec4(0.125f, 0.188f, 0.250f, 1.0f));
    }

    {
        SCOOPED_PROFILE_EVENT("Draw_Surfaces");
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