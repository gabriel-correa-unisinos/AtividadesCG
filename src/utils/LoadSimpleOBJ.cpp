/*
 *  Codificado por Rossana Baptista Queiroz
 *  para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 *  Versão inicial: 07/04/2017
 *  Última atualização: 14/03/2025
 *
 *  Este arquivo contém a função `loadSimpleOBJ`, responsável por carregar arquivos
 *  no formato Wavefront .OBJ e armazenar seus vértices em um VAO para renderização
 *  com OpenGL.
 *
 *  Forma de uso (carregamento de um .obj)
 *  -----------------
 *  ...
 *  int nVertices;
 *  GLuint objVAO = loadSimpleOBJ("../Modelos3D/Cube.obj", nVertices);
 *  ...
 *
 *  Chamada de desenho (Polígono Preenchido - GL_TRIANGLES), no loop do programa:
 *  ----------------------------------------------------------
 *  ...
 *  glBindVertexArray(objVAO);
 *  glDrawArrays(GL_TRIANGLES, 0, nVertices);
 *
 */

// Cabeçalhos necessários (para esta função), acrescentar ao seu código
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include "LoadMTL.hpp"
#include "ModelTypes.hpp"
#include <cfloat>
#include <algorithm>

std::string getBasePath(const std::string &filePath)
{
    size_t pos = filePath.find_last_of("/\\");

    if (pos == std::string::npos)
        return "";

    return filePath.substr(0, pos + 1);
}

Mesh loadSimpleOBJ(string filePATH)
{
    Mesh mesh;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::map<std::string, Material> materials;
    std::map<std::string, std::vector<GLfloat>> buffersByMaterial;

    std::string currentMaterialName = "default";
    std::string basePath = getBasePath(filePATH);

    std::ifstream arqEntrada(filePATH.c_str());
    if (!arqEntrada.is_open())
    {
        std::cerr << "Erro ao tentar ler o arquivo " << filePATH << std::endl;
        return mesh;
    }

    std::string line;
    while (std::getline(arqEntrada, line))
    {
        std::istringstream ssline(line);
        std::string word;
        ssline >> word;

        if (word.empty() || word[0] == '#')
            continue;

        if (word == "mtllib")
        {
            std::string mtlFile;
            ssline >> mtlFile;

            std::string mtlPath = basePath + mtlFile;

            materials = loadMTL(mtlPath, basePath);
        }
        else if (word == "usemtl")
        {
            ssline >> currentMaterialName;
        }
        else if (word == "v")
        {
            glm::vec3 vertice;
            ssline >> vertice.x >> vertice.y >> vertice.z;
            vertices.push_back(vertice);
        }
        else if (word == "vt")
        {
            glm::vec2 vt;
            ssline >> vt.x >> vt.y;
            texCoords.push_back(vt);
        }
        else if (word == "vn")
        {
            glm::vec3 normal;
            ssline >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (word == "f")
        {
            while (ssline >> word)
            {
                int vi = -1, ti = -1, ni = -1;

                std::stringstream ss(word);

                std::string vStr, tStr, nStr;

                std::getline(ss, vStr, '/');
                std::getline(ss, tStr, '/');
                std::getline(ss, nStr, '/');

                if (!vStr.empty())
                    vi = std::stoi(vStr) - 1;
                if (!tStr.empty())
                    ti = std::stoi(tStr) - 1;
                if (!nStr.empty())
                    ni = std::stoi(nStr) - 1;

                // =========================
                // VALIDAÇÃO CRÍTICA (EVITA SEGFAULT)
                // =========================
                if (vi < 0 || vi >= vertices.size())
                    continue;

                glm::vec3 pos = vertices[vi];

                glm::vec3 normal(0.0f, 0.0f, 1.0f);
                glm::vec2 texCoord(0.0f, 0.0f);

                if (ni >= 0 && ni < normals.size())
                    normal = normals[ni];

                if (ti >= 0 && ti < texCoords.size())
                    texCoord = texCoords[ti];

                auto &vBuffer = buffersByMaterial[currentMaterialName];

                vBuffer.push_back(pos.x);
                vBuffer.push_back(pos.y);
                vBuffer.push_back(pos.z);

                vBuffer.push_back(normal.x);
                vBuffer.push_back(normal.y);
                vBuffer.push_back(normal.z);

                vBuffer.push_back(texCoord.x);
                vBuffer.push_back(texCoord.y);
            }
        }
    }

    arqEntrada.close();

    if (!vertices.empty())
    {
        BoundingBox bbox;

        bbox.min = glm::vec3(FLT_MAX);
        bbox.max = glm::vec3(-FLT_MAX);

        for (auto &v : vertices)
        {
            bbox.min.x = std::min(bbox.min.x, v.x);
            bbox.min.y = std::min(bbox.min.y, v.y);
            bbox.min.z = std::min(bbox.min.z, v.z);

            bbox.max.x = std::max(bbox.max.x, v.x);
            bbox.max.y = std::max(bbox.max.y, v.y);
            bbox.max.z = std::max(bbox.max.z, v.z);
        }

        mesh.bbox = bbox;
    }

    for (auto &[materialName, vBuffer] : buffersByMaterial)
    {
        if (vBuffer.empty())
            continue;

        GLuint VBO, VAO;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(
            GL_ARRAY_BUFFER,
            vBuffer.size() * sizeof(GLfloat),
            vBuffer.data(),
            GL_STATIC_DRAW);

        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(GLfloat),
            (GLvoid *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(GLfloat),
            (GLvoid *)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(GLfloat),
            (GLvoid *)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        SubMesh submesh;

        submesh.VAO = VAO;
        submesh.VBO = VBO;
        submesh.nVertices = vBuffer.size() / 8;

        if (materials.find(materialName) != materials.end())
        {
            submesh.material = materials[materialName];
        }
        else
        {
            submesh.material.name = materialName;
        }

        mesh.submeshes.push_back(submesh);
    }

    return mesh;
}
