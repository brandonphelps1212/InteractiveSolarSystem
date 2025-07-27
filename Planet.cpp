#include "Planet.h"
#include "Moon.h"
#include <GL/glew.h>
#include <cmath>

Planet::Planet(const std::string &name, float radius, const std::string &texturePath,
               float orbitRadius, float orbitSpeed, float rotationSpeed)
    : CelestialBody(name, radius, texturePath),
      m_orbitRadius(orbitRadius), m_orbitSpeed(orbitSpeed),
      m_orbitAngle(0.0f), m_time(0.0f)
{
    m_rotationSpeed = rotationSpeed;
    updateOrbitalPosition();
}

void Planet::update(float deltaTime)
{
    m_time += deltaTime;

    // Update orbital position around the sun
    m_orbitAngle += m_orbitSpeed * deltaTime;
    updateOrbitalPosition();

    // Update rotation on own axis
    m_rotation.y = m_time * m_rotationSpeed;

    // Update all moons
    for (auto &moon : m_moons)
    {
        moon->update(deltaTime);
    }
}

void Planet::render(Shader &shader, unsigned int sphereVAO, int vertexCount)
{
    if (m_texture)
    {
        glActiveTexture(GL_TEXTURE0);
        m_texture->bind();
        shader.setInt("texture1", 0);
    }

    // Set model matrix
    shader.setMat4("model", getModelMatrix());

    // Render the planet
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Render all moons
    for (auto &moon : m_moons)
    {
        moon->render(shader, sphereVAO, vertexCount);
    }
}

glm::mat4 Planet::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);

    // Apply transformations: translate to orbital position, then rotate and scale
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
    // Calculate position in orbit around the sun (at origin)
    m_position.x = m_orbitRadius * cos(m_orbitAngle);
    m_position.z = m_orbitRadius * sin(m_orbitAngle);
    m_position.y = 0.0f; // Keep planets in the same plane for now
}