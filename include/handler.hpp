#ifndef HANDLER__HPP
#define HANDLER__HPP

#include "SDL.h"

#include "gui.hpp"

/// general event handling class
class KBX_EventHandler{
protected:
    KBX_Scene* scene;
public:
    KBX_EventHandler(KBX_Scene* scene);
    virtual int handle(SDL_Event* event)=0;
};

/// handler for exit events
class KBX_ExitEventHandler : public KBX_EventHandler{
public:
    KBX_ExitEventHandler(KBX_Scene* scene) :KBX_EventHandler(scene) {};
    int handle(SDL_Event* event);
};
#endif
