#include "Skybox.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Skybox::Skybox(const std::string &texturePath)
{
    try
    {
        m_texture = new Texture(texturePath.c_str());
        std::cout << "Loaded skybox texture: " << texturePath << std::endl;
    }
    catch (...)
    {
        std::cerr << "Failed to load skybox texture: " << texturePath << std::endl;
        m_texture = nullptr;
    }
}

Skybox::~Skybox()
{
    if (m_texture)
    {
        delete m_texture;
    }
}

void Skybox::render(Shader &shader, unsigned int sphereVAO, int vertexCount, const glm::vec3 &cameraPosition)
{
    if (!m_texture)
        return;

    // Disable depth writing so skybox is always drawn behind everything
    glDepthMask(GL_FALSE);

    // Bind skybox texture
    glActiveTexture(GL_TEXTURE0);
    m_texture->bind();
    shader.setInt("texture1", 0);

    // Create a huge sphere centered on the camera
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cameraPosition); // Follow the camera
    model = glm::scale(model, glm::vec3(100.0f));  // Make it huge

    // Flip the sphere inside-out so we see the texture from the inside
    model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));

    shader.setMat4("model", model);

    // Set ambient lighting to full brightness for stars
    shader.setVec3("lightPos", glm::vec3(0.0f)); // No directional lighting

    // Render the skybox sphere
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Re-enable depth writing
    glDepthMask(GL_TRUE);
}