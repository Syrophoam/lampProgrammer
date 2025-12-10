//
//  lampProgrammer.h
//  lampProg
//
//  Created by syro Fullerton on 28/11/2025.
//

#ifndef lampProgrammer_h
#define lampProgrammer_h

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

struct Response{
	char* response;
	char* curLine;
	int curLineLen;
	size_t responseLen;
};


struct Command{
	char* command;
	size_t commandLen;
	int deviceIsReady;
};

struct LampInfo {
	int *lampIndex;
	int connectedState;
	int fd;
	int *brightness; // 0 - 15
	struct Response resp;
	struct Command command;
	pthread_t comThread;
};

struct Animation {
	char* name;
	int numFrames;
	int fps;
	int** frames;
	int numlamps;
}; // COPIED TO SHIFT REG CODE UPDATE THERE IF UPDATING


struct AnimSim{
	int isPlaying;
	const struct Animation* animation;
};

enum AnimationState{
	NONE = 0,
	NUMLOADED = 1,
	PLAYING = 2,
	SENDING = 3
};

void initAnimations(void);

void createLampMapping(int numLamps);

char* openDevice(char* path);
void closeDevice(void);
int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);
void sendCommand(const char* com);

int getAnimationState(void);
int getNumAnimations(void);
const struct LampInfo* getLampInfo(void);
const char* getAnimationName(int animationIndex);

void addAnimation(const char* path);
int sendAnimation(const char* animationName);
const char* playAnimation(const char* animationName);
void stopAnimation(void);

void connectFaceToLamp(int face, int lamp);
void setLampBrightness(int lamp, int brightness, int sendToDev);

#endif /* lampProgrammer_h */
