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
#include <stdlib.h>
#include <time.h>

#include <QApplication>

#include "tools.hpp"
#include "main_window.hpp"

class App: public QApplication {
  public:
    App(int &argc, char* argv[])
        : QApplication(argc, argv) {    
    }
    bool notify(QObject * receiver, QEvent * event) {
      KBX::Logger log("events");
      bool done = true;
      try {
        done = QApplication::notify(receiver, event);
      } catch (const std::exception& e) {
        log.error(KBX::stringprintf("Exception thrown: %s", e.what()));
      } catch (const char* s){
        log.error(s);
        exit(1);
      }
      return done;
    }
};

int main(int argc, char** argv) {
  // load Qt-Resources
  Q_INIT_RESOURCE(res);

  // enable logging
  //KBX::Logger::enableInfos();
  //KBX::Logger::enableWarnings();
  //KBX::Logger::enableErrors();
  //KBX::Logger::enableDebug();
//  KBX::Logger::filter("DieState");
//  KBX::Logger::filter("evaluation");

  // TODO think about using boost::program_options
  bool quit = false;
  std::string loadgame = "";
  std::string randomseed = "";
  std::string* val = NULL;
  for(int i=1; i<argc; i++){
    std::string arg(argv[i]);
    if(val){
      *val = arg;
      val = NULL;
      continue;
    }
    if(arg == "-q") {
      quit = true;
    }
    if(arg == "--load-game"){
      val = &loadgame;
    }
    if(arg== "--random-seed"){
      val = &randomseed;
    }
  }
  
  try {
    QApplication::setAttribute(Qt::AA_X11InitThreads);
    App app(argc, argv);
    MainWindow *window = new MainWindow();
    window->show();
    if(loadgame.size() > 0){
      window->loadGameFromFile(loadgame);
    } else if(KBX::fileExists(".autosave.kbx")){
      window->loadGameFromFile(".autosave.kbx");
    }
    if(randomseed.size() > 0){
      srand(atoi(randomseed.c_str()));
    } else {
      srand(time(NULL));
    }
    if(quit) return 0;
    return app.exec();
  } catch (const char* errMsg) {
    KBX::Logger("main").error(std::string(errMsg));
    return 1;
  }
}
