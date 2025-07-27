#include "SolarSystem.h"
#include <iostream>

SolarSystem::SolarSystem()
{
}

SolarSystem::~SolarSystem()
{
}

void SolarSystem::initialize()
{
    std::cout << "Initializing Solar System..." << std::endl;

    // Create the sun
    m_sun = std::make_unique<Sun>("assets/textures/sun.jpg");

    // Create planets and moons
    createPlanets();
    createMoons();

    std::cout << "Solar System initialized with " << m_planets.size() << " planets" << std::endl;
}

void SolarSystem::update(float deltaTime)
{
    // Update sun
    if (m_sun)
    {
        m_sun->update(deltaTime);
    }

    // Update all planets (which will update their moons)
    for (auto &planet : m_planets)
    {
        planet->update(deltaTime);
    }
}

void SolarSystem::render(Shader &shader, unsigned int sphereVAO, int vertexCount, const glm::vec3 &cameraPos)
{
    // Set camera position for lighting calculations
    shader.setVec3("viewPos", cameraPos);

    // Render sun first
    if (m_sun)
    {
        m_sun->render(shader, sphereVAO, vertexCount);
    }

    // Render all planets (which will render their moons)
    for (auto &planet : m_planets)
    {
        planet->render(shader, sphereVAO, vertexCount);
    }
}

glm::vec3 SolarSystem::getSunPosition() const
{
    if (m_sun)
    {
        return m_sun->getPosition();
    }
    return glm::vec3(0.0f);
}

void SolarSystem::createPlanets()
{
    // Create planets with realistic-ish orbital parameters (scaled for visibility)

    // Earth
    auto earth = std::make_shared<Planet>("Earth", 0.8f, "assets/textures/earth.jpg",
                                          8.0f, 0.5f, 2.0f);
    m_planets.push_back(earth);

    // Mars
    auto mars = std::make_shared<Planet>("Mars", 0.6f, "assets/textures/mars.jpg",
                                         12.0f, 0.3f, 1.8f);
    m_planets.push_back(mars);

    // Jupiter
    auto jupiter = std::make_shared<Planet>("Jupiter", 1.5f, "assets/textures/jupiter.jpg",
                                            18.0f, 0.2f, 1.2f);
    m_planets.push_back(jupiter);

    // Venus (closer to sun)
    auto venus = std::make_shared<Planet>("Venus", 0.7f, "assets/textures/venus.jpg",
                                          5.0f, 0.7f, 1.5f);
    m_planets.push_back(venus);
}

void SolarSystem::createMoons()
{
    // Find Earth and add its moon
    for (auto &planet : m_planets)
    {
        if (planet->getName() == "Earth")
        {
            auto moon = std::make_shared<Moon>("Moon", 0.2f, "assets/textures/moon.jpg",
                                               planet.get(), 1.5f, 3.0f);
            planet->addMoon(moon);
        }

        // Add moons to Jupiter
        if (planet->getName() == "Jupiter")
        {
            auto io = std::make_shared<Moon>("Io", 0.15f, "assets/textures/moon.jpg",
                                             planet.get(), 2.0f, 4.0f);
            auto europa = std::make_shared<Moon>("Europa", 0.12f, "assets/textures/moon.jpg",
                                                 planet.get(), 2.5f, 3.0f);
            planet->addMoon(io);
            planet->addMoon(europa);
        }
    }
}
