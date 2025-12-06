//
//  X11.c
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#include "X11.h"

void drawChip(int xPos, int yPos, int frameIndex){
	
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
	
	if(lampMapping.connectedState){
		if(lampMapping.command.deviceIsReady){
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
	animInfo.nchars = 1;
	animInfo.chars = NULL;
	char* playingSymbols[] = {"[", "|", "]"};
	
	XSetForeground(x.display, x.gc, x.white);
	XFillRectangle(x.display, x.window, x.gc, xPos - 5, yPos, 30, 25);
	
	XSetForeground(x.display, x.gc, x.black);
	for (int i = 0; i < 6; i++) {
		int mult = 41 / 6;
		XFillRectangle(x.display, x.window, x.gc, (xPos - 3) + (i * mult), yPos + 2, 3, 3);
		XFillRectangle(x.display, x.window, x.gc, (xPos - 3) + (i * mult), yPos + 20, 3, 3);
	}
	XFillRectangle(x.display, x.window, x.gc, xPos - 4, yPos + 6, 28, 13);
	
	if(animationState == 0){
		animInfo.chars = "X";
	}else if(animationState == NUMLOADED){
		if(numAnimations > 9){
			animInfo.chars = malloc(2);
			animInfo.nchars = 2;
		}else{
			animInfo.chars = malloc(1);
		}
		sprintf(animInfo.chars, "%i", numAnimations);
		free(animInfo.chars);
	}else if(animationState == SENDING){
		animInfo.chars = playingSymbols[frameIndex % 3];
	}
	
	XDrawText(x.display, x.window, x.gc,
			  xPos + 7, yPos + 15,
			  &animInfo, 1);
	
}

void* drawLoop(void* usrArg){
	
	struct FrameThreadArgs *args = (struct FrameThreadArgs*)usrArg;
	
	args->pointIsVisible = malloc(sizeof(int) * getNumPoints(0));
	args->points = malloc(sizeof(struct Vec3f) * getNumPoints(0));
	
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
	
	XTextItem faceIndexList[20];
	XTextItem lampIndexList[20];
	
	for (int i = 0; i < 20; i++) {
		faceIndexList[i].chars = malloc(2);
		lampIndexList[i].chars = malloc(2); // TODO: free my mans
		
		sprintf(faceIndexList[i].chars, "%i", i + 1);
		faceIndexList[i].nchars = ((i+1) < 10) ? 1 : 2;
		
		sprintf(lampIndexList[i].chars, "%i", i + 1);
		lampIndexList[i].nchars = ((i+1) < 10) ? 1 : 2;

	}
	
	int frameIndex = 0;
	
	while(!args->shouldExit){
		
		XLockDisplay(x.display);
		
		XClearWindow(x.display, x.window);
		 
		for (int i = 0; i < getNumPoints(0); i++) {
			args->points[i] = projectPoint(getPoint(0,i), x.width, x.height, &args->pointIsVisible[i], args->mesh);
		}
		 
		drawProjection(args->points, args->pointIsVisible, args->mesh, args->x, args->lampMapping);
		XSetLineAttributes(x.display, x.gc, 0, LineSolid, CapButt, JoinMiter);
		
		XSetForeground(x.display, x.gc, x.white);
		XDrawText(x.display, x.window, x.gc,
				  10, 15,
				  &title, 1);
		
		XDrawText(x.display, x.window, x.gc,
				  620, 15,
				  &mapTable, 1);
		
		for (int i = 0; i < 20; i++) {
			XDrawText(x.display, x.window, x.gc,
					  650, 35 + (i * 20),
					  &faceIndexList[i], 1);
			
			XDrawText(x.display, x.window, x.gc,
					  750, 35 + (i * 20),
					  &lampIndexList[i], 1);
			
			if(lampMapping.lampIndex[i] < 0)
				continue;
			
			XDrawLine(x.display, x.window, x.gc,
					  670, 30 + (i * 20),
					  740, 30 + (lampMapping.lampIndex[i] * 20));
			
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
		
		drawChip(20, 35, frameIndex);
		drawFilm(20, 70, frameIndex);
		
		XFlush(x.display);
		
		XUnlockDisplay(x.display);
		
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
								   1000, 300,
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
	
//	XStoreColors(x.display, XDefaultColormap(x.display, x.screen), x.greyScale, 16);
	
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
	frameThreadArgs->mesh = mesh; 
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

void setLampMapping(struct LampMapping* lampMapping){
	frameThreadArgs->lampMapping = lampMapping;
}


void quitX(void){
	
	XFreeGC(x.display, x.gc);
	XDestroyWindow(x.display, x.window);
	XCloseDisplay(x.display);
	
	
}
