#include "LoadScene.hpp"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "LoadSimpleObj.hpp"

using json = nlohmann::json;

glm::vec3 readVec3(const json &value)
{
    return glm::vec3(
        value[0].get<float>(),
        value[1].get<float>(),
        value[2].get<float>());
}

SceneData loadScene(const std::string &scenePath)
{
    SceneData scene;

    std::ifstream file(scenePath);

    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir arquivo de cena: "
                  << scenePath
                  << std::endl;

        return scene;
    }

    json data;
    file >> data;

    if (data.contains("camera"))
    {
        auto camera = data["camera"];

        if (camera.contains("position"))
            scene.camera.position = readVec3(camera["position"]);

        if (camera.contains("front"))
            scene.camera.front = readVec3(camera["front"]);

        if (camera.contains("up"))
            scene.camera.up = readVec3(camera["up"]);

        if (camera.contains("fov"))
            scene.camera.fov = camera["fov"].get<float>();

        if (camera.contains("near"))
            scene.camera.nearPlane = camera["near"].get<float>();

        if (camera.contains("far"))
            scene.camera.farPlane = camera["far"].get<float>();
    }

    if (data.contains("objects"))
    {
        for (const auto &objectData : data["objects"])
        {
            SceneObject object;

            std::string objPath = objectData["obj"].get<std::string>();

            object.mesh = loadSimpleOBJ(objPath);

            if (objectData.contains("position"))
                object.position = readVec3(objectData["position"]);

            if (objectData.contains("rotation"))
                object.rotation = readVec3(objectData["rotation"]);

            if (objectData.contains("scale"))
                object.scale = readVec3(objectData["scale"]);

            if (objectData.contains("animation"))
            {
                auto anim = objectData["animation"];

                if (anim.contains("enabled"))
                    object.animation.enabled = anim["enabled"].get<bool>();

                if (anim.contains("type"))
                    object.animation.type = anim["type"].get<std::string>();

                if (anim.contains("center"))
                    object.animation.center = readVec3(anim["center"]);

                if (anim.contains("radius"))
                    object.animation.radius = anim["radius"].get<float>();

                if (anim.contains("speed"))
                    object.animation.speed = anim["speed"].get<float>();
            }

            scene.objects.push_back(object);
        }
    }

    if (data.contains("lights"))
    {
        for (const auto &lightData : data["lights"])
        {
            SceneLight light;

            if (lightData.contains("position"))
                light.position = readVec3(lightData["position"]);

            if (lightData.contains("color"))
                light.color = readVec3(lightData["color"]);

            if (lightData.contains("intensity"))
                light.intensity = lightData["intensity"].get<float>();

            if (lightData.contains("enabled"))
                light.enabled = lightData["enabled"].get<bool>();

            scene.lights.push_back(light);
        }
    }

    return scene;
}