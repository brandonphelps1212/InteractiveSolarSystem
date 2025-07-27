#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <GL/glew.h>

class Texture {
public:
    GLuint ID;

    Texture(const std::string& path);
    void bind() const;
};

#endif

