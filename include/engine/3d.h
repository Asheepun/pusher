#ifndef H3D_H_
#define H3D_H_

#include "engine/geometry.h"

#include "glad/wgl.h"
#include "glad/gl.h"

typedef struct Model{
	unsigned int VBO;
	unsigned int VAO;
	unsigned int numberOfTriangles;
}Model;

Vec3f *getMeshDataFromFile_obj(char *, int *);

void Model_initFromFile_obj(Model *, char *);

void GL3D_uniformMat4f(unsigned int, char *, Mat4f);

void GL3D_uniformVec3f(unsigned int, char *, Vec3f);

void GL3D_uniformVec4f(unsigned int, char *, Vec4f);

void GL3D_uniformInt(unsigned int, char *, int);

#endif
