#ifndef H3D_H_
#define H3D_H_

#include "engine/geometry.h"

#include "glad/wgl.h"
#include "glad/gl.h"

typedef struct Face{
	unsigned int indices[4];
}Face;

typedef struct Model{
	unsigned int VBO;
	unsigned int VAO;
	unsigned int numberOfTriangles;
}Model;

void Model_initFromFile_obj(Model *, char *, int);

void GL3D_uniformMat4f(unsigned int, char *, Mat4f);

void GL3D_uniformVec3f(unsigned int, char *, Vec3f);

void GL3D_uniformInt(unsigned int, char *, int);

#endif
