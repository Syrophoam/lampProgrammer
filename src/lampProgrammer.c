//
//  lampProgrammer.c
//  lampProg
//
//  Created by syro Fullerton on 28/11/2025.
//

#include "lampProgrammer.h"

void createLampMapping(void){
	
	for (int i = 0; i < 20; i++) {
		lampMapping.lampIndex[i] = i;
		lampMapping.brightness[i] = rand() % 15;
	}
	
	lampMapping.connectedState = 0; // not connected
	lampMapping.fd = -1;
}

void connectFaceToLamp(int face, int lamp){
	lampMapping.lampIndex[face] = lamp;
	
	char* com = malloc(64);
	sprintf(com, "face %i lamp %i\n", face, lamp);
	sendCommand(com);
	free(com);
	
}
 
void setLampBrightness(int lamp, int brightness, int sendToDev){
	
	lampMapping.brightness[lamp] = brightness;
	
	if(sendToDev){
		char* com = malloc(64);
		sprintf(com, "lamp %i brightness %i\n", lamp + 1, brightness);
		sendCommand(com);
		free(com);
	}
}

void sendCommand(const char* com){
	if(lampMapping.connectedState && lampMapping.command.deviceIsReady){
		
		memset(lampMapping.command.command, 0x0, 128);
		memcpy(lampMapping.command.command, com, strlen(com));
		lampMapping.command.commandLen = strlen(com);
		
		write(lampMapping.fd, lampMapping.command.command, strlen(com));
		usleep((128) * 1000);
	}
	
}


void processLine(const char* line){
	
	printf("\tesp = %s\n", line);
	
	if(strcmp(line, "LampProgrammer") == 0){
		lampMapping.command.deviceIsReady = 1;
	}
	
	
	
}
	
struct Animation makeAnimation(const char* path){
	
	FILE* file = fopen(path, "r");
	if(!file){
		printf("failed to open\n");
	}
	
	struct Animation anim;
	memset(&anim, 0x0, sizeof(struct Animation));
	
	int c = 0;
	int numTokens = 0;
	char** tokens = malloc(sizeof(char*));
	char* tokenBuff = malloc(128);
	int tokenLen = 0;
	
	while((c = getc(file)) != EOF){
		if((c == ' ') || (c == '\n') || (c == '\t')){
			tokens = realloc(tokens, sizeof(char*) * (numTokens + 1));
			
			tokens[numTokens] = malloc(tokenLen + 1);
			memcpy(tokens[numTokens], tokenBuff, tokenLen + 1);
			tokens[numTokens][tokenLen] = '\0';
			
			memset(tokenBuff, 0x0, 128);
			tokenLen = 0;
			numTokens++;
			
			
		}else{
			tokenBuff[tokenLen] = (char)c;
			tokenLen++;
		}
	}
	
	int frameIndex = 0;
	
	for (int i = 0; i < numTokens; i++) {
		if(strcmp(tokens[i], "NAME:") == 0){
			anim.name = malloc(strlen(tokens[i + 1]) + 1);
			memcpy(anim.name, tokens[i + 1], strlen(tokens[i + 1]));
			anim.name[strlen(tokens[i + 1])] = '\0';
			continue;
		}
		
		if(strcmp(tokens[i], "FRAMES:") == 0){
			anim.numFrames = atoi(tokens[i + 1]);
			continue;
		}
		
		if(strcmp(tokens[i], "FPS:") == 0){
			anim.fps = atoi(tokens[i + 1]);
			continue;
		}
		
		if(strcmp(tokens[i], "LAMPS:") == 0){
			anim.numlamps = atoi(tokens[i + 1]);
			continue;
		}
		
		if(strcmp(tokens[i], "BEGIN:") == 0){
			anim.frames = malloc(sizeof(int*) * anim.numFrames);
			for (int i = 0; i < anim.numFrames; i++) {
				anim.frames[i] = calloc(sizeof(int) * anim.numlamps, sizeof(int));
			}
			continue;
		}
		
		if(strcmp(tokens[i], "FRAME:") == 0){
			for (int j = 0; j < anim.numlamps; j++) {
				anim.frames[frameIndex][j] = atoi(tokens[i + j + 1]);
			}
			
			frameIndex++;
			continue;
		}
		
	}
	
	for (int i = 0; i < numTokens; i++) {
		free(tokens[i]);
	}
	free(tokens);
	free(tokenBuff);
	
	return anim;
}

