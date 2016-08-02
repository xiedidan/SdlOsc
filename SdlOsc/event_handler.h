#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

void eventHandler(SDL_Event* event);

// helper
void keyEventHandler(SDL_Keysym* keysym);
void mouseEventHandler();

#endif
