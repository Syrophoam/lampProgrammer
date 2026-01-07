//
//  OBJLoader.h
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#ifndef OBJLoader_h
#define OBJLoader_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Math.h"

struct OBJ {
	const char* filePath;
	FILE* file;
	struct Mesh* meshes;
	int numMeshes;
};

struct Token{
	char** tokens;
	int numTokens;
};

struct Mesh{
	char* name;
	
	struct Vec3f origin;
	 
	struct Vec3f* vertices;
	struct Vec3f* normals;
	
	int** vertIndicies;
	int** normIndicies;
	int numVerticies;
	int numNormals;
	int numFaces;
	int* numVertsInFace;
	
	float xRotation, yRotation, zRotation;
	float xTranslation, yTranslation, zTranslation;
};

int getNumPoints(int meshIndex);
int getNumFaces(int meshIndex);
int getNumMeshes(void);
struct Vec3f* getPoint(int meshIndex, int vertIndex);
struct Mesh* getMesh(int index);
void parseVector(const char* line, int offset, struct Vec3f* vec);
int getNumVertsInFace(const char* line);
void parseFaceIndexList(const char* line, int numVerticies, int* vertIndicies, int* normIndicies, int currentNumVerts);
struct Token parseTokens(const char* line, unsigned int lineLen, const char* delimiters);
int loadFile(const char* path);
void destroyMesh(struct Mesh* mesh);
void rotateMesh(int meshIndex, float amount);

struct OBJ* getOBJ(void); 

#endif /* OBJLoader_h */
