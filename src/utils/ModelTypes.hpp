#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

struct Material
{
    std::string name;

    glm::vec3 Ka = glm::vec3(1.0f);
    glm::vec3 Kd = glm::vec3(1.0f);
    glm::vec3 Ks = glm::vec3(0.5f);

    float shininess = 32.0f;

    GLuint textureId = 0;
};

struct SubMesh
{
    GLuint VAO;
    GLuint VBO;
    int nVertices;

    Material material;
};

struct BoundingBox
{
    glm::vec3 min = glm::vec3(0.0f);
    glm::vec3 max = glm::vec3(0.0f);
};

struct Mesh
{
    std::vector<SubMesh> submeshes;
    BoundingBox bbox;
};