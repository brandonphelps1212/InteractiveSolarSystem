#include "Sun.h"
#include <GL/glew.h>

Sun::Sun(const std::string &texturePath)
    : CelestialBody("Sun", 2.0f, texturePath), m_time(0.0f)
{
    m_position = glm::vec3(0.0f, 0.0f, 0.0f); // Sun at center
    m_rotationSpeed = 0.2f;                   // Slow rotation
}

void Sun::update(float deltaTime)
{
    m_time += deltaTime;

    // Sun rotates on its own axis
    m_rotation.y = m_time * m_rotationSpeed;
}

void Sun::render(Shader &shader, unsigned int sphereVAO, int vertexCount)
{
    if (m_texture)
    {
        glActiveTexture(GL_TEXTURE0);
        m_texture->bind();
        shader.setInt("texture1", 0);
    }

    // Set model matrix
    shader.setMat4("model", getModelMatrix());

    // Sun is the light source
    shader.setVec3("lightPos", m_position);

    // Render the sphere
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

glm::mat4 Sun::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);

    // Apply transformations
    model = glm::translate(model, m_position);
    model = glm::rotate(model, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_radius));

    return model;
}