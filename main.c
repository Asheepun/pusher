#include "engine/engine.h"
#include "engine/array.h"
#include "engine/files.h"
#include "engine/shaders.h"
#include "engine/3d.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

enum EntityType{
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_OBSTACLE,
};

int WIDTH = 1920;
int HEIGHT = 1080;

float PLAYER_WALK_SPEED = 0.10;
float PLAYER_GROUND_RESISTANCE = 0.9;
float PLAYER_JUMP_SPEED = 0.5;
float PLAYER_GRAVITY = 0.03;

float CAMERA_MOVEMENT_SCALE = 0.0015;

float triangleVertices[] = {
	-1.0, -1.0, 0.0, 	0.0, 0.0, 1.0,
	0.0, 1.0, 0.0, 		0.0, 0.0, 1.0,
	1.0, -1.0, 0.0, 	0.0, 0.0, 1.0,
};

typedef struct Entity{
	float scale;
	Vec3f pos;
	Vec3f velocity;
	Vec3f acceleration;
	Vec3f resistance;
	Vec3f rotation;
	bool onGround;
	Model model;
	enum EntityType type;
}Entity;

Model teapotModel;
Model boxModel;

unsigned int triangleVBO;

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

Vec3f lightPos;
Vec3f lightDirection;

Array entities;

void Engine_start(){

	Engine_setWindowSize(WIDTH / 2, HEIGHT / 2);

	Engine_centerWindow();

	Model_initFromFile_obj(&teapotModel, "assets/models/teapot.obj");
	Model_initFromFile_obj(&boxModel, "assets/models/box.obj");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

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

	{
		Entity *entity_p = Array_addItem(&entities);
	
		entity_p->pos = getVec3f(0.0, 0.0, 0.0);
		entity_p->velocity = getVec3f(0.0, 0.0, 0.0);
		entity_p->acceleration = getVec3f(0.0, 0.0, 0.0);
		entity_p->resistance = getVec3f(PLAYER_GROUND_RESISTANCE, 1.0, PLAYER_GROUND_RESISTANCE);
		entity_p->onGround = false;

		entity_p->rotation = getVec3f(0.0, 0.0, 0.0);
		entity_p->scale = 1.0;

		entity_p->model = teapotModel;
		entity_p->type = ENTITY_TYPE_PLAYER;
	}

	/*
	for(int i = 0; i < 10; i++){

		Entity *entity_p = Array_addItem(&entities);
	
		entity_p->pos = getVec3f(4.0 * i - 20.0, -4.0, 0.0);
		entity_p->velocity = getVec3f(0.0, 0.0, 0.0);
		entity_p->acceleration = getVec3f(0.0, 0.0, 0.0);
		entity_p->resistance = getVec3f(1.0, 1.0, 1.0);
		entity_p->onGround = false;

		entity_p->rotation = getVec3f(0.0, 0.0, 0.0);
		entity_p->scale = 2.0;

		entity_p->model = boxModel;

		entity_p->type = ENTITY_TYPE_OBSTACLE;
	
	}
	*/
	
	cameraPos = getVec3f(0, 2.0, -10);
	cameraRotation = getVec3f(M_PI / 2, 0, 0);

	lightPos = getVec3f(0, 20, 0);
	lightDirection = getVec3f(0, -1.0, 1.0);

}

float time = 0;

