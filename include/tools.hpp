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


class TextureHandler;
class KBX_ObjectHandler;

template<class NumType> int sgn(NumType n);
int sgn(float f);
int sgn(int i);

void checkGLError();
std::string stringprintf(std::string str, ...);

/// represents a simple logger class, use it instead of cout-statements!
class KBX_Logger{
    std::ostream _out;
    std::ostream _err;
    string       _name;
    static bool  _loggingEnabled;
    static string _getTime();
    void _sendMessage(string category, string msg);
public:
    KBX_Logger(string name);
    KBX_Logger(string name, std::ostream out, std::ostream err);
    static void enableLogging();
    static void disableLogging();
    void info(string msg);
    void warning(string msg);
    void error(string msg);
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

/// handles all KBX_Objects (mainly for mouse selection) 
class KBX_ObjectHandler{
    std::vector<KBX_Object*> objects;
public: 
    // the nullId is the id that corresponds to the NULL pointer
    // it is, however, not neccessarily zero
    // as it is supposed to correspond to the color id 
    // of the background color
    size_t nullId;
    KBX_ObjectHandler();
    KBX_Object* get(size_t id);
    size_t add(KBX_Object* obj);
    void remove(KBX_Object* obj);
    void remove(size_t id);
};
#endif
