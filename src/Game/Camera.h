#pragma once

//==============================
// Base Camera
//==============================

class BaseCamera
{
public:
    void SetViewMatrix(const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up);
    void SetPerspectiveMatrix(float fov, float aspect, float zNear, float zFar);

    const glm::mat4 &GetViewMatrix() const;
    const glm::mat4 &GetProjMatrix() const;

private:
    void UpdateProjectionMatrix();

    float verticalFOV;
    float aspectRatio;
    float nearZClip;
    float farZClip;

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

private:
    void UpdateDirectionVector();

    float walkSpeed;
    float mouseSensitivity;
    glm::vec3 position;
    glm::vec3 direction;
    float yaw;
    float pitch;
};