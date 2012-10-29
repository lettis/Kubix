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

//TODO: check if all these inputs are really needed

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <QString>
#include <QPixmap>
#include <QtOpenGL/QGLWidget>

#include "models.hpp"
#include "tools.hpp"


namespace KBX {
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
  Logger::Logger(std::string name)
    : _name(name)
  {}
  // set out stream and error stream to stdout and stderr
  std::ostream* Logger::_out = &std::cout;
  std::ostream* Logger::_err = &std::cerr;
  void Logger::setOut(std::ostream* out){
    Logger::_out = out;
  }
  void Logger::setErr(std::ostream* err){
    Logger::_err = err;
  }
  
  // logging will be disabled per default
  bool Logger::_infosEnabled      = false;
  bool Logger::_warningsEnabled   = false;
  bool Logger::_errorsEnabled     = false;
  /// use this method to enable info logging
  void Logger::enableInfos(){
    Logger::_infosEnabled = true;
  }
  /// use this method to disable info logging
  void Logger::disableInfos(){
    Logger::_infosEnabled = false;
  }
  /// use this method to enable warning logging
  void Logger::enableWarnings(){
    Logger::_warningsEnabled = true;
  }
  /// use this method to disable warning logging
  void Logger::disableWarnings(){
    Logger::_warningsEnabled = false;
  }
  /// use this method to enable error logging
  void Logger::enableErrors(){
    Logger::_errorsEnabled = true;
  }
  /// use this method to disable error logging
  void Logger::disableErrors(){
    Logger::_errorsEnabled = false;
  }
  /// return local time as string
  std::string Logger::_getTime(){
    time_t rawTime;
    struct tm* timeInfo;
    // get current time (raw)
    time(&rawTime);
    // convert to local time
    timeInfo = localtime(&rawTime);
    return std::string( asctime(timeInfo) );
  }
  std::vector<std::string>  Logger::_filters;
  /// show only messages with given name-tag(s)
  void Logger::filter(std::string name){
    Logger::_filters.push_back( name );
  }
  /// write message to logfile / stdout
  void Logger::_sendMessage(std::string category, std::string msg){
    bool printIt = true;
    if (!Logger::_filters.empty()){
      printIt = false;
      for (size_t i=0; i < Logger::_filters.size(); i++){
        if (this->_name == Logger::_filters[i]){
          printIt = true;
          break;
        }
      }
    }
    if (printIt){
      (*this->_out) << this->_getTime() << "\t" << this->_name;
      (*this->_out) << " | " << category << ": " << msg << std::endl;
    }
  }
  /// write info to logfile / stdout
  void Logger::info(std::string msg){
    if ( this->_infosEnabled ){
      this->_sendMessage("INFO", msg);
    }
  }
  /// write warning to logfile / stdout
  void Logger::warning(std::string msg){
    if ( this->_warningsEnabled ){
      this->_sendMessage("WARNING", msg);
    }
  }
  /// write error to logfile / stdout
  void Logger::error(std::string msg){
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
} // end namespace
