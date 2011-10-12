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

template<class NumType>
int sgn(NumType n){
    if( n < 0 ){
        return -1;
    }
    return 1;
}

int sgn(float f){
    return sgn<float> (f);
}
int sgn(int i){
    return sgn<int> (i);
}


/// initialize logger and give it a name
/**
    \param name logger will print this name in the logs
    errors will be printed to stderr,
    warnings and infos will be printed to stdout.
*/
KBX_Logger::KBX_Logger(std::string name) :
     _name(name)
{}
// set out stream and error stream to stdout and stderr
std::ostream* KBX_Logger::_out = &std::cout;
std::ostream* KBX_Logger::_err = &std::cerr;
void KBX_Logger::setOut(std::ostream* out){
    KBX_Logger::_out = out;
}
void KBX_Logger::setErr(std::ostream* err){
    KBX_Logger::_err = err;
}

// logging will be disabled per default
bool KBX_Logger::_infosEnabled      = false;
bool KBX_Logger::_warningsEnabled   = false;
bool KBX_Logger::_errorsEnabled     = false;
/// use this method to enable info logging
void KBX_Logger::enableInfos(){
    KBX_Logger::_infosEnabled = true;
}
/// use this method to disable info logging
void KBX_Logger::disableInfos(){
    KBX_Logger::_infosEnabled = false;
}
/// use this method to enable warning logging
void KBX_Logger::enableWarnings(){
    KBX_Logger::_warningsEnabled = true;
}
/// use this method to disable warning logging
void KBX_Logger::disableWarnings(){
    KBX_Logger::_warningsEnabled = false;
}
/// use this method to enable error logging
void KBX_Logger::enableErrors(){
    KBX_Logger::_errorsEnabled = true;
}
/// use this method to disable error logging
void KBX_Logger::disableErrors(){
    KBX_Logger::_errorsEnabled = false;
}
/// return local time as string
std::string KBX_Logger::_getTime(){
    time_t rawTime;
    struct tm* timeInfo;
    // get current time (raw)
    time(&rawTime);
    // convert to local time
    timeInfo = localtime(&rawTime);
    return std::string( asctime(timeInfo) );
}
/// write message to logfile / stdout
void KBX_Logger::_sendMessage(std::string category, std::string msg){
    (*this->_out) << this->_getTime() << "\t" << this->_name << " | " << category << ": " << msg << std::endl;
}
/// write info to logfile / stdout
void KBX_Logger::info(std::string msg){
    if ( this->_infosEnabled ){
        this->_sendMessage("INFO", msg);
    }
}
/// write warning to logfile / stdout
void KBX_Logger::warning(std::string msg){
    if ( this->_warningsEnabled ){
        this->_sendMessage("WARNING", msg);
    }
}
/// write error to logfile / stdout
void KBX_Logger::error(std::string msg){
    if ( this->_errorsEnabled ){
        this->_sendMessage("ERROR", msg);
    }
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

void loadTextures(){
    // declare filename-key combinations for die faces
    std::map<size_t, std::string> dieFaces;
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_K_W,  "./res/sidek.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_1_W,  "./res/side1.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_2_W,  "./res/side2.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_3_W,  "./res/side3.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_4_W,  "./res/side4.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_5_W,  "./res/side5.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_6_W,  "./res/side6.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_K_B, "./res/sidekb.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_1_B, "./res/side1b.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_2_B, "./res/side2b.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_3_B, "./res/side3b.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_4_B, "./res/side4b.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_5_B, "./res/side5b.png")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_6_B, "./res/side6b.png")  );
    // initialize (load) die face textures
    KBX_Die::textures.load(dieFaces);
}
