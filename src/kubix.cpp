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
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>
#include <map>

#include "SDL.h"
#include "SDL_opengl.h"

#include "gui.hpp"
#include "handler.hpp"

void setupBoard(KBX_Scene* scene){
    // define rotation axes
    KBX_Vec toFront(-1.0, 0.0,  0.0);
    KBX_Vec toBack ( 1.0, 0.0,  0.0);
    KBX_Vec toLeft ( 0.0, 0.0,  1.0);
    KBX_Vec toRight( 0.0, 0.0, -1.0);
    KBX_Vec clockwise( 0.0, -1.0, 0.0);
    KBX_Vec counterClockwise( 0.0, 1.0, 0.0);
    // setup dice with correct orientation
    // white dice; w1 is in lower left corner, w8 in lower right
    KBX_Die* w1 = new KBX_Die( KBX_Vec(-4,0, 4), KBX_Die::WHITE );
    w1->rotate( counterClockwise, 90 );
    KBX_Die* w2 = new KBX_Die( KBX_Vec(-3,0, 4), KBX_Die::WHITE );
    w2->rotate( toBack, 90 );
    w2->rotate( counterClockwise, 90 );
    KBX_Die* w3 = new KBX_Die( KBX_Vec(-2,0, 4), KBX_Die::WHITE );
    w3->rotate( toBack, 180 );
    w3->rotate( counterClockwise, 90 );
    KBX_Die* w4 = new KBX_Die( KBX_Vec(-1,0, 4), KBX_Die::WHITE );
    w4->rotate( toFront, 90 );
    w4->rotate( counterClockwise, 90 );
    KBX_Die* wK = new KBX_Die( KBX_Vec( 0,0, 4), KBX_Die::WHITE, true );
    KBX_Die* w5 = new KBX_Die( KBX_Vec( 1,0, 4), KBX_Die::WHITE );
    w5->rotate( toFront, 90 );
    w5->rotate( counterClockwise, 90 );
    KBX_Die* w6 = new KBX_Die( KBX_Vec( 2,0, 4), KBX_Die::WHITE );
    w6->rotate( toBack, 180 );
    w6->rotate( counterClockwise, 90 );
    KBX_Die* w7 = new KBX_Die( KBX_Vec( 3,0, 4), KBX_Die::WHITE );
    w7->rotate( toBack, 90 );
    w7->rotate( counterClockwise, 90 );
    KBX_Die* w8 = new KBX_Die( KBX_Vec( 4,0, 4), KBX_Die::WHITE );
    w8->rotate( counterClockwise, 90 );
    // black dice; b1 is in upper left corner, b8 in upper right
    KBX_Die* b1 = new KBX_Die( KBX_Vec(-4,0,-4), KBX_Die::BLACK );
    b1->rotate( clockwise, 90 );
    KBX_Die* b2 = new KBX_Die( KBX_Vec(-3,0,-4), KBX_Die::BLACK );
    b2->rotate( toBack, 90 );
    b2->rotate( clockwise, 90 );
    KBX_Die* b3 = new KBX_Die( KBX_Vec(-2,0,-4), KBX_Die::BLACK );
    b3->rotate( toBack, 180 );
    b3->rotate( clockwise, 90 );
    KBX_Die* b4 = new KBX_Die( KBX_Vec(-1,0,-4), KBX_Die::BLACK );
    b4->rotate( toFront, 90 );
    b4->rotate( clockwise, 90 );
    KBX_Die* bK = new KBX_Die( KBX_Vec( 0,0,-4), KBX_Die::BLACK, true );
    KBX_Die* b5 = new KBX_Die( KBX_Vec( 1,0,-4), KBX_Die::BLACK );
    b5->rotate( toFront, 90 );
    b5->rotate( clockwise, 90 );
    KBX_Die* b6 = new KBX_Die( KBX_Vec( 2,0,-4), KBX_Die::BLACK );
    b6->rotate( toBack, 180 );
    b6->rotate( clockwise, 90 );
    KBX_Die* b7 = new KBX_Die( KBX_Vec( 3,0,-4), KBX_Die::BLACK );
    b7->rotate( toBack, 90 );
    b7->rotate( clockwise, 90 );
    KBX_Die* b8 = new KBX_Die( KBX_Vec( 4,0,-4), KBX_Die::BLACK );
    b8->rotate( clockwise, 90 );
    // add dice to scene
    scene->add( w1 );
    scene->add( w2 );
    scene->add( w3 );
    scene->add( w4 );
    scene->add( w5 );
    scene->add( w6 );
    scene->add( w7 );
    scene->add( w8 );
    scene->add( wK );
    scene->add( b1 );
    scene->add( b2 );
    scene->add( b3 );
    scene->add( b4 );
    scene->add( b5 );
    scene->add( b6 );
    scene->add( b7 );
    scene->add( b8 );
    scene->add( bK );
    // initialize the board and add it to the scene
    KBX_Board* board = new KBX_Board(9, 9);
    scene->add( board );
}


int main(){
    // initialize SDL
    initSDL();
    // initialize OpenGL
    initOpenGL();
    // declare filename-key combinations for die faces
    std::map<size_t, std::string> dieFaces;
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_K_W,  "./res/side1.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_1_W,  "./res/side1.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_2_W,  "./res/side2.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_3_W,  "./res/side3.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_4_W,  "./res/side4.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_5_W,  "./res/side5.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_6_W,  "./res/side6.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_K_B, "./res/side1b.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_1_B, "./res/side1b.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_2_B, "./res/side2b.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_3_B, "./res/side3b.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_4_B, "./res/side4b.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_5_B, "./res/side5b.bmp")  );
    dieFaces.insert(  std::pair<size_t, std::string> (KBX_Die::FACE_6_B, "./res/side6b.bmp")  );

    try{
        // initialize (load) die face textures
        KBX_Die::textures.load(dieFaces);
        // initialize scene
        KBX_Scene* scene = new KBX_Scene();
        // setup the board with all dice
        setupBoard(scene);
        // initialize event handlers
        KBX_ExitEventHandler exitEvents(scene);
        KBX_MotionEventHandler motionEvents(scene);
        KBX_SelectionEventHandler selectionEvents(scene);
        // enter event loop
        SDL_Event* event = new SDL_Event();
        bool DONE=false;
        while ( !DONE ){
            // get next event
            SDL_PollEvent( event );
            // handle possible exit events
            if (exitEvents.handle( event ) != 0){
                DONE=true;
            }
            // handle possible selection events
            if(!selectionEvents.handle( event )){
                // handle possible motion events
                motionEvents.handle( event );
            }
            // redraw scene
            scene->display();
            SDL_GL_SwapBuffers();
        }
    }catch(const char* errMsg){
        std::cout << "error: " << errMsg << std::endl;
    }
    SDL_Quit();
    return 0;
}

