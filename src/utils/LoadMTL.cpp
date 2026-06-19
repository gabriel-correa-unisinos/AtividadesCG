#include "LoadMTL.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "LoadTexture.hpp"

std::map<std::string, Material>
loadMTL(
    const std::string &mtlPath,
    const std::string &textureBasePath)
{
    std::map<std::string, Material> materials;

    std::ifstream file(mtlPath);

    if (!file.is_open())
    {
        std::cerr
            << "Erro ao abrir arquivo MTL: "
            << mtlPath
            << std::endl;

        return materials;
    }

    Material currentMaterial;
    bool hasMaterial = false;

    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string token;
        ss >> token;

        if (token.empty())
            continue;

        if (token[0] == '#')
            continue;

        if (token == "newmtl")
        {
            if (hasMaterial)
            {
                materials[currentMaterial.name] =
                    currentMaterial;
            }

            currentMaterial = Material();

            ss >> currentMaterial.name;

            hasMaterial = true;
        }
        else if (token == "Ka")
        {
            ss >> currentMaterial.Ka.x >> currentMaterial.Ka.y >> currentMaterial.Ka.z;
        }
        else if (token == "Kd")
        {
            ss >> currentMaterial.Kd.x >> currentMaterial.Kd.y >> currentMaterial.Kd.z;
        }
        else if (token == "Ks")
        {
            ss >> currentMaterial.Ks.x >> currentMaterial.Ks.y >> currentMaterial.Ks.z;
        }
        else if (token == "Ns")
        {
            ss >> currentMaterial.shininess;
        }
        else if (token == "map_Kd")
        {
            std::string textureFile;

            ss >> textureFile;

            std::string texturePath =
                textureBasePath +
                textureFile;

            currentMaterial.textureId =
                loadTexture(texturePath);

            std::cout
                << "Textura carregada: "
                << texturePath
                << std::endl;
        }
    }

    if (hasMaterial)
    {
        materials[currentMaterial.name] =
            currentMaterial;
    }

    file.close();

    return materials;
}