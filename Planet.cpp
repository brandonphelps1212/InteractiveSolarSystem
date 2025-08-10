#include "Planet.h"
#include "Moon.h"
#include <GL/glew.h>
#include <cmath>
#include <limits>
#include <random>

Planet::Planet(const std::string &name, float radius, const std::string &texturePath,
               float orbitRadius, float orbitSpeed, float rotationSpeed,
               const std::vector<std::string> &facts)
    : CelestialBody(name, radius, texturePath),
      m_orbitRadius(orbitRadius), m_orbitSpeed(orbitSpeed),
      m_orbitAngle(0.0f), m_time(0.0f), m_facts(facts)
{
    m_rotationSpeed = rotationSpeed;
    updateOrbitalPosition();
}

void Planet::update(float deltaTime)
{
    m_time += deltaTime;

    m_orbitAngle += m_orbitSpeed * deltaTime;
    updateOrbitalPosition();

    m_rotation.y = m_time * m_rotationSpeed;

    for (auto &moon : m_moons)
        moon->update(deltaTime);
}

void Planet::render(Shader &shader, unsigned int sphereVAO, int vertexCount)
{
    if (m_texture)
    {
        glActiveTexture(GL_TEXTURE0);
        m_texture->bind();
        shader.setInt("texture1", 0);
    }

    // Selection highlight: slight scale up
    float scaleBoost = m_selected ? 1.15f : 1.0f;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_radius * scaleBoost));
    shader.setMat4("model", model);

    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    for (auto &moon : m_moons)
        moon->render(shader, sphereVAO, vertexCount);
}

glm::mat4 Planet::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_radius));
    return model;
}

void Planet::addMoon(std::shared_ptr<Moon> moon)
{
    m_moons.push_back(moon);
}

void Planet::updateOrbitalPosition()
{
    m_position.x = m_orbitRadius * cos(m_orbitAngle);
    m_position.z = m_orbitRadius * sin(m_orbitAngle);
    m_position.y = 0.0f;
}

std::string Planet::chooseRandomFact()
{
    if (m_facts.empty())
    {
        static const std::string empty = "";
        return empty;
    }

    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, (int)m_facts.size() - 1);

    int idx = dist(rng);
    if (m_facts.size() > 1 && idx == m_lastFactIndex)
    {
        // simple no-repeat: pick a neighbor
        idx = (idx + 1) % (int)m_facts.size();
    }

    m_lastFactIndex = idx;
    return m_facts[idx];
}

const std::string &Planet::currentFact() const
{
    if (m_facts.empty())
    {
        static const std::string empty = "";
        return empty;
    }
    if (m_lastFactIndex < 0 || m_lastFactIndex >= (int)m_facts.size())
        return m_facts.front();
    return m_facts[m_lastFactIndex];
}

// Ray–sphere intersection against this planet's bounding sphere
bool Planet::intersectsRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, float &tOut) const
{
    glm::vec3 oc = rayOrigin - m_position;
    float a = glm::dot(rayDir, rayDir); // should be 1 if dir normalized
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - (m_radius * m_radius);

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0f)
        return false;

    float sqrtD = sqrtf(discriminant);
    float t1 = (-b - sqrtD) / (2.0f * a);
    float t2 = (-b + sqrtD) / (2.0f * a);

    float t = std::numeric_limits<float>::infinity();
    if (t1 > 0.0f)
        t = t1;
    if (t2 > 0.0f && t2 < t)
        t = t2;

    if (std::isinf(t))
        return false;
    tOut = t;
    return true;
}
