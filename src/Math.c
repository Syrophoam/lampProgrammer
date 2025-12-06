//
//  math.c
//  x11Render-run
//
//  Created by syro Fullerton on 07/11/2025.
//

#include "Math.h"

struct Vec3f multiplyPointByMatrix(const struct Vec3f *in, const struct Matrix44f* M)
{
	struct Vec3f res;
	res.x   = in->x * M->_00 + in->y * M->_10 + in->z * M->_20 + M->_30;
	res.y   = in->x * M->_01 + in->y * M->_11 + in->z * M->_21 + M->_31;
	res.z   = in->x * M->_02 + in->y * M->_12 + in->z * M->_22 + M->_32;
	float w = in->x * M->_03 + in->y * M->_13 + in->z * M->_23 + M->_33;
	
	// normalize if w is different than 1 (convert from homogeneous to Cartesian coordinates)
	if (w != 1) {
		res.x /= w;
		res.y /= w;
		res.z /= w;
	}
	return res;
}


void setMatrixIdentity(struct Matrix44f* mat){
	mat->_00 = 1;
	mat->_11 = 1;
	mat->_22 = 1;
	mat->_33 = 1;
}

float dotProduct(const struct Vec3f* a, const struct Vec3f* b){
	float sum = 0;
	sum += a->x * b->x;
	sum += a->y * b->y;
	sum += a->z * b->z;
	return sum;
}

struct Vec3f crossProduct(const struct Vec3f* a, const struct Vec3f* b){
	struct Vec3f ret;
	ret.x = a->y * b->z - a->z * b->y;
	ret.y = a->x * b->z - a->z * b->x;
	ret.z = a->x * b->y - a->y * b->x;
	return ret;
}

void normalise(struct Vec3f* vec){
	float mag = length(vec);
	vec->x /= mag;
	vec->y /= mag;
	vec->z /= mag;
}
struct Vec3f subtractVectors(const struct Vec3f* a, const struct Vec3f* b){
	struct Vec3f ret;
	ret.x = a->x - b->x;
	ret.y = a->y - b->y;
	ret.z = a->z - b->z;
	return ret;
}

void scale(struct Vec3f* vec, float amount){
	vec->x *= amount;
	vec->y *= amount;
	vec->z *= amount;
}

void negate(struct Vec3f* vec){
	vec->x = -vec->x;
	vec->y = -vec->y;
	vec->z = -vec->z;
}

struct Vec3f addVectors(const struct Vec3f* a, const struct Vec3f* b){
	struct Vec3f res;
	res.x = a->x + b->x;
	res.y = a->y + b->y;
	res.z = a->z + b->z;
	return res;
}

float lerp(float a, float b, float t) {
	return a + (b - a) * t;
}

float length(const struct Vec3f* vec){
	return sqrt(pow(vec->x,2.) + pow(vec->y, 2.) + pow(vec->z, 2.));
}

struct Matrix44f multiplyMatrices(const struct Matrix44f* a, const struct Matrix44f* b){
	struct Matrix44f ret;
	
	struct Matrix44a* reta = (struct Matrix44a*)&ret;
	const struct Matrix44a* aa = (struct Matrix44a*)a;
	const struct Matrix44a* ba = (struct Matrix44a*)b;
	
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			
			reta->m[i][j] = 0;
			for (int k = 0; k < 4; k++) {
				reta->m[i][j] += aa->m[i][k] * ba->m[k][j];
			}
			
		}
	}
	
	return ret;
}

void rotateVectorX(float angle, struct Vec3f* vec){
 
	struct Matrix44f mat;
	memset(&mat, 0x0, sizeof(struct Matrix44f)); 

	mat._00 = 1;
	mat._11 = cos(angle);
	mat._12 = -sin(angle);
	mat._21 = sin(angle);
	mat._22 = cos(angle);
	mat._33 = 1;
	
	*vec = multiplyPointByMatrix(vec, &mat);
}

void rotateVectorY(float angle, struct Vec3f* vec){

	struct Matrix44f mat;
	memset(&mat, 0x0, sizeof(struct Matrix44f));

	mat._00 = cos(angle);
	mat._20 = -sin(angle);
	mat._11 = 1;
	mat._02 = sin(angle);
	mat._22 = cos(angle);
	mat._33 = 1;
	
	*vec = multiplyPointByMatrix(vec, &mat);
}

void rotateVectorZ(float angle, struct Vec3f* vec){

	struct Matrix44f mat;
	memset(&mat, 0x0, sizeof(struct Matrix44f));

	mat._00 = cos(angle);
	mat._10 = -sin(angle);
	mat._01 = sin(angle);
	mat._11 = cos(angle);
	mat._22 = 1;
	mat._33 = 1;
	
	*vec = multiplyPointByMatrix(vec, &mat);
}
