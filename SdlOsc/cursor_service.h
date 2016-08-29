#ifndef CURSOR_SERVICE_H
#define CURSOR_SERVICE_H

extern volatile int mouseX, mouseY;
extern volatile Uint32 mouseButtonMask;

// user interface
void startCursorThread();
void stopCursorThread();
void drawCursor();

// helper
int cursorThreadFunc(void* data);

#endif
