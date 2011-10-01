#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>

#include "tools.hpp"

#include "SDL.h"
#include "SDL_opengl.h"

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

void TextureHandler::loadTexture(const char* filename, GLuint textureId ){
    // This surface will tell us the details of the image
    SDL_Surface *surface;	
    GLenum textureFormat;
    
    if ( (surface = SDL_LoadBMP(filename)) ) { 
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
