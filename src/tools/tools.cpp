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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "gui.hpp"
#include "tools.hpp"

#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_image.h"

int sgn(float f){
    if( f < 0 ){
        return -1;
    }
    return 1;
}
int sgn(int i){
    if( i < 0 ){
        return -1;
    }
    return 1;
}

/// write OpenGL error codes to stderr
void checkGLError() {
    GLuint err = glGetError();
    if (err != GL_NO_ERROR){
        std::cerr << std::hex << err << std::endl;
    }
}

/// behaves like sprintf(char*, ...), but with c++ strings and returns the result
/**
   \param str pattern to be printed to
   \return resulting string
   The function interally calles sprintf, but converts the result to a c++ string and returns that one. 
   Problems of memory allocation are taken care of automatically.
 */
std::string stringprintf(std::string str, ...){
    unsigned int size = 256;
    va_list args;
    char* buf = (char*)malloc(size*sizeof(char));
    va_start (args, str);
    while(size <= (unsigned int)vsnprintf(buf, size, str.c_str(), args)){
        size *= 2;
        buf = (char*)realloc(buf, size*sizeof(char));
    }
    va_end (args);      
    return std::string(buf);
}  

/// load textures from the file list specified
/**
   \param files map containing graphic files of the textures and the keys to reference them
 */
void TextureHandler::load(std::map<size_t, std::string> files){
    // Have OpenGL generate a texture object handle for us
    this->textures = (GLuint*)malloc(files.size() * sizeof(GLuint));
    glGenTextures( files.size(), this->textures );
    size_t i=0;
    for(std::map<size_t, std::string>::iterator it = files.begin(); it != files.end(); it++){
        loadTexture(it->second.c_str(), this->textures[i]);
        this->keys.insert(std::pair<size_t, size_t>(it->first, i));
        i++;
    }
}

/// get object with given id
/**
   \param id id of the desired object
   \return KBX_Object with given id
*/
KBX_Object* KBX_ObjectHandler::get(size_t id){
    if ( id == this->nullId ){
        return NULL;
    } else if(id > this->objects.size()){
        throw stringprintf("cannot access object %d: not in list", int(id)).c_str();
    } else return this->objects.at(id);
}

/// construct a new object handler
KBX_ObjectHandler::KBX_ObjectHandler(){
    this->nullId = 0;
}

/// add an object to the handler
/**
   \param obj object to add to the handler
   \return number of object in list (object id)
*/
size_t KBX_ObjectHandler::add(KBX_Object* obj){
    if(this->objects.size() == nullId){
        this->objects.push_back(NULL);
    }
    this->objects.push_back(obj);
    return this->objects.size()-1;
}

/// remove an object from the handler
/**
   \param obj object to remove from the handler
 */
void KBX_ObjectHandler::remove(KBX_Object* obj){
    for(size_t i=0; i<this->objects.size(); i++){
        if(objects[i] == obj) objects[i] = NULL;
    }
}

/// remove an object from the handler
/**
   \param id id of object to remove from the handler
 */
void KBX_ObjectHandler::remove(size_t id){
    if(id < objects.size()) objects[id] = NULL;
}

void TextureHandler::loadTexture(const char* filename, GLuint textureId ){
    // This surface will tell us the details of the image
    SDL_Surface *surface;	
    GLenum textureFormat;
    
    if ( (surface = IMG_Load(filename)) ) { 
        // Check that the image's width is a power of 2
        if ( (surface->w & (surface->w - 1)) != 0 ) {
            throw stringprintf("warning: image width of %s is not a power of 2", filename).c_str();
        }
        // Also check if the height is a power of 2
        if ( (surface->h & (surface->h - 1)) != 0 ) {
            throw stringprintf("warning: image height of %s is not a power of 2", filename).c_str();
        }
        // get the number of channels in the SDL surface
        GLint nColors = surface->format->BytesPerPixel;
        // contains an alpha channel
        if (nColors == 4){
            if (surface->format->Rmask == 0x000000ff){
                textureFormat = GL_RGBA;
            } else {
                textureFormat = GL_BGRA;
            }
        } else if (nColors == 3){
            // no alpha channel
            if (surface->format->Rmask == 0x000000ff){
                textureFormat = GL_RGB;
            } else {
                textureFormat = GL_BGR;
            }
        } else {
            throw stringprintf("warning: %s is not truecolor..  this will probably break", filename).c_str();
        }
        // Bind the texture object
        glBindTexture( GL_TEXTURE_2D, textureId );
        // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        // Edit the texture object's image data using the information SDL_Surface gives us
        glTexImage2D( GL_TEXTURE_2D, 0, nColors, surface->w, surface->h, 0,
                      textureFormat, GL_UNSIGNED_BYTE, surface->pixels );
    } 
    // Free the SDL_Surface only if it was successfully created
    if ( surface ) { 
        SDL_FreeSurface( surface );
    }
}
/// get an individual texture from the list
/**
   \param key key of the desired texture (same as key to filename specified in TextureHandler::load)
   \return GL handle to texture
 */
GLuint TextureHandler::get(size_t key){
    // TODO: throw exception if key not in this->keys
    return this->textures[this->keys[key]];
}

