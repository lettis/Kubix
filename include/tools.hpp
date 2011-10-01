#ifndef TOOLS__HPP
#define TOOLS__HPP

#include <string>
#include <map>

#include "SDL_opengl.h"

void checkGLError();

class TextureHandler{
    GLuint* textures;
    /// point external keys to internal index of textures array
    std::map<size_t, size_t> keys;
    void loadTexture(const char* filename, GLuint textureId );
public: 
    void load(std::map<size_t, std::string> files);
    GLuint get(size_t key);
};


std::string stringprintf(std::string str, ...);

#endif
