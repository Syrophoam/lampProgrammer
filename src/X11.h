//
//  X11Setup.h
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#ifndef X11_h
#define X11_h

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <pthread.h>
#include <stdlib.h>

#include "RenderEngine.h"
#include "Math.h"
#include "OBJLoader.h"
#include "lampProgrammer.h"

struct X{
	Window window;

	Display* display;
	int screen;
	GC gc;
	
	struct timespec tm;
	unsigned int width, height;
	Visual* visual;
	
	unsigned long black;
	unsigned long white;
	
	pthread_t drawThread;
	
	XColor* greyScale;
	
	int frame;
} x;

struct FrameThreadArgs{
	
	int shouldExit;
	struct Vec3f *points;
	int* pointIsVisible;
	const struct Mesh *mesh;
	struct X *x;
	int* inputMode;
	char* commandBuffer;
	int cmdBufferLength;
	char* respBuffer;
	int respBufferLen;
	struct LampMapping* lampMapping;
	
} *frameThreadArgs;

enum AnimationState{
	NONE = 0,
	NUMLOADED = 1,
	PLAYING = 2,
	SENDING = 3
};

void* drawLoop(void* usrArg);
void initX(int width, int height, int fps);
void addMeshToScreen(const struct Mesh* mesh);
void quitX(void);
void setInputModeFlag(int* flag);
void setCommandBuffer(char* buff, int len);
void setRespBuffer(char* buff, int len);
void setLampMapping(struct LampMapping* lampMapping);

#endif /* X11_h */
