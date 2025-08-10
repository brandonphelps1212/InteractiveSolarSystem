#include "Moon.h"
#include "Planet.h"
#include <GL/glew.h>
#include <cmath>

Moon::Moon(const std::string &name, float radius, const std::string &texturePath,
           Planet *parentPlanet, float orbitRadius, float orbitSpeed)
    : CelestialBody(name, radius, texturePath),
      m_parentPlanet(parentPlanet), m_orbitRadius(orbitRadius),
      m_orbitSpeed(orbitSpeed), m_orbitAngle(0.0f), m_time(0.0f)
{
    m_rotationSpeed = orbitSpeed * 0.8f; // Fixed: Added missing '*' operator
    updateOrbitalPosition();
}

void Moon::update(float deltaTime)
{
    m_time += deltaTime;

    // Update orbital position around parent planet
    m_orbitAngle += m_orbitSpeed * deltaTime;
    updateOrbitalPosition();

    // Update rotation on own axis
    m_rotation.y = m_time * m_rotationSpeed;
}

void Moon::render(Shader &shader, unsigned int sphereVAO, int vertexCount)
{
    if (m_texture)
    {
        glActiveTexture(GL_TEXTURE0);
        m_texture->bind();
        shader.setInt("texture1", 0);
    }

    // Set model matrix
    shader.setMat4("model", getModelMatrix());

    // Render the moon
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

glm::mat4 Moon::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);

    // Apply transformations
    model = glm::translate(model, m_position);
    model = glm::rotate(model, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_radius));

    return model;
}

void Moon::updateOrbitalPosition()
{
    if (m_parentPlanet)
    {
        // Get parent planet's position
        glm::vec3 planetPos = m_parentPlanet->getPosition();

        // Calculate position in orbit around parent planet
        m_position.x = planetPos.x + m_orbitRadius * cos(m_orbitAngle);
        m_position.z = planetPos.z + m_orbitRadius * sin(m_orbitAngle);
        m_position.y = planetPos.y; // Keep moons in same plane as planet for now
    }
}