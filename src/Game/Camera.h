#pragma once
#include "Renderer/RenderDevice.h"

//==============================
// Base Camera
//==============================

class BaseCamera : public renderer::ICamera
{
public:
    void SetViewMatrix(const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up);
    void SetPerspectiveMatrix(float fov, float aspect, float zNear, float zFar);

    virtual const float *GetViewPositionVector() const;
    virtual const float *GetViewMatrix() const;
    virtual const float *GetProjMatrix() const;

private:
    void UpdateProjectionMatrix();

    float verticalFOV;
    float aspectRatio;
    float nearZClip;
    float farZClip;

    glm::vec3 viewPosition;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
};

//==============================
// Camera controller
//==============================

class CameraController
{
public:
    CameraController();

    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void RotateLookAtDirection(const glm::vec2 mouseOffset);
    void UpdateCameraView(BaseCamera *camera);

    void SetWalkSpeed(float inWalkSpeed) { walkSpeed = inWalkSpeed; }

private:
    void UpdateDirectionVector();

    float walkSpeed;
    float mouseSensitivity;
    glm::vec3 position;
    glm::vec3 direction;
    float yaw;
    float pitch;
};