void Engine_update(float deltaTime){

	time++;

	if(Engine_keys[ENGINE_KEY_Q].down){
		Engine_quit();
	}

	cameraRotation.x += -Engine_pointer.movement.x * CAMERA_MOVEMENT_SCALE;
	cameraRotation.y += -Engine_pointer.movement.y * CAMERA_MOVEMENT_SCALE;

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

	if(Engine_keys[ENGINE_KEY_W].down){
		cameraPos.x += cameraDirection.x * PLAYER_WALK_SPEED;
		cameraPos.z += cameraDirection.z * PLAYER_WALK_SPEED;
	}
	if(Engine_keys[ENGINE_KEY_S].down){
		cameraPos.x += -cameraDirection.x * PLAYER_WALK_SPEED;
		cameraPos.z += -cameraDirection.z * PLAYER_WALK_SPEED;
	}
	if(Engine_keys[ENGINE_KEY_A].down){
		Vec3f left = getCrossVec3f(cameraDirection, getVec3f(0, 1.0, 0));
		Vec3f_normalize(&left);
		cameraPos.x += left.x * PLAYER_WALK_SPEED;
		cameraPos.z += left.z * PLAYER_WALK_SPEED;
	}
	if(Engine_keys[ENGINE_KEY_D].down){
		Vec3f right = getCrossVec3f(getVec3f(0, 1.0, 0), cameraDirection);
		Vec3f_normalize(&right);
		cameraPos.x += right.x * PLAYER_WALK_SPEED;
		cameraPos.z += right.z * PLAYER_WALK_SPEED;
	}

	//control player
	for(int i = 0; i < entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		entity_p->acceleration = getVec3f(0.0, 0.0, 0.0);

		//entity_p->rotation.x += 0.05;
		//entity_p->rotation.y += 0.05;
		//entity_p->rotation.z += 0.05;

	}

	//apply physics for entities
	for(int i = 0; i < entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		Vec3f_add(&entity_p->velocity, entity_p->acceleration);

		Vec3f_mulByVec3f(&entity_p->velocity, entity_p->resistance);

	}

	//move entities x
	for(int i = 0; i < entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		entity_p->pos.x += entity_p->velocity.x;

	}

	//move entities y
	for(int i = 0; i < entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		entity_p->pos.y += entity_p->velocity.y;

	}

	//handle entities collision y
	for(int i = 0; i < entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		entity_p->onGround = false;

		if(entity_p->pos.y < 0
		&& entity_p->type == ENTITY_TYPE_PLAYER){
			entity_p->pos.y = 0;
			entity_p->velocity.y = 0;
			entity_p->onGround = true;
		}

	}

	//move entities z
	for(int i = 0; i < entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		entity_p->pos.z += entity_p->velocity.z;

	}

}

void Engine_draw(){

	//setup camera matrices
	Mat4f cameraMat4f = getIdentityMat4f();

	Mat4f_mulByMat4f(&cameraMat4f, getLookAtMat4f(cameraPos, cameraDirection));

	//setup light matrices
	Mat4f lightMat4f = getIdentityMat4f();

	Mat4f_mulByMat4f(&lightMat4f, getLookAtMat4f(lightPos, lightDirection));

	//setup projection matrices
	Mat4f perspectiveMat4f = getIdentityMat4f();

	Mat4f_mulByMat4f(&perspectiveMat4f, getPerspectiveMat4f(fov, (float)Engine_clientWidth / (float)Engine_clientHeight, far, near));

	//setup light matrices

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);  
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);  

	glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	//glViewport(0, 0, Engine_clientWidth, Engine_clientHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_DEPTH_BUFFER_BIT);

	for(int i = 0; i < entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		Model currentModel = boxModel;

		unsigned int currentShaderProgram = shadowMapShaderProgram;

		glUseProgram(currentShaderProgram);

		glBindBuffer(GL_ARRAY_BUFFER, entity_p->model.VBO);
		glBindVertexArray(entity_p->model.VAO);

		Mat4f modelRotationMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelRotationMat4f, getRotationMat4f(entity_p->rotation.x, entity_p->rotation.y, entity_p->rotation.z));

		Mat4f modelMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(entity_p->pos.x, entity_p->pos.y, entity_p->pos.z));

		Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(entity_p->scale));

		GL3D_uniformMat4f(currentShaderProgram, "modelMatrix", modelMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "modelRotationMatrix", modelRotationMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "perspectiveMatrix", perspectiveMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "lightMatrix", lightMat4f);

		GL3D_uniformVec3f(currentShaderProgram, "lightPos", lightPos);

		glDrawArrays(GL_TRIANGLES, 0, teapotModel.numberOfTriangles * 3);
	
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); 

	glViewport(0, 0, Engine_clientWidth, Engine_clientHeight);

	glClearColor(0.0, 0.3, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(int i = 0; i < entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&entities, i);

		unsigned int currentShaderProgram = shaderProgram;

		glUseProgram(currentShaderProgram);

		glBindBuffer(GL_ARRAY_BUFFER, entity_p->model.VBO);
		glBindVertexArray(entity_p->model.VAO);

		Mat4f modelRotationMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelRotationMat4f, getRotationMat4f(entity_p->rotation.x, entity_p->rotation.y, entity_p->rotation.z));

		Mat4f modelMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(entity_p->pos.x, entity_p->pos.y, entity_p->pos.z));

		Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(entity_p->scale));

		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);

		GL3D_uniformMat4f(currentShaderProgram, "modelMatrix", modelMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "modelRotationMatrix", modelRotationMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "cameraMatrix", cameraMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "perspectiveMatrix", perspectiveMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "lightMatrix", lightMat4f);

		GL3D_uniformVec3f(currentShaderProgram, "lightPos", lightPos);

		glDrawArrays(GL_TRIANGLES, 0, teapotModel.numberOfTriangles * 3);
	
	}

}

void Engine_finnish(){

}
