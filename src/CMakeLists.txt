
cmake_minimum_required (VERSION 2.6)

project (Kubix)

set (Kubix_VERSION_MAJOR 1)
set (Kubix_VERSION_MINOR 0)

include_directories("../include")

INCLUDE(FindSDL)
INCLUDE(FindSDL_image)
INCLUDE(FindOpenGL) 

if(SDL_FOUND)
  include_directories( ${SDL_INCLUDE_DIR} )
else(SDL_FOUND)
  message( FATAL_ERROR "SDL not found" )
endif(SDL_FOUND)

if(SDLIMAGE_FOUND)
  include_directories( ${SDLIMAGE_INCLUDE_DIR} )
else(SDLIMAGE_FOUND)
  message( FATAL_ERROR "SDL_image not found" )
endif(SDLIMAGE_FOUND)

if(OPENGL_FOUND)
  include_directories( ${OPENGL_INCLUDE_DIR} )
else(OPENGL_FOUND)
  message( FATAL_ERROR "OpenGL not found" )
endif(OPENGL_FOUND)

add_library(controller controller/controller.cpp)
add_library(engine engine/engine.cpp)
add_library(eventHandler eventHandler/handler.cpp)
add_library(gui gui/gui.cpp)
add_library(tools tools/tools.cpp)

add_executable(Kubix kubix.cpp)
target_link_libraries(Kubix controller engine eventHandler gui tools ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${OPENGL_LIBRARIES})

