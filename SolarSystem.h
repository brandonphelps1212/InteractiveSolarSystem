#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Sun.h"
#include "Planet.h"
#include "Moon.h"
#include "Shader.h"

class SolarSystem
{
public:
    SolarSystem();
    ~SolarSystem();

    void initialize();

    // Time & update
    void update(float deltaTime);
    void setPaused(bool p) { return (void)(m_paused = p); }
    bool isPaused() const { return m_paused; }
    void increaseTimeScale();
    void decreaseTimeScale();
    float timeScale() const { return m_timeScale; }

    // Selection / focus
    void cycleSelection(int dir); // dir = +1 next, -1 prev
    void setSelected(int idx);
    int selectedIndex() const { return m_selected; }
    std::string selectedName() const;
    glm::vec3 selectedPosition() const;
    float selectedRadius() const;
    std::string selectedFact() const;

    // Rendering
    void render(Shader &shader, unsigned int sphereVAO, int vertexCount, const glm::vec3 &cameraPos);

    // Picking (ray from origin along dir). Returns index or -1. tHit is distance along the ray.
    int pickPlanet(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, float &tHit) const;

    // For lighting, etc.
    glm::vec3 getSunPosition() const;

    // ---- NEW: small helpers to fetch planet info by name or index ----
    int findPlanetIndex(const std::string &name) const;
    glm::vec3 planetPosition(int idx) const;
    float planetRadiusByIndex(int idx) const;

private:
    std::unique_ptr<Sun> m_sun;
    std::vector<std::shared_ptr<Planet>> m_planets;

    // Interactive state
    bool m_paused = false;
    float m_timeScale = 1.0f;
    int m_selected = -1;

    void createPlanets();
    void createMoons();
    void applySelectionFlags();
};

#endif
