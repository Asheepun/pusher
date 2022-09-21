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

typedef struct Face{
	unsigned int indices[4];
}Face;

Vec3f *getMeshDataFromFile_obj(char *path, int *numberOfTrianglesOut_p){

	//read vertices and indices
	long int fileSize;
	char *data = getFileData_mustFree(path, &fileSize);

	Array vertices;
	Array faces;

	Array_init(&vertices, sizeof(Vec3f));
	Array_init(&faces, sizeof(Face));

	for(int i = 0; i < fileSize; i++){

		if(i == 0
		|| data[i - 1] == *"\n"){

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
				strtol(next_p + 1, &next_p, 10);
				strtol(next_p + 1, &next_p, 10);
				face_p->indices[1] = strtol(next_p + 1, &next_p, 10) - 1;
				strtol(next_p + 1, &next_p, 10);
				strtol(next_p + 1, &next_p, 10);
				face_p->indices[2] = strtol(next_p + 1, &next_p, 10) - 1;
				strtol(next_p + 1, &next_p, 10);
				strtol(next_p + 1, &next_p, 10);

				char *lastNext_p = next_p + 1;
				strtol(next_p + 1, &next_p, 10);

				if(lastNext_p != next_p){

					Face *face2_p = Array_addItem(&faces);

					face2_p->indices[2] = strtol(data + i + 1, &next_p, 10) - 1;
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);

					face2_p->indices[0] = strtol(next_p + 1, &next_p, 10) - 1;
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);
					face2_p->indices[1] = strtol(next_p + 1, &next_p, 10) - 1;

				}

			}
		
		}
	}

	//create mesh
	Vec3f *mesh;

	mesh = malloc(sizeof(Vec3f) * faces.length * 3);

	for(int i = 0; i < faces.length; i++){

		Face *face_p = Array_getItemPointerByIndex(&faces, i);

		for(int j = 0; j < 3; j++){

			Vec3f *vertex_p = Array_getItemPointerByIndex(&vertices, face_p->indices[j]);

			mesh[i * 3 + j] = *vertex_p;
		
		}
	
	}

	//free data
	Array_free(&vertices);
	Array_free(&faces);
	free(data);

	//return output
	*numberOfTrianglesOut_p = faces.length;

	/*
	printf("\n");
	for(int i = 0; i < faces.length * 3; i++){
		if(i % 3 == 0){
			printf("%i\n", i / 3);
		}
		Vec3f_log(mesh[i]);
	}
	printf("\n");
	*/

	return mesh;

}

void Model_initFromFile_obj(Model *model_p, char *path){

	//read vertices and indices
	long int fileSize;
	char *data = getFileData_mustFree(path, &fileSize);

	Array vertices;
	Array faces;

	Array_init(&vertices, sizeof(Vec3f));
	Array_init(&faces, sizeof(Face));

	for(int i = 0; i < fileSize; i++){

		if(i == 0
		|| data[i - 1] == *"\n"){

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
				strtol(next_p + 1, &next_p, 10);
				strtol(next_p + 1, &next_p, 10);
				face_p->indices[1] = strtol(next_p + 1, &next_p, 10) - 1;
				strtol(next_p + 1, &next_p, 10);
				strtol(next_p + 1, &next_p, 10);
				face_p->indices[2] = strtol(next_p + 1, &next_p, 10) - 1;
				strtol(next_p + 1, &next_p, 10);
				strtol(next_p + 1, &next_p, 10);

				char *lastNext_p = next_p + 1;
				strtol(next_p + 1, &next_p, 10);

				if(lastNext_p != next_p){

					Face *face2_p = Array_addItem(&faces);

					face2_p->indices[2] = strtol(data + i + 1, &next_p, 10) - 1;
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);

					face2_p->indices[0] = strtol(next_p + 1, &next_p, 10) - 1;
					strtol(next_p + 1, &next_p, 10);
					strtol(next_p + 1, &next_p, 10);
					face2_p->indices[1] = strtol(next_p + 1, &next_p, 10) - 1;

				}

			}
		
		}
	}

	//create mesh
	Vec3f *mesh;

	model_p->numberOfTriangles = faces.length;

	mesh = malloc(sizeof(Vec3f) * model_p->numberOfTriangles * 6);

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

void GL3D_uniformVec4f(unsigned int shaderProgram, char *locationName, Vec4f v){

	unsigned int location = glGetUniformLocation(shaderProgram, locationName);

	glUniform4f(location, v.x, v.y, v.z, v.w);

}

void GL3D_uniformInt(unsigned int shaderProgram, char *locationName, int x){

	unsigned int location = glGetUniformLocation(shaderProgram, locationName);

	glUniform1i(location, x);

}
