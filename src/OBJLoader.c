//
//  OBJLoader.c
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#include "OBJLoader.h"

struct OBJ obj;
struct OBJ* getOBJ(void){ return &obj; }

int getNumPoints(int meshIndex){ return obj.meshes[meshIndex].numVerticies; }
int getNumFaces(int meshIndex) { return obj.meshes[meshIndex].numFaces; }

struct Vec3f* getPoint(int meshIndex, int vertIndex){ return &obj.meshes[meshIndex].vertices[vertIndex]; }
struct Mesh* getMesh(int index){ return &obj.meshes[index]; }
int getNumMeshes(void){ return obj.numMeshes; }

void rotateMesh(int meshIndex, float amount){
	obj.meshes[meshIndex].yRotation += amount;
}

struct Token parseTokens(const char* line, unsigned int lineLen, const char* delimiters){
	size_t numDelimiters = strlen(delimiters);
	
	int numTokens = 0;
	char** tokens = NULL;
	
	int tokenStart = 0;
	int tokenLen = 0;
	for(int i = 0; i < (lineLen); i++){
		
		int isDelimiter = 0;
		for (int j = 0; j < numDelimiters; j++) {
			isDelimiter += (line[i] == delimiters[j]);
			isDelimiter += (line[i] == '\0');
		}
		if(isDelimiter){
			numTokens++;
			tokens = realloc(tokens, sizeof(char*) * numTokens);
			tokens[numTokens - 1] = malloc(tokenLen + 1);
			
			memcpy(tokens[numTokens - 1], line + tokenStart, tokenLen);
			tokens[numTokens - 1][tokenLen] = '\0';
			
			tokenStart = i + 1;
			tokenLen = 0;
		}else{
			tokenLen++;
		}
		
	}
	struct Token ret;
	ret.tokens = tokens;
	ret.numTokens = numTokens;
	
	return ret;
}

void freeTokens(struct Token* tokens){
	for (int i = 0; i < tokens->numTokens; i++) {
		free(tokens->tokens[i]);
	}
	free(tokens->tokens);
}

void setOrigins(struct OBJ* obj){
	
	for (int i = 0; i < obj->numMeshes; i++) {
		struct Vec3f centerOfMass = {0,0,0};
		
		for (int j = 0; j < obj->meshes[i].numVerticies; j++) {
			centerOfMass.x += obj->meshes[i].vertices[j].x;
			centerOfMass.y += obj->meshes[i].vertices[j].y;
			centerOfMass.z += obj->meshes[i].vertices[j].z;
		}
		centerOfMass.x /= (float)obj->meshes[i].numVerticies;
		centerOfMass.y /= (float)obj->meshes[i].numVerticies;
		centerOfMass.z /= (float)obj->meshes[i].numVerticies;
		
		obj->meshes[i].origin = centerOfMass;
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
	int currentNumVerts = 0;
	char* tmp = malloc(256);
	while (1) {
		
		memset(tmp, 0x0, 256);
		int lineLen = 0;
		int c;
		
		while ((c = fgetc(obj.file)) != EOF) {
			lineLen++;
			if(lineLen == 256){
				printf("line too long :(\n");
				return -1;
			}
			
			if(c == '\n'){
				break;
			}
			tmp[lineLen - 1] = c;
		}
		
		if(c == EOF){ // finished parsing
			
			setOrigins(&obj);
			
			fclose(obj.file);
			free(tmp);
			return 1;
		}
		
		struct Token t = parseTokens(tmp, lineLen, " ");
		
		if(t.tokens[0][0] == 'o'){ // allocate new mesh
			
			if(obj.numMeshes > 0)
				currentNumVerts += obj.meshes[obj.numMeshes - 1].numVerticies;
			
			obj.numMeshes++;
			obj.meshes = realloc(obj.meshes, sizeof(struct Mesh) * obj.numMeshes);
			struct Mesh* mesh = &obj.meshes[obj.numMeshes - 1];
			memset(mesh, 0x0, sizeof(struct Mesh));
			
			mesh->name = malloc(strlen(t.tokens[1]));
			memcpy(mesh->name, t.tokens[1], strlen(t.tokens[1]));
			
			
			printf("Mesh: %s Loading\n", mesh->name);
			
		}
		
		if(strcmp(t.tokens[0], "v") == 0){
			struct Mesh* mesh = &obj.meshes[obj.numMeshes - 1];
			mesh->numVerticies++;
			mesh->vertices = realloc(mesh->vertices, sizeof(struct Vec3f) * mesh->numVerticies);
			
			mesh->vertices[mesh->numVerticies - 1].x = atof(t.tokens[1]);
			mesh->vertices[mesh->numVerticies - 1].y = atof(t.tokens[2]);
			mesh->vertices[mesh->numVerticies - 1].z = atof(t.tokens[3]);
			
		}
		if(strcmp(t.tokens[0], "vn") == 0){
			struct Mesh* mesh = &obj.meshes[obj.numMeshes - 1];
			mesh->numNormals++;
			mesh->normals = realloc(mesh->normals, sizeof(struct Vec3f) * mesh->numNormals);
			
			mesh->normals[mesh->numNormals - 1].x = atof(t.tokens[1]);
			mesh->normals[mesh->numNormals - 1].y = atof(t.tokens[2]);
			mesh->normals[mesh->numNormals - 1].z = atof(t.tokens[3]);
			
		}
		
		if(strcmp(t.tokens[0], "f") == 0){ // face indicies
			
			struct Mesh* mesh = &obj.meshes[obj.numMeshes - 1];
			
			mesh->numFaces++;
			mesh->vertIndicies = realloc(mesh->vertIndicies, sizeof(int*) * mesh->numFaces);
			mesh->normIndicies = realloc(mesh->normIndicies, sizeof(int*) * mesh->numFaces);
			mesh->numVertsInFace = realloc(mesh->numVertsInFace, sizeof(int) * mesh->numFaces);
			
			int numVerts = t.numTokens - 1;
			mesh->numVertsInFace[mesh->numFaces - 1] = numVerts;
			
			mesh->vertIndicies[mesh->numFaces - 1] = malloc(sizeof(int) * numVerts);
			mesh->normIndicies[mesh->numFaces - 1] = malloc(sizeof(int) * numVerts);
			
			
			for (int i = 1; i < t.numTokens; i++) {
				struct Token Indicies = parseTokens(t.tokens[i], strlen(t.tokens[i]) + 1, "/");
				
				mesh->vertIndicies[mesh->numFaces - 1][i - 1] = atoi(Indicies.tokens[0]) - 1 - currentNumVerts;
				mesh->normIndicies[mesh->numFaces - 1][i - 1] = atoi(Indicies.tokens[2]) - 1 - currentNumVerts;
				
				freeTokens(&Indicies);
			}
			
		}
		
		freeTokens(&t);
		
	}
	
	
	return 0;
}

void destroyMesh(struct Mesh* mesh){
	free(mesh->name);
	mesh->name = NULL;
	
	
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
