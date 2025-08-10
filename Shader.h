#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>

class Shader
{
public:
    // Program ID
    unsigned int ID;

    // Constructor: build shader from file paths
    Shader(const char *vertexPath, const char *fragmentPath);

    // Activate the shader
    void use() const;

    // Utility functions to set uniforms
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
};

#endif
