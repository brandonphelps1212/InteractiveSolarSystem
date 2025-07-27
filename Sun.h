#ifndef SUN_H
#define SUN_H

#include "CelestialBody.h"

class Sun : public CelestialBody
{
public:
    Sun(const std::string &texturePath = "assets/textures/sun.jpg");

    void update(float deltaTime) override;
    void render(Shader &shader, unsigned int sphereVAO, int vertexCount) override;
    glm::mat4 getModelMatrix() const override;

private:
    float m_time;
};

#endif