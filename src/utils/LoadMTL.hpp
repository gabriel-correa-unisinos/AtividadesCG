#pragma once

#include <map>
#include <string>

#include <glm/glm.hpp>

#include <glad/glad.h>

#include "ModelTypes.hpp"

std::map<std::string, Material>
loadMTL(
    const std::string &mtlPath,
    const std::string &textureBasePath);