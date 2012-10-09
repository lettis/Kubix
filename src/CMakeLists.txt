

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

add_executable(../Kubix kubix.cpp controller.cpp engine.cpp eventHandler.cpp gui.cpp tools.cpp)
target_link_libraries(../Kubix ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${OPENGL_LIBRARIES})

