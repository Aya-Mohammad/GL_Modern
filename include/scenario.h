#ifndef SCENARIO_H
#define SCENARIO_H

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "glm/glm/glm.hpp"
#include "glad/glad.h"
#include "planet.h"

class Planet;
class Shader;

struct CelestialBody
{
    std::string name;
    float radius;
    std::string texturePath;
    bool isEmissive;

    // Animation parameters
    float orbitRadius;
    float orbitSpeed;
    float rotationSpeed;
    glm::vec3 rotationAxis;

    // Hierarchy
    std::optional<std::string> parentName;

    // Rendering data (initialized later)
    unsigned int textureID = 0;
    glm::mat4 currentModelMatrix = glm::mat4(1.0f);
    std::unique_ptr<Planet> mesh = nullptr;

    CelestialBody(std::string n, float r, std::string tex, bool emissive,
                  float orbRad, float orbSpd, float rotSpd, glm::vec3 rotAx,
                  std::optional<std::string> parent)
        : name(std::move(n)), radius(r), texturePath(std::move(tex)), isEmissive(emissive),
          orbitRadius(orbRad), orbitSpeed(orbSpd), rotationSpeed(rotSpd),
          rotationAxis(rotAx), parentName(std::move(parent)) {}

    // Explicitly default the default constructor (needed due to other constructors)
    CelestialBody() = default;

    // Explicitly define/default special member functions because we have unique_ptr
    ~CelestialBody();
    CelestialBody(const CelestialBody &) = delete;
    CelestialBody &operator=(const CelestialBody &) = delete;
    CelestialBody(CelestialBody &&) = default;
    CelestialBody &operator=(CelestialBody &&) = default;

    void render()
    {
        if (mesh)
        {
            glBindTexture(GL_TEXTURE_2D, textureID);
            mesh->draw();
        }
    }
};

struct Scenario
{
    std::vector<CelestialBody> bodies;
    glm::vec3 initialCameraPos;
    glm::vec3 lightPos;
    glm::vec3 lightColor;
};
Scenario loadScenario_SolarSystemBasic();
#endif
