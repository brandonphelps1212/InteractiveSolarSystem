#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

// Minimal OBJ model: positions, normals, texcoords, indexed.
class ObjModel
{
public:
    ObjModel() = default;
    explicit ObjModel(const std::string &path) { loadFromOBJ(path); }
    ~ObjModel();

    bool loadFromOBJ(const std::string &path, bool flipTexV = true);

    // GPU
    bool uploadToGPU();
    void draw() const;

    // Transform
    void setPosition(const glm::vec3 &p) { m_position = p; }
    void setRotationEuler(const glm::vec3 &eulerRadians) { m_rotation = eulerRadians; }
    void setScale(float s) { m_scale = glm::vec3(s); }
    void setScale(const glm::vec3 &s) { m_scale = s; }
    glm::mat4 modelMatrix() const;

    bool isReady() const { return m_ready; }

private:
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 nrm;
        glm::vec2 uv;
    };

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    bool m_ready = false;

    glm::vec3 m_position{0.0f};
    glm::vec3 m_rotation{0.0f}; // radians (x,y,z)
    glm::vec3 m_scale{1.0f};

    static void parseFaceToken(const std::string &tok, int &vi, int &ti, int &ni);
};

#endif
