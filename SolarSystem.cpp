#include "SolarSystem.h"
#include <iostream>
#include <algorithm>
#include <limits>

SolarSystem::SolarSystem() {}
SolarSystem::~SolarSystem() {}

void SolarSystem::initialize()
{
    std::cout << "Initializing Solar System..." << std::endl;

    m_sun = std::make_unique<Sun>("assets/textures/sun.jpg");

    createPlanets();
    createMoons();

    if (!m_planets.empty())
    {
        m_selected = 0;
        m_planets[m_selected]->chooseRandomFact();
    }
    applySelectionFlags();

    std::cout << "Solar System initialized with " << m_planets.size() << " planets" << std::endl;
}

void SolarSystem::update(float deltaTime)
{
    float dt = m_paused ? 0.0f : (deltaTime * m_timeScale);

    if (m_sun)
        m_sun->update(dt);
    for (auto &planet : m_planets)
        planet->update(dt);
}

void SolarSystem::render(Shader &shader, unsigned int sphereVAO, int vertexCount, const glm::vec3 &cameraPos)
{
    shader.setVec3("viewPos", cameraPos);

    if (m_sun)
        m_sun->render(shader, sphereVAO, vertexCount);

    for (auto &planet : m_planets)
        planet->render(shader, sphereVAO, vertexCount);
}

glm::vec3 SolarSystem::getSunPosition() const
{
    if (m_sun)
        return m_sun->getPosition();
    return glm::vec3(0.0f);
}

void SolarSystem::createPlanets()
{
    auto venus = std::make_shared<Planet>(
        "Venus", 0.7f, "assets/textures/venus.jpg",
        -6.0f, 0.7f, 1.5f,
        std::vector<std::string>{
            "A day on Venus is longer than its year (243 Earth days vs. 225).",
            "Venus rotates retrograde—its Sun rises in the west.",
            "Hottest planet due to runaway greenhouse effect."});
    m_planets.push_back(venus);

    auto earth = std::make_shared<Planet>(
        "Earth", 0.8f, "assets/textures/earth.jpg",
        6.0f, 0.5f, 2.0f,
        std::vector<std::string>{
            "71% of Earth's surface is water.",
            "Only known planet with life (so far).",
            "Magnetic field shields us from solar wind."});
    m_planets.push_back(earth);

    auto mars = std::make_shared<Planet>(
        "Mars", 0.6f, "assets/textures/mars.jpg",
        -10.0f, 0.3f, 1.8f,
        std::vector<std::string>{
            "Olympus Mons is the tallest volcano.",
            "Valles Marineris spans over 4,000 km.",
            "Thin atmosphere; average −60°C."});
    m_planets.push_back(mars);

    auto jupiter = std::make_shared<Planet>(
        "Jupiter", 1.5f, "assets/textures/jupiter.jpg",
        12.0f, 0.2f, 1.2f,
        std::vector<std::string>{
            "More massive than all others combined.",
            "Great Red Spot is centuries-old storm.",
            "10-hour day—fastest rotation."});
    m_planets.push_back(jupiter);
}

void SolarSystem::createMoons()
{
    for (auto &planet : m_planets)
    {
        if (planet->getName() == "Earth")
        {
            auto moon = std::make_shared<Moon>("Moon", 0.2f, "assets/textures/moon.jpg",
                                               planet.get(), 1.5f, 3.0f);
            planet->addMoon(moon);
        }

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

void SolarSystem::increaseTimeScale()
{
    m_timeScale *= 1.5f;
    if (m_timeScale > 64.0f)
        m_timeScale = 64.0f;
}

void SolarSystem::decreaseTimeScale()
{
    m_timeScale /= 1.5f;
    if (m_timeScale < 0.05f)
        m_timeScale = 0.05f;
}

void SolarSystem::cycleSelection(int dir)
{
    if (m_planets.empty())
    {
        m_selected = -1;
        return;
    }
    if (m_selected < 0)
        m_selected = 0;
    m_selected = (int)((m_selected + dir + (int)m_planets.size()) % (int)m_planets.size());
    m_planets[m_selected]->chooseRandomFact();
    applySelectionFlags();
}

void SolarSystem::setSelected(int idx)
{
    if (idx < 0 || idx >= (int)m_planets.size())
        return;
    m_selected = idx;
    m_planets[m_selected]->chooseRandomFact();
    applySelectionFlags();
}

std::string SolarSystem::selectedName() const
{
    if (m_selected < 0 || m_selected >= (int)m_planets.size())
        return "None";
    return m_planets[m_selected]->getName();
}

glm::vec3 SolarSystem::selectedPosition() const
{
    if (m_selected < 0 || m_selected >= (int)m_planets.size())
        return glm::vec3(0.0f);
    return m_planets[m_selected]->getPosition();
}

float SolarSystem::selectedRadius() const
{
    if (m_selected < 0 || m_selected >= (int)m_planets.size())
        return 1.0f;
    return m_planets[m_selected]->getRadius();
}

std::string SolarSystem::selectedFact() const
{
    if (m_selected < 0 || m_selected >= (int)m_planets.size())
        return "";
    return m_planets[m_selected]->currentFact();
}

int SolarSystem::pickPlanet(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, float &tHit) const
{
    int bestIdx = -1;
    float bestT = std::numeric_limits<float>::infinity();
    for (int i = 0; i < (int)m_planets.size(); ++i)
    {
        float t;
        if (m_planets[i]->intersectsRay(rayOrigin, rayDir, t))
        {
            if (t > 0.0f && t < bestT)
            {
                bestT = t;
                bestIdx = i;
            }
        }
    }
    tHit = bestT;
    return bestIdx;
}

void SolarSystem::applySelectionFlags()
{
    for (int i = 0; i < (int)m_planets.size(); ++i)
        m_planets[i]->setSelected(i == m_selected);
}

// ---- NEW helpers ----
int SolarSystem::findPlanetIndex(const std::string &name) const
{
    for (int i = 0; i < (int)m_planets.size(); ++i)
        if (m_planets[i]->getName() == name)
            return i;
    return -1;
}

glm::vec3 SolarSystem::planetPosition(int idx) const
{
    if (idx < 0 || idx >= (int)m_planets.size())
        return glm::vec3(0.0f);
    return m_planets[idx]->getPosition();
}

float SolarSystem::planetRadiusByIndex(int idx) const
{
    if (idx < 0 || idx >= (int)m_planets.size())
        return 1.0f;
    return m_planets[idx]->getRadius();
}
