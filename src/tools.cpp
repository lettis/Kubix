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
/// return sign of number (-1, 0 or 1)
/**
 \returns  1, if sign of x is positive
 \returns -1, if sign of x is negative
 \returns  0, if x is zero
 */
template< class NumType >
int sgn(NumType n) {
  if (n < 0) {
    return -1;
  } else if (n == 0) {
    return 0;
  } else {
    return 1;
  }
}
int sgn(float f) {
  return sgn< float >(f);
}
int sgn(int i) {
  return sgn< int >(i);
}

int sgnP(float f){
  if (f == 0){
    return 1;
  } else {
    return sgn(f);
  }
}


/// swap values of integers a & b
void swap(int& a, int& b) {
  int buf = a;
  a = b;
  b = buf;
}

/// initialize logger and give it a name
/**
 \param name logger will print this name in the logs
 errors will be printed to stderr,
 warnings and infos will be printed to stdout.
 */
Logger::Logger(std::string name)
    : _name(name) {
}
// set out stream and error stream to stdout and stderr
std::ostream* Logger::_out = &std::cout;
std::ostream* Logger::_err = &std::cerr;
void Logger::setOut(std::ostream* out) {
  Logger::_out = out;
}
void Logger::setErr(std::ostream* err) {
  Logger::_err = err;
}

// logging will be disabled per default
bool Logger::_infosEnabled = false;
bool Logger::_warningsEnabled = false;
bool Logger::_errorsEnabled = false;
/// use this method to enable info logging
void Logger::enableInfos() {
  Logger::_infosEnabled = true;
}
/// use this method to disable info logging
void Logger::disableInfos() {
  Logger::_infosEnabled = false;
}
/// use this method to enable warning logging
void Logger::enableWarnings() {
  Logger::_warningsEnabled = true;
}
/// use this method to disable warning logging
void Logger::disableWarnings() {
  Logger::_warningsEnabled = false;
}
/// use this method to enable error logging
void Logger::enableErrors() {
  Logger::_errorsEnabled = true;
}
/// use this method to disable error logging
void Logger::disableErrors() {
  Logger::_errorsEnabled = false;
}
/// return local time as string
std::string Logger::_getTime() {
  time_t rawTime;
  struct tm* timeInfo;
  // get current time (raw)
  time( &rawTime);
  // convert to local time
  timeInfo = localtime( &rawTime);
  return std::string(asctime(timeInfo));
}
std::vector< std::string > Logger::_filters;
/// show only messages with given name-tag(s)
void Logger::filter(std::string name) {
  Logger::_filters.push_back(name);
}
/// write message to logfile / stdout
void Logger::_sendMessage(std::string category, std::string msg) {
  bool printIt = true;
  if ( !Logger::_filters.empty()) {
    printIt = false;
    for (size_t i = 0; i < Logger::_filters.size(); i++) {
      if (this->_name == Logger::_filters[i]) {
        printIt = true;
        break;
      }
    }
  }
  if (printIt) {
    ( *this->_out) << this->_getTime() << "\t" << this->_name;
    ( *this->_out) << " | " << category << ": " << msg << std::endl;
  }
}
/// write info to logfile / stdout
void Logger::info(std::string msg) {
  if (this->_infosEnabled) {
    this->_sendMessage("INFO", msg);
  }
}
/// write warning to logfile / stdout
void Logger::warning(std::string msg) {
  if (this->_warningsEnabled) {
    this->_sendMessage("WARNING", msg);
  }
}
/// write error to logfile / stdout
void Logger::error(std::string msg) {
  if (this->_errorsEnabled) {
    this->_sendMessage("ERROR", msg);
  }
}

/// write OpenGL error codes to stderr
void checkGLError() {
  GLuint err = glGetError();
  if (err != GL_NO_ERROR) {
    std::cerr << std::hex << err << std::endl;
  }
}

/// behaves like sprintf(char*, ...), but with c++ strings and returns the result
/**
 \param str pattern to be printed to
 \return resulting string
 The function internally calls sprintf, but converts the result to a c++ string and returns that one.
 Problems of memory allocation are taken care of automatically.
 */
std::string stringprintf(std::string str, ...) {
  unsigned int size = 256;
  va_list args;
  char* buf = (char*) malloc(size * sizeof(char));
  va_start(args, str);
  while (size <= (unsigned int) vsnprintf(buf, size, str.c_str(), args)) {
    size *= 2;
    buf = (char*) realloc(buf, size * sizeof(char));
  }
  va_end(args);
  return std::string(buf);
}

// define different versions of the Color constructor depending on parameters
Color::Color() {
  this->r = 0;
  this->g = 0;
  this->b = 0;
}
Color::Color(size_t id) {
  this->r = float(id % 255) / 255;
  this->g = float((id / 255) % 255) / 255;
  this->b = float((id / (255 * 255)) % 255) / 255;
}
Color::Color(unsigned char r, unsigned char g, unsigned char b) {
  this->r = float(r) / 255;
  this->g = float(g) / 255;
  this->b = float(b) / 255;
}
Color::Color(int r, int g, int b) {
  this->r = float(r) / 255;
  this->g = float(g) / 255;
  this->b = float(b) / 255;
}
Color::Color(float r, float g, float b) {
  this->r = r;
  this->g = g;
  this->b = b;
}
size_t Color::id() const {
  // this implementation of a (distinct) color id can
  // lead to pretty large numbers. it is used for the
  // object picking mechanism. every object gets an own color
  // to distinguish it from the others when a click happens.
  // therefore, the size of the id-value depends highly
  // on the number of objects.
  // additionally, the picking-mechanism starts by
  // assigning all shades of red before going over to the
  // other numbers. we therefore don't expect much trouble here...
  return this->r * 255 + 255 * 255 * this->g + 255 * 255 * 255 * this->b;
}
/// call glColor3f with internal values
void Color::setAsGlColor() const {
  glColor3f(this->r, this->g, this->b);
}

