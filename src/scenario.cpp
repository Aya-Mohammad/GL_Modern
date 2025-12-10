#include "../include/glm/glm/glm.hpp"
#include "../include/glm/glm/gtc/constants.hpp"
#include "../include/scenario.h"
#include "../include/planet.h"
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <cmath>

// Destructor implementation
CelestialBody::~CelestialBody() = default;

/**
 * @brief Creates and returns a Scenario object containing only Sun, Earth, and Moon.
 */
Scenario loadScenario_SolarSystemBasic()
{
    Scenario scenario;
    scenario.initialCameraPos = glm::vec3(0.0f, 5.0f, 20.0f);
    scenario.lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
    scenario.lightColor = glm::vec3(1.0f, 1.0f, 0.9f);

    // Define baseline parameters relative to Earth for easier scaling
    float earthRadius = 0.5f;
    float earthOrbitRadius = 10.0f;
    float earthOrbitSpeed = 0.5f;
    float earthRotationSpeed = 1.0f;

    // Sun
    CelestialBody sun(
        "Sun", 2.0f, "D:/SolarSystem-OpenGL/textures/sun.jpg", true,
        0.0f, 0.0f, 0.1f, glm::vec3(0.0f, 1.0f, 0.0f),
        std::nullopt
    );
    sun.mesh = std::make_unique<Planet>(1.0f, 64, 64);
    scenario.bodies.push_back(std::move(sun));

    // Earth
    CelestialBody earth(
        "Earth", earthRadius, "textures/earth.jpg", false,
        earthOrbitRadius, earthOrbitSpeed, earthRotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f),
        "Sun");
    earth.mesh = std::make_unique<Planet>(1.0f, 64, 64);
    scenario.bodies.push_back(std::move(earth));

    // Moon
    CelestialBody moon(
        "Moon", earthRadius * 0.27f, "textures/moon.jpg", false,
        earthRadius * 2.0f + 0.5f, earthOrbitSpeed * 2.0f, earthRotationSpeed * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f),
        "Earth"
    );
    moon.mesh = std::make_unique<Planet>(1.0f, 32, 32);
    scenario.bodies.push_back(std::move(moon));

    return scenario;
}