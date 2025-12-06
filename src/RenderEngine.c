//
//  RenderEngine.c
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#include "RenderEngine.h"


void initEngine(void){
	
	memset(&cam.worldToCam, 0x0, sizeof(struct Matrix44f));
	setMatrixIdentity(&cam.worldToCam);
	
	cam.worldToCam._30 = 0; // x ?
	cam.worldToCam._31 = 0; // y
	cam.worldToCam._32 = -3; // z
	
	cam.far = 100;
	cam.near = 0.01;
	cam.angleOfView = 90;
	cam.zoom = 2;
	
	setProjectionMatrix(cam.angleOfView, cam.near, cam.far, &cam.Mproj);
	
}

struct Vec3f getCamPos(void){
	struct Vec3f ret;
	ret.x = cam.worldToCam._30;
	ret.y = cam.worldToCam._31;
	ret.z = cam.worldToCam._32;
	return ret;
}

void moveCam(float xAmount, float yAmount, float zAmount){
	
	cam.worldToCam._30 += xAmount;
	cam.worldToCam._31 += yAmount;
	cam.worldToCam._32 += zAmount;
	
	setProjectionMatrix(cam.angleOfView, cam.near, cam.far, &cam.Mproj);
}

void setCam(float x, float y, float z){
	cam.worldToCam._30 = x;
	cam.worldToCam._31 = y;
	cam.worldToCam._32 = z;
	setProjectionMatrix(cam.angleOfView, cam.near, cam.far, &cam.Mproj);
}

void setProjectionMatrix(float angleOfView, float near, float far, struct Matrix44f* M)
{
	// set the basic projection matrix
	float scale = 1.f / tan(angleOfView * 0.5 * M_PI / 180.f);
	M->_00 = scale;  // scale the x coordinates of the projected point
	M->_11 = scale;  // scale the y coordinates of the projected point
	M->_22 = -far / (far - near);  // used to remap z to [0,1]
	M->_32 = -(far * near / (far - near));  // used to remap z [0,1]
	M->_23 = -1;  // set w = -z
	M->_33 = 0;
	
}

void zoom(float amount){
	cam.zoom += amount;
}


struct Vec3f projectPoint(const struct Vec3f* point, int windowWidth, int windowHeight, int* pointIsVisible, const struct Mesh* mesh){
	struct Vec3f res; // including depth
	
	struct Vec3f rotPoint = *point;
	rotateVectorY(mesh->yRotation, &rotPoint);
	
	struct Vec3f vertCam = multiplyPointByMatrix(&rotPoint, &cam.worldToCam);
	
	if (-vertCam.z <= cam.near) { // vertCam is the point *before* projection
		*pointIsVisible = 0;
	} else {
		*pointIsVisible = 1;
	}
	
	struct Vec3f projectedVert = multiplyPointByMatrix(&vertCam, &cam.Mproj);
	
	projectedVert.x *= cam.zoom;
	projectedVert.y *= cam.zoom;
	
	res.x = (((projectedVert.x + ((float)windowWidth / (float)windowHeight)) * 0.5) * windowHeight);// + ((windowHeight) / 2);
	res.y = (1 - (projectedVert.y + 1) * 0.5) * windowHeight;
	res.z = projectedVert.z;
	
	return res;
}

int comp(void* c, const void* a, const void* b){
	
	float* zDepths = (float*)c;
	
	int i = *(const int*)a;
	int j = *(const int*)b;
	
	if(zDepths[i] < zDepths[j]) { return  1; }
	if(zDepths[i] > zDepths[j]) { return -1; }
	
	return 0;
}



