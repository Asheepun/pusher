#include "engine/array.h"
#include "engine/geometry.h"
#include "engine/files.h"
#include "engine/3d.h"

#include "glad/wgl.h"
#include "glad/gl.h"

#include "stdio.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"

void Model_initFromFile_obj(Model *model_p, char *path){

	//read vertices and indices
	long int fileSize;
	char *data = getFileData_mustFree(path, &fileSize);

	Array vertices;
	Array faces;

	Array_init(&vertices, sizeof(Vec3f));
	Array_init(&faces, sizeof(Face));

	for(int i = 0; i < fileSize; i++){

		if(data[i] == *"v"){

			Vec3f *vertex_p = Array_addItem(&vertices);

			char *next_p;

			vertex_p->x = strtof(data + i + 1, &next_p);
			vertex_p->y = strtof(next_p + 1, &next_p);
			vertex_p->z = strtof(next_p + 1, &next_p);

		}

		if(data[i] == *"f"){

			Face *face_p = Array_addItem(&faces);

			char *next_p;

			face_p->indices[0] = strtol(data + i + 1, &next_p, 10) - 1;//-1 since it's stored as vertex's number
			face_p->indices[1] = strtol(next_p + 1, &next_p, 10) - 1;
			face_p->indices[2] = strtol(next_p + 1, &next_p, 10) - 1;
			
		}
	}

	//create mesh
	Vec3f *mesh = malloc(sizeof(Vec3f) * faces.length * 6);

	for(int i = 0; i < faces.length; i++){

		Face *face_p = Array_getItemPointerByIndex(&faces, i);

		for(int j = 0; j < 3; j++){

			Vec3f *vertex_p = Array_getItemPointerByIndex(&vertices, face_p->indices[j]);

			mesh[i * 6 + j * 2] = *vertex_p;
		
		}

		Vec3f normal = getCrossVec3f(
			getSubVec3f(mesh[i * 6 + 0], mesh[i * 6 + 2]),
			getSubVec3f(mesh[i * 6 + 0], mesh[i * 6 + 4])
		);

		for(int j = 0; j < 3; j++){
			mesh[i * 6 + j * 2 + 1] = normal;
		}
	
	}

	//load gl buffers
	glGenBuffers(1, &model_p->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, model_p->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f) * 6 * faces.length, mesh, GL_STATIC_DRAW);

	glGenVertexArrays(1, &model_p->VAO);
	glBindVertexArray(model_p->VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	model_p->numberOfTriangles = faces.length;

	//free data
	Array_free(&vertices);
	Array_free(&faces);

	free(mesh);
	free(data);

}

void GL3D_uniformMat4f(unsigned int shaderProgram, char *locationName, Mat4f m){

	unsigned int location = glGetUniformLocation(shaderProgram, locationName);

	glUniformMatrix4fv(location, 1, GL_FALSE, (float *)m.values);

}

void GL3D_uniformVec3f(unsigned int shaderProgram, char *locationName, Vec3f v){

	unsigned int location = glGetUniformLocation(shaderProgram, locationName);

	glUniform3f(location, v.x, v.y, v.z);

}
