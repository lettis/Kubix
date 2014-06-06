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
#include <QColor> 

template <typename Iter, typename Cont>
bool is_last(Iter iter, const Cont& cont){
  return (iter != cont.end()) && (next(iter) == cont.end());
}

namespace KBX {
template< class NumType > int sgn(NumType n);
int sgn(float f);
int sgn(int i);
int sgnP(float f);

void swap(int& a, int& b);

void loadTextures();

void checkGLError();
std::string stringprintf(std::string str, ...);

/// represents a simple logger class, use it instead of cout/cerr-statements!
class Logger {
    std::string _name;
    static std::ostream* _out;
    static std::ostream* _err;
    static std::vector< std::string > _filters;
    static bool _infosEnabled;
    static bool _debugEnabled;
    static bool _warningsEnabled;
    static bool _errorsEnabled;
    static std::string _getTime();
    void _sendMessage(std::string category, std::string msg);
  public:
    Logger(std::string name);
    static void setOut(std::ostream* out);
    static void setErr(std::ostream* err);
    static void enableInfos();
    static void disableInfos();
    static void enableDebug();
    static void disableDebug();
    static void enableWarnings();
    static void disableWarnings();
    static void enableErrors();
    static void disableErrors();
    static void filter(std::string name);
    void info(std::string msg);
    void debug(std::string msg);
    void warning(std::string msg);
    void error(std::string msg);
};



/// defines a simple vector in 3d-cartesian coordinates
class Vec {
  public:
    float x;
    float y;
    float z;
    Vec();
    Vec(const Vec& other);
    Vec(float x, float y);
    Vec(float x, float y, float z);
    float norm() const;
    Vec normalized() const;
    Vec scaled(float a) const;
    Vec operator*(float a) const;
    Vec add(Vec v) const;
    Vec operator+(Vec v) const;
    Vec sub(Vec v) const;
    Vec operator-(Vec v) const;
    bool equals(Vec v);
    bool operator==(Vec v);
    Vec rotate(Vec rotAxis, float angle) const;
    Vec rotate(Vec xAxis, Vec yAxis, Vec zAxis) const;
    Vec cross(Vec v) const;
    float dot(const Vec& v) const;
    float operator*(const Vec& v) const;
    Vec round() const;
    void setAsGlVertex3f();
};

std::ostream& operator<<(std::ostream& out, const Vec&);


namespace NormalVectors {
const Vec Null = Vec(0.0f, 0.0f, 0.0f);
const Vec X = Vec(1.0f, 0.0f, 0.0f);
const Vec Y = Vec(0.0f, 1.0f, 0.0f);
const Vec Z = Vec(0.0f, 0.0f, 1.0f);
}

/// defines a color
class Color {
  public:
    float r;
    float g;
    float b;
    Color();
    Color(size_t id);
    Color(unsigned char r, unsigned char g, unsigned char b);
    Color(int r, int g, int b);
    Color(float r, float g, float b);
    Color(const QColor& qcolor);
    void setTo(const QColor& qcolor);
    size_t id() const;
    void setAsGlColor() const;

    Color& operator = (const QColor& qcolor);
};

namespace ColorTable {
const Color BLACK(0.0f, 0.0f, 0.0f);
const Color WHITE(1.0f, 1.0f, 1.0f);
const Color GREY10(0.1f, 0.1f, 0.1f);
const Color GREY20(0.2f, 0.2f, 0.2f);
const Color GREY30(0.3f, 0.3f, 0.3f);
const Color GREY40(0.4f, 0.4f, 0.4f);
const Color GREY50(0.5f, 0.5f, 0.5f);
const Color GREY60(0.6f, 0.6f, 0.6f);
const Color GREY70(0.7f, 0.7f, 0.7f);
const Color GREY80(0.8f, 0.8f, 0.8f);
const Color GREY90(0.9f, 0.9f, 0.9f);
const Color RED(1.0f, 0.0f, 0.0f);
const Color GREEN(0.0f, 1.0f, 0.0f);
const Color BLUE(0.0f, 0.0f, 1.0f);
const Color YELLOW(1.0f, 1.0f, 0.0f);
}

} // end namespace KBX
#endif
