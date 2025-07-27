#ifndef SKYBOX_H
#define SKYBOX_H

#include "Texture.h"
#include "Shader.h"
#include <GL/glew.h>

class Skybox
{
public:
    Skybox(const std::string &texturePath);
    ~Skybox();

    void render(Shader &shader, unsigned int sphereVAO, int vertexCount, const glm::vec3 &cameraPosition);

private:
    Texture *m_texture;
};

#endif