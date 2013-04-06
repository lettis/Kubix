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
#include <math.h>

#include <QtOpenGL/QGLWidget>

namespace KBX {
  template<class NumType> int sgn(NumType n);
  int sgn(float f);
  int sgn(int i);
  
  void swap(int& a, int& b);

  void loadTextures();
  
  void checkGLError();
  std::string stringprintf(std::string str, ...);
  
  /// represents a simple logger class, use it instead of cout/cerr-statements!
  class Logger{
      std::string  _name;
      static std::ostream* _out;
      static std::ostream* _err;
      static std::vector<std::string> _filters;
      static bool  _infosEnabled;
      static bool  _warningsEnabled;
      static bool  _errorsEnabled;
      static std::string _getTime();
      void _sendMessage(std::string category, std::string msg);
  public:
      Logger(std::string name);
      static void setOut(std::ostream* out);
      static void setErr(std::ostream* err);
      static void enableInfos();
      static void disableInfos();
      static void enableWarnings();
      static void disableWarnings();
      static void enableErrors();
      static void disableErrors();
      static void filter(std::string name);
      void info(std::string msg);
      void warning(std::string msg);
      void error(std::string msg);
  };
 
   /// defines a simple vector in 3d-cartesian coordinates
  class Vec{
  public:
      float x;
      float y;
      float z;
      Vec();
      Vec(float x, float y, float z);
      float norm();
      Vec normalize();
      Vec scale(float a);
      Vec add(Vec v);
      Vec sub(Vec v);
      Vec rotate(Vec rotAxis, float angle);
      Vec cross(Vec v);
  };
  
  /// defines a color
  class Color{
  public:
      // define some colors fixed
      static const Color BLACK;
      static const Color WHITE;
      static const Color GREY10;
      static const Color GREY20;
      static const Color GREY30;
      static const Color GREY40;
      static const Color GREY50;
      static const Color GREY60;
      static const Color GREY70;
      static const Color GREY80;
      static const Color GREY90;
      static const Color RED;
      static const Color GREEN;
      static const Color BLUE;
      float r;
      float g;
      float b;
      Color();
      Color(size_t id);
      Color(unsigned char r, unsigned char g, unsigned char b);
      Color(int r, int g, int b);
      Color(float r, float g, float b);
      size_t id() const;
      void glColor() const;
  };
 

} // end namespace KBX
#endif
