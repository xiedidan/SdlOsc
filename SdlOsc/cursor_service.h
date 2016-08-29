#ifndef CURSOR_SERVICE_H
#define CURSOR_SERVICE_H

extern int mouseX, mouseY;
extern Uint32 mouseButtonMask;

// user interface
void drawCursor();

// helper
void cursorHandler();

#endif
