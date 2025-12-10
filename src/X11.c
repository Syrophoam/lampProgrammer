//
//  X11.c
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#include "X11.h"

struct X x;
struct FrameThreadArgs* frameThreadArgs;

 
struct X* getX(void){ return &x; }

void drawChip(int xPos, int yPos, int frameIndex, const struct LampInfo* lampInfo){
	
	XTextItem chipInfo;
	chipInfo.nchars = 1;
	char* loadingSymbols[] = {"\\", "/"};
	
	XDrawRectangle(x.display, x.window, x.gc, xPos, yPos, 19, 19);
		
	for (int p = 0; p < 6; p++) {
		int mult = 15. / 5.;
		
		XDrawLine(x.display, x.window, x.gc, xPos + (p * mult) + 2, yPos, xPos + (p * mult) + 2, yPos - 5);
		XDrawLine(x.display, x.window, x.gc, xPos + (p * mult) + 2, yPos + 20, xPos + (p * mult) + 2, yPos + 25);
		XDrawLine(x.display, x.window, x.gc, xPos + 19, yPos + (p * mult) + 2, xPos + 24, yPos + (p * mult) + 2);
		XDrawLine(x.display, x.window, x.gc, xPos, yPos + (p * mult) + 2, xPos - 5, yPos + (p * mult) + 2);
	}
	
	if(lampInfo->connectedState){
		if(lampInfo->command.deviceIsReady){
			chipInfo.chars = "O";
		}else{
			chipInfo.chars = loadingSymbols[ (frameIndex / 8) % 2];
		}
	}else{
		chipInfo.chars = "X";
	}
	
	
	XDrawText(x.display, x.window, x.gc,
			  xPos + 7, yPos + 15,
			  &chipInfo, 1);
	
}

void drawFilm(int xPos, int yPos, int frameIndex){
	
	XTextItem animInfo;
	animInfo.font = None;
	animInfo.nchars = 1;
	animInfo.chars = NULL;
	char* playingSymbols[] = {"[", "|", "]"};
	char* sendingSymbols[] = {"\\", "/"};
	
	XSetForeground(x.display, x.gc, x.white);
	XFillRectangle(x.display, x.window, x.gc, xPos - 5, yPos, 30, 25);
	
	XSetForeground(x.display, x.gc, x.black);
	for (int i = 0; i < 6; i++) {
		int mult = 41 / 6;
		XFillRectangle(x.display, x.window, x.gc, (xPos - 3) + (i * mult), yPos + 2, 3, 3);
		XFillRectangle(x.display, x.window, x.gc, (xPos - 3) + (i * mult), yPos + 20, 3, 3);
	}
	XFillRectangle(x.display, x.window, x.gc, xPos - 4, yPos + 6, 28, 13);
	
	XSetForeground(x.display, x.gc, x.white);

	int shouldFree = 0;
	if(getAnimationState() == NONE){
		animInfo.chars = "X";
	}else if(getAnimationState() == NUMLOADED){
		if(getNumAnimations() > 9){
			animInfo.chars = malloc(2);
			animInfo.nchars = 2;
		}else{
			animInfo.chars = malloc(1);
		}
		sprintf(animInfo.chars, "%i", getNumAnimations());
		
		shouldFree = 1;
	}else if(getAnimationState() == SENDING){
		animInfo.chars = sendingSymbols[(frameIndex / 8) % 2];
	}else if(getAnimationState() == PLAYING){
		animInfo.chars = playingSymbols[(frameIndex / 8) % 3];
	}

	XDrawText(x.display, x.window, x.gc,
			  xPos + 6, yPos + 17,
			  &animInfo, 1);
	if(shouldFree)
		free(animInfo.chars);
	
}

void drawMesh(int meshIndex, const struct X* x){
	
	struct Vec3f* points = malloc(sizeof(struct Vec3f) * getNumPoints(meshIndex));
	for (int i = 0; i < getNumPoints(meshIndex); i++) {
		points[i] = projectPoint(getPoint(meshIndex, i), x->width, x->height, getMesh(meshIndex));
	}
	drawProjection(points, getMesh(meshIndex), x, getLampInfo());
	free(points);
	
}

int compMesh(void* thunk, const void* a, const void* b){
	
	int A = *(int*)a;
	int B = *(int*)b;
	float aZ = ((float*)thunk)[A];
	float bZ = ((float*)thunk)[B];
	if(aZ > bZ){ return 1; }
	if(aZ < bZ){ return -1;}
	return 0;
}

