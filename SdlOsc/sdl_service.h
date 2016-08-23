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
SDL_Window* initSDL(int width, int height);
void renderBuffer();

// draw
void drawRuler();
void drawFrame(GLfloat* vertices, int pixelCount);

// render thread
void startRender();
void stopRender();
int renderThreadFunc(void* data);

#endif // SDL_SERVICE_H