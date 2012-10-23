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
#include <string>

#include <QApplication>

#include "tools.hpp"
#include "gui_opengl.hpp"

int main(int argc, char** argv){
  // load Qt-Resources
  Q_INIT_RESOURCE(res);

  // enable logging
  KBX::Logger::enableInfos();
  KBX::Logger::enableWarnings();
  KBX::Logger::enableErrors();

  // build logger instance for main
  KBX::Logger mainLog("main");

  try{
    // TODO: get config from cmd arguments + rc-files
    //Config config( HUMAN_AI, 4, KBX_Strategy(1.0) );
    QApplication app(argc, argv);

    KBX::GLWidget window;
    window.resize(800,600);
    window.show();

    return app.exec();

  }catch(const char* errMsg){
    mainLog.error( std::string(errMsg) );
    return 1;
  }
}

