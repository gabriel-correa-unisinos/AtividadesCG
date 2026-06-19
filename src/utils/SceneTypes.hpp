#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "ModelTypes.hpp"

struct Animation
{
    bool enabled = false;
    std::string type = "none";

    glm::vec3 center = glm::vec3(0.0f);
    float radius = 1.0f;
    float speed = 1.0f;
};

struct SceneObject
{
    Mesh mesh;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    Animation animation;
};

struct SceneCamera
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
};

struct SceneLight
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    bool enabled = true;
};

struct SceneData
{
    std::vector<SceneObject> objects;
    std::vector<SceneLight> lights;
    SceneCamera camera;
};