//
//  RenderEngine.h
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#ifndef RenderEngine_h
#define RenderEngine_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Math.h" 

#include "OBJLoader.h"

#include "lampProgrammer.h"
#include "X11.h"


struct Cam {
	struct Matrix44f worldToCam;
	float angleOfView;
	float near;
	float far;
	struct Matrix44f Mproj;
	float zoom;
};

void initEngine(void);  
struct Vec3f getCamPos(void);
void moveCam(float xAmount, float yAmount, float zAmount);
void setCam(float x, float y, float z);
void setProjectionMatrix(float angleOfView, float near, float far, struct Matrix44f* M);
struct Vec3f projectPoint(const struct Vec3f* point, int windowWidth, int windowHeight, const struct Mesh* mesh);
void drawProjection(const struct Vec3f* points, const struct Mesh* mesh, const struct X* x, const struct LampInfo* lampInfo);
void zoom(float amount);

#endif /* RenderEngine_h */