/// constructor initializing null vector
Vec::Vec()
    : x(0),
      y(0),
      z(0) {
}

/// copy constructor
Vec::Vec(const Vec& other)
    : x(other.x),
      y(other.y),
      z(other.z) {
}

/// constructor initializing 2D vector
Vec::Vec(float x, float y)
    : x(x),
      y(y),
      z(0) {
}

/// constructor of the Vec class
/**
 \param x x-coordinate (left-right in standard view)
 \param y y-coordinate (up-down in standard view)
 \param z z-coordinate (front-back in standard view)
 */
Vec::Vec(float x, float y, float z)
    : x(x),
      y(y),
      z(z) {
}
/// calculate the Euclidian norm
float Vec::norm() const {
  float norm = 0;
  norm += this->x * this->x;
  norm += this->y * this->y;
  norm += this->z * this->z;
  return sqrt(norm);
}
/// normalize vector to length=1
/*
 \returns the normalized vector
 */
Vec Vec::normalized() const {
  float norm = this->norm();
  if (norm != 0) {
    return Vec(this->x / norm, this->y / norm, this->z / norm);
  } else {
    throw "cannot normalize null vector!";
  }
}
/// scale vector by factor
/**
 \param a scaling factor
 scaling is done by multiplying every element of the vector by the factor a
 */
Vec Vec::scaled(float a) const {
  return Vec(a * this->x, a * this->y, a * this->z);
}
/// add vector
/**
 \param v the vector to be added
 */
Vec Vec::add(Vec v) const {
  return Vec(this->x + v.x, this->y + v.y, this->z + v.z);
}
/// subtract vector
/**
 \param v the vector to be subtracted
 */
Vec Vec::sub(Vec v) const {
  return this->add(v.scaled( -1));
}

/// compare vector in all three dimensions
/**
 \param v the vector to be compared with
 */
bool Vec::equals(Vec v) {
  return (this->x == v.x) && (this->y == v.y) && (this->z == v.z);
}

// some operator overloading
Vec Vec::operator*(float a) const {
  return this->scaled(a);
}
Vec Vec::operator+(Vec v) const {
  return this->add(v);
}
Vec Vec::operator-(Vec v) const {
  return this->sub(v);
}
bool Vec::operator==(Vec v) {
  return this->equals(v);
}

/// rotate vector around given axis
/**
 \param rotAxis the axis around which the vector is to be rotated
 \param angle angle of rotation, given in degrees
 \returns the rotated vector
 */
Vec Vec::rotate(Vec rotAxis, float angle) const {
  if (angle == 0.0f) {
    return Vec( *this);
  }
  // first normalize rotation axis
  rotAxis = rotAxis.normalized();
  float n1 = rotAxis.x;
  float n2 = rotAxis.y;
  float n3 = rotAxis.z;
  // convert angle from degrees to radians
  angle = angle * 2 * M_PI / 360;
  // calculate cosine, (1-cosine), sine
  float c = cos(angle);
  float cc = 1 - c;
  float s = sin(angle);
  // calculate new coordinates by applying rotation matrix
  float x = this->x * (c + n1 * n1 * cc) + this->y * (n1 * n2 * cc - n3 * s) + this->z * (n1 * n3 * cc + n2 * s);
  float y = this->x * (n2 * n1 * cc + n3 * s) + this->y * (c + n2 * n2 * cc) + this->z * (n2 * n3 * cc - n1 * s);
  float z = this->x * (n3 * n1 * cc - n2 * s) + this->y * (n3 * n2 * cc + n1 * s) + this->z * (c + n3 * n3 * cc);
  return Vec(x, y, z);
}
/// rotate vector into coordinate system as defined by given orthonormal vectors
//Vec Vec::rotate(Vec xAxis, Vec yAxis, Vec zAxis){
//  xAxis = xAxis.normalized();
//  yAxis = yAxis.normalized();
//  zAxis = zAxis.normalized();
//  float x = this->x * xAxis.x + this->y * yAxis.x + this->z * zAxis.x;
//  float y = this->x * xAxis.y + this->y * yAxis.y + this->z * zAxis.y;
//  float z = this->x * xAxis.z + this->y * yAxis.z + this->z * zAxis.z;
//  return Vec(x, y, z);
//}

/// calculate the cross product
/**
 \param v the other vector
 \returns the cross product  (this X v)
 */
Vec Vec::cross(Vec v) const {
  Vec result;
  result.x = this->y * v.z - this->z * v.y;
  result.y = this->z * v.x - this->x * v.z;
  result.z = this->x * v.y - this->y * v.x;
  return result;
}

float Vec::dot(const Vec& v) const {
  return this->x*v.x + this->y*v.y + this->z*z;
}

float Vec::operator*(const Vec& v) const {
  return this->dot(v);
}


void Vec::setAsGlVertex3f() {
  glVertex3f(this->x, this->y, this->z);
}

} // end namespace
