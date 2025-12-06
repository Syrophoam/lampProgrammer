//
//  main.c
//  lampProgrammer
//
//  Created by syro Fullerton on 26/11/2025.
//

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "Math.h"
#include "OBJLoader.h"
#include "RenderEngine.h"
#include "X11.h"
#include "lampProgrammer.h"

static KeySym key;
static char text[255];
XEvent event;
int respLen = 128;
char* resp;

void reply(const char* r){
	memset(resp, ' ', respLen);
	memcpy(resp, r, strlen(r));
}

void processCommand(const char* command, int commandLen){
	
	if(commandLen == 0){
		reply("empty command");
		return;
	}
	
	if(strcmp(command, "hello") == 0){
		reply("HI!");
		return;
	}
	
	int numTokens = 0;
	char** tokens = NULL;
	
	int tokenLen = 0;
	for(int i = 0; i < commandLen; i++){
		
		if((command[i] == ' ') || (i == (commandLen - 1))){
			numTokens++;
			tokens = realloc(tokens, sizeof(char*) * numTokens);
			tokens[numTokens - 1] = malloc(tokenLen + 1);
			
			const char* tokenStart = command + i - tokenLen;
			if(i == (commandLen - 1))
				tokenLen++;
			
			memcpy(tokens[numTokens - 1], tokenStart, tokenLen);
			tokens[numTokens - 1][tokenLen] = '\0';
			tokenLen = -1;
		}
		tokenLen++;
	}
	
	
	if(strcmp(tokens[0], "set") == 0){
		
		int face = -1;
		int lamp = -1;
		int brightness = -1;
		
		if(strcmp(tokens[1], "face") == 0){
			if(numTokens < 5){
				reply("not enough arguments provided :(");
			}
			
			face = atoi(tokens[2]);
			
			if((strcmp(tokens[3], "to") & (strcmp(tokens[4], "lamp"))) == 0){
				lamp = atoi(tokens[5]);
				connectFaceToLamp(face - 1, lamp - 1); // idk where i got off by one but fixing it here ig
			}
			
		}else if(strcmp(tokens[1], "lamp") == 0){
			lamp = atoi(tokens[2]);
			
			if((strcmp(tokens[3], "to") & (strcmp(tokens[4], "face"))) == 0){
				face = atoi(tokens[5]);
				connectFaceToLamp(face - 1, lamp - 1);
				
			}else if((strcmp(tokens[3], "brightness") & strcmp(tokens[4], "to")) == 0 ){
				
				brightness = atoi(tokens[5]); 
				
				if(strcmp(tokens[2], "all") == 0){
					for (int i = 0; i < 20; i++) {
						setLampBrightness(i, brightness, 1);
					}
				}
				
				
				setLampBrightness(lamp - 1, brightness, 1);
			}
		}
		
	}
	
	
	if(strcmp(tokens[0], "open") == 0){
		
		if(numTokens > 1)
			printf("%s", openDevice(tokens[1]));
		else
			printf("no device provided\n");
	}
	
	if(strcmp(tokens[0], "close") == 0){
		
		if(lampMapping.connectedState > 0){
			closeDevice();
		}
		
	}
	
	if(strcmp(tokens[0], "led") == 0){
		
		if(strcmp(tokens[1], "on") == 0)
			sendCommand("led on\n");
		if(strcmp(tokens[1], "off") == 0)
			sendCommand("led off\n");
	}
	
	
	if(strcmp(tokens[0], "rand") == 0){
		sendCommand("rand\n");
	}
	
	if(strcmp(tokens[0], "animation") == 0){
		
		if(strcmp(tokens[1], "load") == 0){
			
			char* path = malloc(128);
			sprintf(path, "/Users/syro/Desktop/xcode/lampProgrammer/Media/%s", tokens[2]);
			struct Animation anim = makeAnimation(path);
			free(path);
			numAnimations++;
			animations = realloc(animations, sizeof(struct Animation) * numAnimations);
			animations[numAnimations - 1] = anim;
			
			char* rep = malloc(64);
			memset(rep, ' ', 64);
			sprintf(rep, "animation: %s loaded", anim.name);
			reply(rep);
			free(rep);
			
			animationState = NUMLOADED;
			
			return;
		}
		
		if(strcmp(tokens[1], "play") == 0){
			playAnimation(&animations[0]);
			
			char* rep = malloc(64);
			memset(rep, ' ', 64);
			sprintf(rep, "animation: %s playing", animations[0].name);
			reply(rep);
			free(rep);
			animationState = PLAYING;
			return;
		}
		
		if(strcmp(tokens[1], "stop") == 0){
			stopAnimation();
			animationState = NUMLOADED;
			return;
		}
		
		if(strcmp(tokens[1], "send") == 0){
			
			reply("sending animation");
			animationState = SENDING;
			if(sendAnimation(&animations[0])){
				reply("sent animation");
			}else{
				reply("failed to send, check if device is connected");
			}
			animationState = NUMLOADED;
			
			return;
		}
		
	}
	
	
	for (int i = 0; i < numTokens; i++) {
		free(tokens[i]);
	}
	free(tokens);
	memset(resp, ' ', respLen);
}

int main(void){
	
	loadFile("/Users/syro/Desktop/xcode/lampProgrammer/Media/icosahedron.obj");
	initEngine();
	initX(800, 600, 24);
	
	
	addMeshToScreen(getMesh(0));
	
	createLampMapping();
	
	int textInputMode = 0;
	char input[64];
	memset(input, ' ', sizeof(input));
	int inputPos = 0;
	setInputModeFlag(&textInputMode);
	setCommandBuffer(input, 64);
	
	resp = malloc(respLen);
	memset(resp, ' ', respLen);
	
	setRespBuffer(resp, respLen);
	setLampMapping(&lampMapping);
	
	animations = NULL;
	numAnimations = 0;
	animationState = NONE;
	
	while (1) {
		XNextEvent(x.display, &event);
		
		switch (event.type) {
			case KeyPress:{
				 
				
				if(XLookupString(&event.xkey, text, 255, &key, 0) == 1){
					
					if(textInputMode){
						
						if(event.xkey.keycode == 44){
							input[inputPos] = '\0';
							processCommand(input, inputPos);
							memset(input, ' ', sizeof(input));
							inputPos = 0;
							textInputMode = 0;
						}else if(event.xkey.keycode == 59){
							if(inputPos == 0)
								continue;
							
							inputPos--;
							input[inputPos] = ' ';
						}else{
							
							input[inputPos] = text[0];
							inputPos = (++inputPos == 64) ? 0 : inputPos;
						}
							
						
						continue;
					}
					
					if(text[0] == 'q'){
						closeDevice();
						quitX();
						exit(0);
					}
					
					if(text[0] == 'i'){
						moveCam(0, 0, 0.1);
					}
					if(text[0] == 'o'){
						moveCam(0, 0, -0.1);
					}
					if(text[0] == 'I'){
						zoom(0.1);
					}
					if(text[0] == 'O'){
						zoom(-0.1);
					}
					
					
					if(text[0] == ','){
						rotateMesh(0, M_PI / 24.);
					}
					if(text[0] == '.'){
						rotateMesh(0, -(M_PI / 24.));
					}
					
					if(text[0] == ':'){
						textInputMode = 1;
						
					}
					
				}
				
				break;
			}
				
			default:
				break;
		}
		
	}
	
	
	
	
	
	return 0;
}
