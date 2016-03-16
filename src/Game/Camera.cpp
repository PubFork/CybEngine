#include "stdafx.h"
#include "Camera.h"
#include "Base/Macros.h"

//==============================
// Base Camera
//==============================

void BaseCamera::SetViewMatrix(const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up)
{
    viewMatrix = glm::lookAt(pos, target, up);
}

void BaseCamera::SetPerspectiveMatrix(float fov, float aspect, float zNear, float zFar)
{
    verticalFOV = fov;
    aspectRatio = aspect;
    nearZClip = zNear;
    farZClip = zFar;
    UpdateProjectionMatrix();
}

const glm::mat4 &BaseCamera::GetViewMatrix() const
{
    return viewMatrix;
}

const glm::mat4 &BaseCamera::GetProjMatrix() const
{
    return projMatrix;
}

// private:
void BaseCamera::UpdateProjectionMatrix()
{
    projMatrix = glm::tweakedInfinitePerspective(verticalFOV, aspectRatio, nearZClip);
}

//==============================
// Vertex Input Layout
//==============================

CameraController::CameraController()
{
    walkSpeed = 8.2f;
    mouseSensitivity = 0.134f;
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    yaw = 0.0f;
    pitch = 0.0f;
    UpdateDirectionVector();
}

void CameraController::MoveForward(float dt)
{
    float moveUnits = walkSpeed * dt;
    position += direction * moveUnits;
}

void CameraController::MoveRight(float dt)
{
    double moveUnits = walkSpeed * dt;
    glm::dvec3 dir = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f));
    position += dir * moveUnits;
}

void CameraController::MoveUp(float dt)
{
    float moveUnits = walkSpeed * dt;
    position.y += (float)moveUnits;
}

void CameraController::RotateLookAtDirection(const glm::vec2 mouseOffset)
{
    yaw += mouseOffset.x * mouseSensitivity;
    pitch = Clamp(pitch - mouseOffset.y * mouseSensitivity, -89.0f, 89.0f);
    UpdateDirectionVector();
}

void CameraController::UpdateCameraView(BaseCamera *camera)
{
    camera->SetViewMatrix(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
}

// private:
void CameraController::UpdateDirectionVector()
{
    direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    direction.y = sin(glm::radians(pitch));
    direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    direction = glm::normalize(glm::vec3(direction));
}