int sendAnimation(const struct Animation* animation){

	if( !(lampMapping.connectedState && lampMapping.command.deviceIsReady) ){
		return 0;
	}
	lampMapping.command.deviceIsReady = 0;
	
	char startCom[] = "SENDING_ANIM\n";
	write(lampMapping.fd, startCom, strlen(startCom));
	usleep((64) * 1000);
	
	char* animData = malloc(1024);
	sprintf(animData, "META NAME: %s FPS: %i FRAMES: %i LAMPS: %i\n",
			animation->name,
			animation->fps,
			animation->numFrames,
			animation->numlamps);
	
	write(lampMapping.fd, animData, strlen(animData));
	usleep((256) * 1000);
	
	char* frameData = malloc(1024);
	int frameDataPos = 0;
	
	for (int i = 0; i < animation->numFrames; i++) {
		
		memset(frameData, ' ', 1024);
		for(int j = 0; j < animation->numlamps; j++){
			frameDataPos += sprintf(frameData + frameDataPos, "%i ", animation->frames[i][j]);
		}
		frameData[frameDataPos - 1] = '\n';
		write(lampMapping.fd, frameData, strlen(frameData));
		usleep((64) * 1000);
		
		frameDataPos = 0;
		
	}
	free(frameData);
	
	char endCom[] = "ANIM_END\n";
	write(lampMapping.fd, endCom, strlen(startCom));
	usleep((64) * 1000);
	lampMapping.command.deviceIsReady = 1;
	
	free(animData);
	
	return 1;
}

void* animSim(void* arg){
	struct AnimSim* animSimArgs = arg;
	
	while (animSimArgs->isPlaying) {
		for (int f = 0; f < animSimArgs->animation->numFrames; f++) {
			for (int l = 0; l < animSimArgs->animation->numlamps; l++) {
				setLampBrightness(l, animSimArgs->animation->frames[f][l], 0);
			}
			usleep(1000000 / animSimArgs->animation->fps);
		}
	}
	
	return NULL;
}

void playAnimation(const struct Animation* animation){
	
	animSimArgs.isPlaying = 0;
	pthread_join(animationThread, NULL);
	
	animSimArgs.animation = animation;
	
	animSimArgs.isPlaying = 1;
	pthread_create(&animationThread, NULL, &animSim, &animSimArgs);
}

void stopAnimation(void){
	
	animSimArgs.isPlaying = 0;
	pthread_join(animationThread, NULL);
	
}

void* communication(void* usrArg){
	
	char* line = malloc(256);
	int lineLen = 0;
	
	while (lampMapping.connectedState) {
		
		usleep((128) * 1000);
		lampMapping.resp.responseLen = read(lampMapping.fd, lampMapping.resp.response, 256);
		
		for (int i = 0; i < lampMapping.resp.responseLen; i++) {
			if(lampMapping.resp.response[i] == '\n'){
				line[lineLen - 1] = '\0';
				processLine(line);
				
				memset(lampMapping.resp.curLine, 0x0, 256);
				lampMapping.resp.curLineLen = lineLen;
				memcpy(lampMapping.resp.curLine, line, lineLen);
				
				lineLen = 0;
				memset(line, 0x0, 256);
				
			}else{
				line[lineLen] = lampMapping.resp.response[i];
				lineLen++;
			}
			
		}
		
	}
	
	free(line);
	return NULL;
}

char* openDevice(char* path){
	
	lampMapping.fd = open(path, O_RDWR | O_NONBLOCK | O_NOCTTY | O_SYNC);
	if(lampMapping.fd < 0){
		lampMapping.connectedState = -1; // failed to connect
		return "couldnt open\n";
	}
	lampMapping.connectedState = 1;
	
	set_interface_attribs(lampMapping.fd, 115200, 0);
	set_blocking(lampMapping.fd, 0);
	
	lampMapping.resp.response = malloc(256);
	lampMapping.resp.responseLen = 0;
	lampMapping.resp.curLine = malloc(256);
	lampMapping.resp.curLineLen = 0;
	
	lampMapping.command.command = malloc(128);
	lampMapping.command.commandLen = 0;
	lampMapping.command.deviceIsReady = 0;
	
	pthread_create(&lampMapping.comThread, NULL, &communication, &lampMapping.resp);
	
	return "device opened successfully";
}

void closeDevice(void){
	
	if(lampMapping.connectedState > 0){
		lampMapping.connectedState = 0;
		lampMapping.command.deviceIsReady = 0;
		pthread_join(lampMapping.comThread, NULL);
		
		close(lampMapping.fd);
		free(lampMapping.resp.response);
	}
	
}

int set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	if (tcgetattr (fd, &tty) != 0)
	{
		//				error_message ("error %d from tcgetattr", errno);
		return -1;
	}
	
	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);
	
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
	
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	
	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;
	
	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		//				error_message ("error %d from tcsetattr", errno);
		return -1;
	}
	return 0;
}

void set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		//				error_message ("error %d from tggetattr", errno);
		return;
	}
	
	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
}


