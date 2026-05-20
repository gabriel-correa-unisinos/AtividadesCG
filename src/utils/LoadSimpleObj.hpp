#ifndef MESH_H
#define MESH_H

#include <string>
#include <glad/glad.h>

struct Mesh
{
    GLuint VAO;
    GLuint VBO;
    GLuint textureID;
    int nVertices;
};

Mesh loadSimpleOBJ(std::string filePath);

#endif