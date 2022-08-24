#include "engine/engine.h"
#include "engine/array.h"
#include "engine/files.h"
#include "engine/shaders.h"

#include "glad/wgl.h"
#include "glad/gl.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

int WIDTH = 1920;
int HEIGHT = 1080;

float triangleVertices[] = {
	-1.0, -1.0, 0.0, 	0.0, 0.0, 1.0,
	0.0, 1.0, 0.0, 		0.0, 0.0, 1.0,
	1.0, -1.0, 0.0, 	0.0, 0.0, 1.0,
};

typedef struct Face{
	unsigned int indices[3];
}Face;

typedef struct Entity{
	float scale;
	Vec3f pos;
	Vec3f rotation;
}Entity;

unsigned int triangleVBO;

unsigned int VBO;
unsigned int VAO;

unsigned int shadowMapFBO;
unsigned int shadowMapTexture;
unsigned int SHADOW_MAP_WIDTH = 1920;
unsigned int SHADOW_MAP_HEIGHT = 1080;

unsigned int shaderProgram;
unsigned int shadowMapShaderProgram;

int numberOfTriangles;

float fov = M_PI / 4;
float aspectRatio = 16 / 9;
float far = 100.0;
float near = 0.1;

Vec3f cameraPos;
Vec3f cameraRotation;
Vec3f cameraDirection;
Vec3f cameraUp;
Vec3f cameraRight;

Vec3f lightPos;
Vec3f lightDirection;

Array entities;

void Engine_start(){

	Engine_setWindowSize(WIDTH / 2, HEIGHT / 2);

	Engine_centerWindow();

	//read model
	{
		long int fileSize;
		char *data = getFileData_mustFree("assets/models/teapot.obj", &fileSize);

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

		Array_free(&vertices);
		Array_free(&faces);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f) * 6 * faces.length, mesh, GL_STATIC_DRAW);

		numberOfTriangles = faces.length;

		free(mesh);
		free(data);

	}

	/*
	//setup triangle mesh
	glGenBuffers(1, &triangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	*/

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	//generate shadow map
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  

	{
		unsigned int vertexShader = getCompiledShader("shaders/vertex-shader.glsl", GL_VERTEX_SHADER);
		unsigned int fragmentShader = getCompiledShader("shaders/fragment-shader.glsl", GL_FRAGMENT_SHADER);

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
	}
	{
		unsigned int vertexShader = getCompiledShader("shaders/shadow-map-vertex-shader.glsl", GL_VERTEX_SHADER);
		unsigned int fragmentShader = getCompiledShader("shaders/shadow-map-fragment-shader.glsl", GL_FRAGMENT_SHADER);

		shadowMapShaderProgram = glCreateProgram();
		glAttachShader(shadowMapShaderProgram, vertexShader);
		glAttachShader(shadowMapShaderProgram, fragmentShader);
		glLinkProgram(shadowMapShaderProgram);
	}

	Array_init(&entities, sizeof(Entity));

	/*
	{
		Entity *entity_p = Array_addItem(&entities);
	
		entity_p->pos = getVec3f(0.0, 0.0, 0.0);
		entity_p->rotation = getVec3f(0.0, 0.0, 0.0);
		entity_p->scale = 1.0;
	}
	*/
	
	{
		Entity *entity_p = Array_addItem(&entities);
	
		entity_p->pos = getVec3f(0.0, 0.0, 5.0);
		entity_p->rotation = getVec3f(0.0, 0.0, 0.0);
		entity_p->scale = 0.2;
	}
	
	{
		Entity *entity_p = Array_addItem(&entities);
	
		entity_p->pos = getVec3f(0.0, 0.0, 10.0);
		entity_p->rotation = getVec3f(0.0, 0.0, 0.0);
		entity_p->scale = 0.5;
	}

	cameraPos = getVec3f(0, 0, 0);
	cameraRotation = getVec3f(M_PI / 2, 0, 0);

	lightPos = getVec3f(0, 0, 0);

}

float time = 0;

void Engine_update(float deltaTime){

	time++;

	if(Engine_keys[ENGINE_KEY_Q].down){
		Engine_quit();
	}

	if(Engine_keys[ENGINE_KEY_LEFT].down){
		cameraRotation.x += 0.01;
	}
	if(Engine_keys[ENGINE_KEY_RIGHT].down){
		cameraRotation.x -= 0.01;
	}
	if(Engine_keys[ENGINE_KEY_UP].down){
		cameraRotation.y += 0.01;
	}
	if(Engine_keys[ENGINE_KEY_DOWN].down){
		cameraRotation.y -= 0.01;
	}

	if(cameraRotation.y > M_PI / 2 - 0.01){
		cameraRotation.y = M_PI / 2 - 0.01;
	}
	if(cameraRotation.y < -(M_PI / 2 - 0.01)){
		cameraRotation.y = -(M_PI / 2 - 0.01);
	}

	cameraDirection.y = sin(cameraRotation.y);
	cameraDirection.x = cos(cameraRotation.x) * cos(cameraRotation.y);
	cameraDirection.z = sin(cameraRotation.x) * cos(cameraRotation.y);
	Vec3f_normalize(&cameraDirection);

	cameraRight = getCrossVec3f(getVec3f(0.0, 1.0, 0.0), cameraDirection);
	cameraUp = getCrossVec3f(cameraDirection, cameraRight);

	Vec3f_normalize(&cameraRight);
	Vec3f_normalize(&cameraUp);

	if(Engine_keys[ENGINE_KEY_W].down){
		float scale = 0.03;
		cameraPos.x += cameraDirection.x * scale;
		cameraPos.y += cameraDirection.y * scale;
		cameraPos.z += cameraDirection.z * scale;
	}

	if(Engine_keys[ENGINE_KEY_S].down){
		float scale = 0.03;
		cameraPos.x += -cameraDirection.x * scale;
		cameraPos.y += -cameraDirection.y * scale;
		cameraPos.z += -cameraDirection.z * scale;
	}

	for(int i = 0; i < entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		if(i == 0){
			entity_p->rotation.y += 0.02;
			entity_p->rotation.z += 0.02;
			entity_p->pos.y = 0.4 + 0.1 * sin(time / 50);
		}

	}

}

