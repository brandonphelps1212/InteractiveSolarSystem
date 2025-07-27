#include "CelestialBody.h"
#include <iostream>

CelestialBody::CelestialBody(const std::string &name, float radius, const std::string &texturePath)
    : m_name(name), m_radius(radius), m_position(0.0f), m_rotation(0.0f), m_rotationSpeed(1.0f)
{
    try
    {
        m_texture = new Texture(texturePath.c_str());
        std::cout << "Loaded texture for " << m_name << ": " << texturePath << std::endl;
    }
    catch (...)
    {
        std::cerr << "Failed to load texture for " << m_name << ": " << texturePath << std::endl;
        m_texture = nullptr;
    }
}

CelestialBody::~CelestialBody()
{
    if (m_texture)
    {
        delete m_texture;
    }
}