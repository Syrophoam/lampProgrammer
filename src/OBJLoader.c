//
//  OBJLoader.c
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#include "OBJLoader.h"

int getNumPoints(int meshIndex){ return obj.meshes[meshIndex].numVerticies; }
int getNumFaces(int meshIndex) { return obj.meshes[meshIndex].numFaces; }

struct Vec3f* getPoint(int meshIndex, int vertIndex){ return &obj.meshes[meshIndex].vertices[vertIndex]; }
struct Mesh* getMesh(int index){ return &obj.meshes[index]; }

void rotateMesh(int meshIndex, float amount){
	obj.meshes[meshIndex].yRotation += amount;
}

void parseVector(const char* line, int offset, struct Vec3f* vec){
	char* flt = malloc(16); // values from blender are ~8 long
	int idxInLine = 2 + offset;
	
	for(int i = 0; i < 3; i++){ // .obj supports 4 dimensions, skipping it
		int fltIndex = 0;
		memset(flt, 0, 16);
		while (line[idxInLine] != ' ') {
			flt[fltIndex++] = line[idxInLine];
			
			idxInLine++;
		}
		idxInLine++;
		float val = atof(flt); 
		
		if(i == 0)
			vec->x = val;
		else if(i == 1)
			vec->y = val;
		else if (i == 2) 
			vec->z = val;
	}
	free(flt);

}

int getNumVertsInFace(const char* line){
	
	int pos = 0;
	int numVerts = 0;
	while (line[pos] != '\n') {
		if(line[pos] == ' ')
			numVerts++;
		pos++;
	}
	return numVerts;
}

void parseFaceIndexList(const char* line, int numVerticies, int* vertIndicies, int* normIndicies){
	
	int indexInLine = 2;
	char* indexStr = malloc(8); // absurdly large buffer
	
	for (int i = 0; i < numVerticies; i++) {
		
		int index = 0;
		
		while (line[indexInLine] != '/') {
			indexStr[index++] = line[indexInLine++];
		}
		
		vertIndicies[i] = atoi(indexStr) - 1; // dumbass .obj start index = 1 BOOOO
		memset(indexStr, 0, 8);
		index = 0;
		
		indexInLine += 2; // increment to normal index, assuming no texture coords
		
		while (line[indexInLine] != ' ') {
			indexStr[index++] = line[indexInLine++];
		}
		
		normIndicies[i] = atoi(indexStr) - 1;
		
		memset(indexStr, 0, 8);
		index = 0;
		
		indexInLine ++; // increment to next index
	}
	
}

int loadFile(const char* path){
	
	obj.filePath = path;
	obj.file = fopen(path, "r");
	if(obj.file == NULL){
		printf("couldn't open file");
		exit(-1);
		return -1;
	}
	
	obj.numMeshes = 0;
	obj.meshes = NULL;
	
	while (1) {
		
		size_t lineLen;
		char* tmp = fgetln(obj.file, &lineLen);
		
		if(tmp == NULL){ // finished parsing
			fclose(obj.file);
			return 1;
		}
		
		if(tmp[0] == 'o'){ // allocate new mesh
			obj.numMeshes++;
			obj.meshes = realloc(obj.meshes, sizeof(struct Mesh) * obj.numMeshes);
			struct Mesh* mesh = &obj.meshes[obj.numMeshes - 1];
			
			mesh->vertices = NULL;
			mesh->normals = NULL;
			
			mesh->normIndicies = NULL;
			mesh->vertIndicies = NULL;
			
			mesh->numVerticies = 0;
			mesh->numNormals = 0;
			mesh->numFaces = 0;
			mesh->numVertsInFaces = NULL;
			
			mesh->xRotation = 0; mesh->yRotation = 0; mesh->zRotation = 0;
			mesh->xTranslation = 0; mesh->yTranslation = 0; mesh->zTranslation = 0;
			
			mesh->name = malloc(lineLen);
			memcpy(mesh->name, tmp + 2, lineLen - 3);
			
			printf("Mesh: %s Loading\n", mesh->name);
			
		}
		
		if(tmp[0] == 'v'){ // add new vertex
			
			if(tmp[1] == ' '){ // vertex position
				
				struct Mesh* mesh = &obj.meshes[obj.numMeshes - 1];
				if(mesh == NULL){
					printf("attempting to index vertex without mesh");
					exit(-1);
				}
				
				mesh->numVerticies++;
				mesh->vertices = realloc(mesh->vertices, sizeof(struct Vec3f) * mesh->numVerticies);
				
				parseVector(tmp, 0, &mesh->vertices[mesh->numVerticies - 1] );
				
			}else if(tmp[1] == 'n'){ // vertex normal
				
				struct Mesh* mesh = &obj.meshes[obj.numMeshes - 1];
				if(mesh == NULL){
					printf("attempting to index vertex without mesh");
					exit(-1);
				}
				
				mesh->numNormals++;
				mesh->normals = realloc(mesh->normals, sizeof(struct Vec3f) * mesh->numNormals);
				
				parseVector(tmp, 1, &mesh->normals[mesh->numNormals - 1]);
				
			}
			
		}
		
		if(tmp[0] == 'f'){ // face indicies
			
			struct Mesh* mesh = &obj.meshes[obj.numMeshes - 1];
			if(mesh == NULL){
				printf("attempting to index vertex without mesh");
				exit(-1);
			}
			
			mesh->numFaces++;
			mesh->numVertsInFaces = realloc(mesh->numVertsInFaces, sizeof(int) * mesh->numFaces);
			mesh->vertIndicies = realloc(mesh->vertIndicies, sizeof(int*) * mesh->numFaces);
			mesh->normIndicies = realloc(mesh->normIndicies, sizeof(int*) * mesh->numFaces);
			
			int numVertsInFace = getNumVertsInFace(tmp);
			mesh->numVertsInFaces[mesh->numFaces - 1] = numVertsInFace;
			mesh->vertIndicies[mesh->numFaces - 1] = malloc(sizeof(int) * numVertsInFace);
			mesh->normIndicies[mesh->numFaces - 1] = malloc(sizeof(int) * numVertsInFace);
			
			parseFaceIndexList(tmp,
							   numVertsInFace,
							   mesh->vertIndicies[mesh->numFaces - 1],
							   mesh->normIndicies[mesh->numFaces - 1]);
			
			
		}
		
		
	}
	
	
	return 0;
}

void destroyMesh(struct Mesh* mesh){
	free(mesh->name);
	mesh->name = NULL;
	
	free(mesh->numVertsInFaces);
	mesh->numVertsInFaces = NULL;
	
	free(mesh->vertices);
	mesh->vertices = NULL;
	
	free(mesh->normals);
	mesh->normals = NULL;
	
	for (int face = 0; face < mesh->numFaces; face++) {
		
		free(mesh->vertIndicies[face]);
		mesh->vertIndicies[face] = NULL;
		
		free(mesh->normIndicies[face]);
		mesh->normIndicies[face] = NULL;
	}
	
	free(mesh);
	mesh = NULL;
}
