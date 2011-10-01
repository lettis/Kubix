#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>

#include "tools.hpp"

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

}

/// get an individual texture from the list
/**
   \param key key of the desired texture (same as key to filename specified in TextureHandler::load)
   \return GL handle to texture
 */
GLuint TextureHandler::get(size_t key){
    GLuint texture;

    return texture;
}
