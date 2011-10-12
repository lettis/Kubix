/*  
    Kubix - 3D OpenGL implementation of the board game "Tactix"/"Duell" with AI
    Copyright (C) 2011  Florian Sittel & Carsten Burgard

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TOOLS__HPP
#define TOOLS__HPP

#include <string>
#include <map>
#include <vector>

#include "SDL_opengl.h"

#include "gui.hpp"


template<class NumType> int sgn(NumType n);
int sgn(float f);
int sgn(int i);

void loadTextures();

void checkGLError();
std::string stringprintf(std::string str, ...);

/// represents a simple logger class, use it instead of cout/cerr-statements!
class KBX_Logger{
    std::string  _name;
    std::ostream& _out;
    std::ostream& _err;
    static bool  _loggingEnabled;
    static std::string _getTime();
    void _sendMessage(std::string category, std::string msg);
public:
    KBX_Logger(std::string name);
    KBX_Logger(std::string name, std::ostream out, std::ostream err);
    static void enableLogging();
    static void disableLogging();
    void info(std::string msg);
    void warning(std::string msg);
    void error(std::string msg);
};

/// handles the textures for OpenGL
class TextureHandler{
    GLuint* textures;
    /// point external keys to internal index of textures array
    std::map<size_t, size_t> keys;
    void loadTexture(const char* filename, GLuint textureId );
public: 
    void load(std::map<size_t, std::string> files);
    GLuint get(size_t key);
};
#endif
