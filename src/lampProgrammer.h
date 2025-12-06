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

struct LampMapping {
	int lampIndex[20]; //TODO: make variable for more lamps i make
	int connectedState;
	int fd;
	int brightness[20]; // 0 - 15
	struct Response resp;
	struct Command command;
	pthread_t comThread;
} lampMapping;

struct Animation {
	char* name;
	int numFrames;
	int fps;
	int** frames;
	int numlamps;
}; // COPIED TO SHIFT REG CODE UPDATE THERE IF UPDATING

struct Animation* animations;
int numAnimations;


int animationState;

pthread_t animationThread;
struct AnimSim{
	int isPlaying;
	const struct Animation* animation;
} animSimArgs;

void createLampMapping(void);
char* openDevice(char* path);
void closeDevice(void);
int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);
void sendCommand(const char* com);


struct Animation makeAnimation(const char* path);
int sendAnimation(const struct Animation* animation);
void playAnimation(const struct Animation* animation);
void stopAnimation(void);

void connectFaceToLamp(int face, int lamp);
void setLampBrightness(int lamp, int brightness, int sendToDev);

#endif /* lampProgrammer_h */
