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
#ifndef CONTROLLER__HPP
#define CONTROLLER__HPP

#include <map>

#include "SDL.h"
#include "SDL_opengl.h"

#include "gui.hpp"
#include "engine.hpp"

class KBX_Controller{
    KBX_Scene* _scene;
    KBX_Game*  _game;
    KBX_Board* _board;
    std::vector<KBX_Die*> _dice;
    std::map<size_t, size_t> _id2Die;
public:
    KBX_Controller(KBX_Scene* scene, KBX_Game* game);
    int handle(SDL_Event* event);
};
#endif