void* drawLoop(void* usrArg){
	
	struct FrameThreadArgs *args = (struct FrameThreadArgs*)usrArg;
	
	XTextItem title;
	title.chars = "Lamp Programmer";
	title.nchars = 15;
	
	XTextItem input;
	input.chars = ":";
	input.nchars = 1;
	
	XTextItem inputCMD;
	inputCMD.chars = args->commandBuffer;
	inputCMD.nchars = args->cmdBufferLength;
	
	XTextItem cmdResp;
	cmdResp.chars = args->respBuffer;
	cmdResp.nchars = args->respBufferLen;
	
	XTextItem mapTable;
	mapTable.chars = "Face Index        Lamp Index";
	mapTable.nchars = 28;
	
	XTextItem *faceIndexList = calloc(getNumFaces(0), sizeof(XTextItem));
	XTextItem *lampIndexList = calloc(getNumFaces(0), sizeof(XTextItem));
	
	for (int i = 0; i < getNumFaces(0); i++) {
		faceIndexList[i].chars = malloc(4);
		lampIndexList[i].chars = malloc(4); // TODO: free my mans
		
		sprintf(faceIndexList[i].chars, "%i", i + 1);
		faceIndexList[i].nchars = ((i+1) < 10) ? 1 : 2;
		
		sprintf(lampIndexList[i].chars, "%i", i + 1);
		lampIndexList[i].nchars = ((i+1) < 10) ? 1 : 2;
	}
	
	int frameIndex = 0;
	while(!args->shouldExit){
		
		XClearWindow(x.display, x.window);
		
		int* zSortedMeshIndicies = malloc(sizeof(int) * getNumMeshes());
		float* meshZ = malloc(sizeof(float) * getNumMeshes());
		for (int i = 0; i < getNumMeshes(); i++) {
			zSortedMeshIndicies[i] = i;
			struct Vec3f org = getMesh(i)->origin;
			rotateVectorY(getMesh(i)->yRotation, &org);
			meshZ[i] = org.z;
		}
		
		qsort_r(zSortedMeshIndicies, getNumMeshes(), sizeof(int), meshZ, compMesh);
		free(meshZ);

		for (int i = 0; i < getNumMeshes(); i++) {
			drawMesh(zSortedMeshIndicies[i], &x);
		}
		
		free(zSortedMeshIndicies);
		
		
		XSetLineAttributes(x.display, x.gc, 0, LineSolid, CapButt, JoinMiter);
		
		XSetForeground(x.display, x.gc, x.white);
		XDrawText(x.display, x.window, x.gc,
				  10, 15,
				  &title, 1);
		
		XDrawText(x.display, x.window, x.gc,
				  620, 15,
				  &mapTable, 1);
		
		for (int i = 0; i < getNumFaces(0); i++) {
			XDrawText(x.display, x.window, x.gc,
					  650, 35 + (i * 20),
					  &faceIndexList[i], 1);
			
			XDrawText(x.display, x.window, x.gc,
					  750, 35 + (i * 20),
					  &lampIndexList[i], 1);
			
			if(args->lampInfo->lampIndex[i] < 0)
				continue;
			
			XDrawLine(x.display, x.window, x.gc,
					  670, 30 + (i * 20),
					  740, 30 + (args->lampInfo->lampIndex[i] * 20));
			
		}
		
		if((args->inputMode != NULL) && (*args->inputMode == 1)){
			XDrawText(x.display, x.window, x.gc,
					  10, x.height - 10,
					  &input, 1);
			
			XDrawText(x.display, x.window, x.gc,
					  20, x.height - 10,
					  &inputCMD, 1);
			
		}else if(args->respBufferLen){
			XDrawText(x.display, x.window, x.gc,
					  20, x.height - 10,
					  &cmdResp, 1);
		}
		
		drawChip(20, 35, frameIndex, args->lampInfo);
		drawFilm(20, 70, frameIndex);
		
		XFlush(x.display);
		
		frameIndex++;
		nanosleep(&x.tm, NULL);
		
	}
	
	return NULL;
}


void initX(int width, int height, int fps){
	
	x.width = width;
	x.height = height;

	x.display = XOpenDisplay(NULL);
	x.screen = XDefaultScreen(x.display);
	
	x.visual = XDefaultVisual(x.display, x.screen);
	
	x.black = XBlackPixel(x.display, x.screen);
	x.white = XWhitePixel(x.display, x.screen);
	 
	x.window = XCreateSimpleWindow(x.display, XRootWindow(x.display, x.screen),
								   0, 0,
								   width, height,
								   1,
								   x.white,
								   x.black);
		
	XStoreName(x.display, x.window, "Lamp Programmer");
	XSelectInput(x.display, x.window, ExposureMask | KeyPressMask | ButtonPressMask );
	x.gc = XCreateGC(x.display, x.window, 0, 0);
	

	XSetBackground(x.display, x.gc, x.black);
	XSetForeground(x.display, x.gc, x.white);
	
	x.greyScale = malloc(sizeof(XColor) * 16);
	
	for (int i = 0; i < 16; i++) {
		x.greyScale[i].flags = DoRed | DoGreen | DoBlue;
		
		unsigned short val = ((float)i / 15.) * 0xFFFF;
		x.greyScale[i].red = val;
		x.greyScale[i].green = val;
		x.greyScale[i].blue = val;
		
		XAllocColor(x.display, XDefaultColormap(x.display, x.screen), &x.greyScale[i]);
	}
	
	XMapRaised(x.display, x.window);
	
	x.tm.tv_sec = 0;
	x.tm.tv_nsec = 1000000000. / (float)fps;
	
	x.frame = 0;
	
	frameThreadArgs = calloc(1, sizeof(struct FrameThreadArgs));
	frameThreadArgs->x = &x;
	
	XInitThreads();
	
	
	pthread_create(&x.drawThread, NULL, &drawLoop, frameThreadArgs);
	
}

void addMeshToScreen(const struct Mesh* mesh){
	frameThreadArgs->meshes = realloc(frameThreadArgs->meshes, sizeof(struct Mesh*) * (frameThreadArgs->numMeshes + 1));
	frameThreadArgs->meshes[frameThreadArgs->numMeshes] = mesh;
	frameThreadArgs->numMeshes++;
}
void setInputModeFlag(int* flag){
	frameThreadArgs->inputMode = flag;
}
void setCommandBuffer(char* buff, int len){
	frameThreadArgs->commandBuffer = buff;
	frameThreadArgs->cmdBufferLength = len;
}
void setRespBuffer(char* buff, int len){
	frameThreadArgs->respBuffer = buff;
	frameThreadArgs->respBufferLen = len;
}
 
void setLampInfo(const struct LampInfo* lampInfo){
	frameThreadArgs->lampInfo = lampInfo; 
}

void quitX(void){
	
	XFreeGC(x.display, x.gc);
	XDestroyWindow(x.display, x.window);
	XCloseDisplay(x.display);
	
	
}