void Engine_draw(){

	//setup camera matrices
	Mat4f cameraMat4f = getIdentityMat4f();

	{
		Mat4f lookAtMat4f = { 
			cameraRight.x,	   cameraRight.y, 	  cameraRight.z, 	 0.0,
			cameraUp.x, 	   cameraUp.y, 		  cameraUp.z, 		 0.0,
			cameraDirection.x, cameraDirection.y, cameraDirection.z, 0.0,
			0.0, 			   0.0, 			  0.0, 				 1.0,
		};

		Mat4f_mulByMat4f(&lookAtMat4f, getTranslationMat4f(-cameraPos.x, -cameraPos.y, -cameraPos.z));

		Mat4f_mulByMat4f(&cameraMat4f, lookAtMat4f);
	}

	//setup projection matrices
	Mat4f perspectiveMat4f = getIdentityMat4f();

	Mat4f_mulByMat4f(&perspectiveMat4f, getPerspectiveMat4f(fov, aspectRatio, far, near));

	//setup light matrices

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);  
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);  

	glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	//glViewport(0, 0, Engine_clientWidth, Engine_clientHeight);

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	for(int i = 0; i < entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		unsigned int currentShaderProgram = shadowMapShaderProgram;

		glUseProgram(currentShaderProgram);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindVertexArray(VAO);

		Mat4f modelRotationMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelRotationMat4f, getRotationMat4f(entity_p->rotation.x, entity_p->rotation.y, entity_p->rotation.z));

		Mat4f modelMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(entity_p->pos.x, entity_p->pos.y, entity_p->pos.z));

		Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(entity_p->scale));

		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "modelMatrix");

			glUniformMatrix4fv(location, 1, GL_FALSE, (float *)modelMat4f.values);
		}
		
		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "modelRotationMatrix");

			glUniformMatrix4fv(location, 1, GL_FALSE, (float *)modelRotationMat4f.values);
		}

		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "cameraMatrix");

			glUniformMatrix4fv(location, 1, GL_FALSE, (float *)cameraMat4f.values);
		}

		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "perspectiveMatrix");

			glUniformMatrix4fv(location, 1, GL_FALSE, (float *)perspectiveMat4f.values);
		}

		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "lightPos");

			glUniform3f(location, lightPos.x, lightPos.y, lightPos.z);
		}

		glDrawArrays(GL_TRIANGLES, 0, numberOfTriangles * 3);
	
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); 

	glViewport(0, 0, Engine_clientWidth, Engine_clientHeight);

	glClearColor(0.0, 0.3, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(int i = 0; i < entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		unsigned int currentShaderProgram = shaderProgram;
		//unsigned int currentShaderProgram = shadowMapShaderProgram;

		glUseProgram(currentShaderProgram);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindVertexArray(VAO);

		Mat4f modelRotationMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelRotationMat4f, getRotationMat4f(entity_p->rotation.x, entity_p->rotation.y, entity_p->rotation.z));

		Mat4f modelMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(entity_p->pos.x, entity_p->pos.y, entity_p->pos.z));

		Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(entity_p->scale));

		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);

		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "modelMatrix");

			glUniformMatrix4fv(location, 1, GL_FALSE, (float *)modelMat4f.values);
		}
		
		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "modelRotationMatrix");

			glUniformMatrix4fv(location, 1, GL_FALSE, (float *)modelRotationMat4f.values);
		}

		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "cameraMatrix");

			glUniformMatrix4fv(location, 1, GL_FALSE, (float *)cameraMat4f.values);
		}

		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "perspectiveMatrix");

			glUniformMatrix4fv(location, 1, GL_FALSE, (float *)perspectiveMat4f.values);
		}

		{
			unsigned int location = glGetUniformLocation(currentShaderProgram, "lightPos");

			glUniform3f(location, lightPos.x, lightPos.y, lightPos.z);
		}

		glDrawArrays(GL_TRIANGLES, 0, numberOfTriangles * 3);
	
	}

}

void Engine_finnish(){

}
