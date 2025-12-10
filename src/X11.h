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
};

struct X* getX(void);

struct FrameThreadArgs{
	
	int shouldExit;
	const struct Mesh **meshes;
	int numMeshes;
	struct X *x;
	int* inputMode;
	char* commandBuffer;
	int cmdBufferLength;
	char* respBuffer;
	int respBufferLen;
	const struct LampInfo* lampInfo;
	
};

void* drawLoop(void* usrArg);
void initX(int width, int height, int fps);
void addMeshToScreen(const struct Mesh* mesh);
void quitX(void);
void setInputModeFlag(int* flag);
void setCommandBuffer(char* buff, int len);
void setRespBuffer(char* buff, int len);
void setLampInfo(const struct LampInfo* lampInfo);

#endif /* X11_h */
