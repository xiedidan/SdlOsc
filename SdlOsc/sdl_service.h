#ifndef SDL_SERVICE_H
#define SDL_SERVICE_H

#define FPS_TARGET 50
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define RULER_X_COUNT 13
#define RULER_Y_COUNT 7

#define DIV_WIDTH WINDOW_WIDTH / RULER_X_COUNT
#define DIV_HEIGHT WINDOW_HEIGHT / RULER_Y_COUNT

void sdlExit(char* msg);
void initGL(int width, int height);
void resizeGL(int width, int height);
void initSDL(int width, int height);
void eventHandler();
void renderBuffer();

// draw
void drawRuler();
void drawFrame(GLfloat* vertices, GLfloat* colors);

// render thread
void startRender();
void stopRender();
int renderThreadFunc(void* data);

// helper
void keyEventHandler(SDL_Keysym* keysym);

#endif // SDL_SERVICE_H