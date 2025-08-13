#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <limits>

ObjModel::~ObjModel()
{
    if (m_ebo)
        glDeleteBuffers(1, &m_ebo);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
}

static inline std::string trim(const std::string &s)
{
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos)
        return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

void ObjModel::parseFaceToken(const std::string &tok, int &vi, int &ti, int &ni)
{

    vi = ti = ni = 0;
    std::string acc;
    int vals[3] = {0, 0, 0};
    int idx = 0, stage = 0;
    for (char c : tok)
    {
        if (c == '/')
        {
            if (!acc.empty())
            {
                vals[idx++] = std::stoi(acc);
                acc.clear();
            }
            else
            {
                ++idx;
            }
            ++stage;
        }
        else
            acc.push_back(c);
    }
    if (!acc.empty() && idx < 3)
        vals[idx++] = std::stoi(acc);
    vi = vals[0];
    if (stage >= 1)
        ti = vals[1];
    if (stage >= 2)
        ni = vals[2];
}

bool ObjModel::loadFromOBJ(const std::string &path, bool flipTexV)
{
    m_vertices.clear();
    m_indices.clear();
    m_ready = false;

    std::ifstream in(path);
    if (!in.is_open())
    {
        fprintf(stderr, "[ObjModel] Could not open OBJ: %s\n", path.c_str());
        return false;
    }

    std::vector<glm::vec3> pos;
    std::vector<glm::vec2> tex;
    std::vector<glm::vec3> nrm;

    struct Key
    {
        int v, t, n;
        bool operator==(const Key &o) const { return v == o.v && t == o.t && n == o.n; }
    };
    struct KeyHash
    {
        size_t operator()(const Key &k) const { return (size_t)k.v * 73856093u ^ (size_t)k.t * 19349663u ^ (size_t)k.n * 83492791u; }
    };
    std::unordered_map<Key, unsigned, KeyHash> lut;

    std::string line;
    while (std::getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ls(line);
        std::string tag;
        ls >> tag;

        if (tag == "v")
        {
            glm::vec3 p;
            ls >> p.x >> p.y >> p.z;
            pos.push_back(p);
        }
        else if (tag == "vt")
        {
            glm::vec2 t;
            ls >> t.x >> t.y;
            if (flipTexV)
                t.y = 1.0f - t.y;
            tex.push_back(t);
        }
        else if (tag == "vn")
        {
            glm::vec3 n;
            ls >> n.x >> n.y >> n.z;
            nrm.push_back(glm::normalize(n));
        }
        else if (tag == "f")
        {
            std::vector<std::string> toks;
            std::string tok;
            while (ls >> tok)
                toks.push_back(tok);
            if (toks.size() < 3)
                continue;

            auto getIndex = [&](const std::string &t) -> unsigned
            {
                int vi, ti, ni;
                parseFaceToken(t, vi, ti, ni);
                auto fix = [&](int i, int size) -> int
                { if(i>0) return i-1; if(i<0) return size+i; return -1; };
                int v = fix(vi, (int)pos.size());
                int tt = fix(ti, (int)tex.size());
                int nn = fix(ni, (int)nrm.size());
                glm::vec3 P = (v >= 0) ? pos[v] : glm::vec3(0);
                glm::vec2 T = (tt >= 0) ? tex[tt] : glm::vec2(0);
                glm::vec3 N = (nn >= 0) ? nrm[nn] : glm::vec3(0, 1, 0);
                Key key{v, tt, nn};
                auto it = lut.find(key);
                if (it != lut.end())
                    return it->second;
                unsigned idx = (unsigned)m_vertices.size();
                m_vertices.push_back(Vertex{P, N, T});
                lut.emplace(key, idx);
                return idx;
            };

            unsigned i0 = getIndex(toks[0]);
            for (size_t i = 2; i < toks.size(); ++i)
            {
                unsigned i1 = getIndex(toks[i - 1]);
                unsigned i2 = getIndex(toks[i]);
                m_indices.push_back(i0);
                m_indices.push_back(i1);
                m_indices.push_back(i2);
            }
        }
        // ignore materials/groups
    }
    in.close();
    return uploadToGPU();
}

bool ObjModel::uploadToGPU()
{
    if (m_vertices.empty() || m_indices.empty())
    {
        fprintf(stderr, "[ObjModel] Empty geometry; nothing to upload.\n");
        return false;
    }
    if (!m_vao)
        glGenVertexArrays(1, &m_vao);
    if (!m_vbo)
        glGenBuffers(1, &m_vbo);
    if (!m_ebo)
        glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned), m_indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, nrm));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    m_ready = true;
    return true;
}

glm::mat4 ObjModel::modelMatrix() const
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, m_position);
    m = glm::rotate(m, m_rotation.y, glm::vec3(0, 1, 0));
    m = glm::rotate(m, m_rotation.x, glm::vec3(1, 0, 0));
    m = glm::rotate(m, m_rotation.z, glm::vec3(0, 0, 1));
    m = glm::scale(m, m_scale);
    return m;
}

void ObjModel::draw() const
{
    if (!m_ready)
        return;
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