void drawProjection(const struct Vec3f* points, int* pointIsVisible, const struct Mesh* mesh, const struct X* x, struct LampMapping* lampMapping){
	
	float* faceDepths = malloc(sizeof(float) * getNumFaces(0));
	struct Vec2f* facePositions = malloc(sizeof(struct Vec2f) * getNumFaces(0));
	XTextItem faceLabels[mesh->numFaces];
	
	for (int f = 0; f < mesh->numFaces; f++) {
		
		float avgZ = 0;
		float avgX = 0; float avgY = 0;
		
		int numV = mesh->numVertsInFaces[f];
		for (int v = 0; v < numV; v++) {
			avgZ += points[mesh->vertIndicies[f][v]].z;
			avgX += points[mesh->vertIndicies[f][v]].x;
			avgY += points[mesh->vertIndicies[f][v]].y;
		}
		avgZ /= (float)numV;
		avgX /= (float)numV;
		avgY /= (float)numV;
		
		faceDepths[f] = avgZ;
		facePositions[f].x = avgX;
		facePositions[f].y = avgY + 5;
		
		memset(&faceLabels[f], 0x0, sizeof(XTextItem));
		faceLabels[f].chars = malloc(2);
		sprintf(faceLabels[f].chars, "%i", f + 1);
		faceLabels[f].nchars = ((f + 1) < 10) ? 1 : 2;
		
		}
		
	int* zSortedFaceIndices = malloc(sizeof(int) * getNumFaces(0));
	for (int i = 0; i < getNumFaces(0); i++) {
		zSortedFaceIndices[i] = i;
	}
	
	qsort_r(zSortedFaceIndices, mesh->numFaces, sizeof(int), faceDepths, comp);
	
	free(faceDepths);
	
	for (int face = 0; face < mesh->numFaces; face++) {
		int sortedFace = zSortedFaceIndices[face];
		
		int numVerts = mesh->numVertsInFaces[sortedFace];
		int drawFace = 0;
		for (int i = 0; i < numVerts; i++) {
			drawFace += pointIsVisible[mesh->vertIndicies[sortedFace][i]];
		}
		
		XPoint* faceCorners = malloc(sizeof(XPoint) * numVerts);
		
		for (int vert = 0; vert < numVerts; vert++) {
			
			float p1x = points[mesh->vertIndicies[sortedFace][vert]].x;
			float p1y = points[mesh->vertIndicies[sortedFace][vert]].y;
			
			float p2x = points[mesh->vertIndicies[sortedFace][ (vert + 1) % numVerts]].x;
			float p2y = points[mesh->vertIndicies[sortedFace][ (vert + 1) % numVerts]].y;
			
			
			if(drawFace == numVerts){
				XSetForeground(x->display, x->gc, x->white);
				
				if((sortedFace == 19) && (vert == 2)){
					XSetLineAttributes(x->display, x->gc, 6, LineOnOffDash, CapButt, JoinMiter);
				}else{
					XSetLineAttributes(x->display, x->gc, 2, LineSolid, CapButt, JoinMiter);
				}
				XDrawLine(x->display, x->window, x->gc,
						  p1x, p1y,
						  p2x, p2y);
			}
			
			
			faceCorners[vert].x = p1x;
			faceCorners[vert].y = p1y;
		}
		
		if (drawFace == numVerts) {
			
			int brightness = lampMapping->brightness[sortedFace];
			
			XSetForeground(x->display, x->gc, x->greyScale[brightness].pixel);
			
			XFillPolygon(x->display, x->window, x->gc,
						 faceCorners,
						 numVerts,
						 Convex,
						 FillSolid);
		}
		
		free(faceCorners);
	}
	
	
	for (int i = 10; i < 20; i++) {
		
		int faceIndex = zSortedFaceIndices[i];
		
		if(lampMapping->brightness[faceIndex] < 8){
			XSetForeground(x->display, x->gc, x->white);
		}else{
			XSetForeground(x->display, x->gc, x->black);
		}
		
		XDrawText(x->display, x->window, x->gc,
				  (int)facePositions[faceIndex].x, (int)facePositions[faceIndex].y,
				  &(faceLabels[faceIndex]), 1);
	}
	
	
	free(zSortedFaceIndices);
	free(facePositions);
	for (int i = 0; i < 20; i++) {
		free(faceLabels[i].chars);
	}
}
