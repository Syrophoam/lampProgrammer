//
//  Math.h
//  lampProg
//
//  Created by syro Fullerton on 26/11/2025.
//

#ifndef Math_h
#define Math_h

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


struct Vec2f{
	float x, y;
};
struct Vec3f {
	float x, y, z;
};

struct Matrix44f{
	float
	_00, _01, _02, _03,
	_10, _11, _12, _13,
	_20, _21, _22, _23,
	_30, _31, _32, _33;
};

struct Matrix44a{
	float m[4][4];
};

struct Vec3f multiplyPointByMatrix(const struct Vec3f *in, const struct Matrix44f* M);
void setMatrixIdentity(struct Matrix44f* mat);
float dotProduct(const struct Vec3f* a, const struct Vec3f* b);
struct Vec3f crossProduct(const struct Vec3f* a, const struct Vec3f* b);
void normalise(struct Vec3f* vec);
struct Vec3f subtractVectors(const struct Vec3f* a, const struct Vec3f* b);
void scale(struct Vec3f* vec, float amount);
void negate(struct Vec3f* vec);
struct Vec3f addVectors(const struct Vec3f* a, const struct Vec3f* b);
float lerp(float a, float b, float t);
float length(const struct Vec3f* vec);
struct Matrix44f multiplyMatrices(const struct Matrix44f* a, const struct Matrix44f* b);
void rotateVectorX(float angle, struct Vec3f* vec);
void rotateVectorY(float angle, struct Vec3f* vec);
void rotateVectorZ(float angle, struct Vec3f* vec);

#endif /* Math_h */
