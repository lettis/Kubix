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
#ifndef HANDLER__HPP
#define HANDLER__HPP

#include <sstream>
#include <string>

#include <QEvent>

#include "models.hpp"

namespace KBX {
  /// general event handling class
  class EventHandler{
  protected:
      Scene* scene;
  public:
      EventHandler(Scene* scene);
      virtual int handle(QEvent* event)=0;
  };
  
  /// handler for exit events
  class ExitEventHandler : public EventHandler{
  public:
      ExitEventHandler(Scene* scene);
      int handle(QEvent* event);
  };
  
  /// handler for motion events
  class MotionEventHandler : public EventHandler{
      bool resize;
      bool keydown;
      bool fullscreen;
      float zoom;
      float rotateHorizontal;
      float rotateVertical;
      float width;
      float height;
      bool cameraDrag;
      float clickPosX;
      float clickPosY;
  public:
      MotionEventHandler(Scene* scene);
      void proceed();
      int handle(QEvent* event);
  };
  
  /// handler for motion events
  class SelectionEventHandler : public EventHandler{
  public:
      Object* selectedObj;
      SelectionEventHandler(Scene* scene);
      int handle(QEvent* event);
      Object* getObject( size_t x, size_t y );
  };
  
  /// handler for text/console events
  class ConsoleEventHandler : public EventHandler{
      std::stringstream input;
      bool active;
  public:
      ConsoleEventHandler(Scene* scene);
      int handle(QEvent* event);
  };
} // end namespace KBX
#endif
