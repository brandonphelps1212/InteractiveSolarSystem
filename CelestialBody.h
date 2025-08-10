#ifndef CELESTIALBODY_H
#define CELESTIALBODY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "Shader.h"
#include "Texture.h"

class CelestialBody
{
public:
    // Constructor
    CelestialBody(const std::string &name, float radius, const std::string &texturePath);
    virtual ~CelestialBody();

    // Pure virtual functions - must be implemented by derived classes
    virtual void update(float deltaTime) = 0;
    virtual void render(Shader &shader, unsigned int sphereVAO, int vertexCount) = 0;
    virtual glm::mat4 getModelMatrix() const = 0;

    // Getters
    std::string getName() const { return m_name; }
    float getRadius() const { return m_radius; }
    glm::vec3 getPosition() const { return m_position; }

    // Setters
    void setPosition(const glm::vec3 &position) { m_position = position; }

protected:
    std::string m_name;
    float m_radius;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    float m_rotationSpeed;
    Texture *m_texture;
};

#endif
