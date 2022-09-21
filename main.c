#include "engine/engine.h"
#include "engine/array.h"
#include "engine/files.h"
#include "engine/shaders.h"
#include "engine/3d.h"
#include "engine/renderer2d.h"
#include "engine/igui.h"

#include "game.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

Model teapotModel;
Model treeModel;
Model keyModel;

unsigned int triangleVBO;

unsigned int shadowMapFBO;
unsigned int shadowMapTexture;
unsigned int SHADOW_MAP_WIDTH = 1920;
unsigned int SHADOW_MAP_HEIGHT = 1080;

unsigned int shaderProgram;
unsigned int shadowMapShaderProgram;

enum ViewMode currentViewMode;

Vec3f lightPos;
Vec3f lightDirection;

World world;

Renderer2D_Renderer renderer2D;

void Engine_start(){

	Engine_setFPSMode(true);

	{

		Array_init(&world.entities, sizeof(Entity));

		String_set(world.currentLevel, "level11", STRING_SIZE);
	
	}

	Engine_setWindowSize(WIDTH / 2, HEIGHT / 2);

	Engine_centerWindow();

	Model_initFromFile_obj(&teapotModel, "assets/models/teapot.obj");
	Model_initFromFile_obj(&world.boxModel, "assets/models/box.obj");
	Model_initFromFile_obj(&treeModel, "assets/models/tree.obj");
	Model_initFromFile_obj(&keyModel, "assets/models/key.obj");

	world.boxMesh = getMeshDataFromFile_obj("assets/models/box.obj", &world.boxMeshNumberOfTriangles);

	Renderer2D_init(&renderer2D, WIDTH, HEIGHT);

	IGUI_init(WIDTH, HEIGHT);

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

	//world.levels[world.currentLevel](&world);

	World_loadLevelFromFile(&world, world.currentLevel);

	world.cameraPos = getVec3f(0, 10.0, -10.0);
	world.cameraDirection = getVec3f(0.0, 0.0, 1.0);
	world.cameraRotation = getVec3f(M_PI / 2, -M_PI / 4, 0.0);

	lightPos = getVec3f(-3, 30, -8);
	lightDirection = getVec3f(0.001, -1.0, 0.0);
	Vec3f_normalize(&lightDirection);

	currentViewMode = VIEW_MODE_CAMERA;

	world.gameState = GAME_STATE_LEVEL;
	
	World_initLevelState(&world);

}

float time = 0;

void Engine_update(float deltaTime){

	IGUI_updatePointerScale();

	//lightPos = getVec3f(-2, 20, -7);
	//lightPos.y = 15 + 10 * sin(time / 50);

	time++;

	if(Engine_keys[ENGINE_KEY_Q].down){
		Engine_quit();
	}

	if(Engine_keys[ENGINE_KEY_F].downed){
		Engine_toggleFullscreen();
	}

	if(Engine_keys[ENGINE_KEY_V].downed){
		currentViewMode++;
	}
	if(currentViewMode >= NUMBER_OF_VIEW_MODES){
		currentViewMode = 0;
	}

	world.cameraDirection.y = sin(world.cameraRotation.y);
	world.cameraDirection.x = cos(world.cameraRotation.x) * cos(world.cameraRotation.y);
	world.cameraDirection.z = sin(world.cameraRotation.x) * cos(world.cameraRotation.y);
	Vec3f_normalize(&world.cameraDirection);

	if(world.gameState == GAME_STATE_LEVEL){
		World_levelState(&world);
	}else if(world.gameState == GAME_STATE_EDITOR){
		World_editorState(&world);
	}

}

void Engine_draw(){

	//setup world.camera matrices
	Mat4f cameraMat4f = getIdentityMat4f();

	Mat4f_mulByMat4f(&cameraMat4f, getLookAtMat4f(world.cameraPos, world.cameraDirection));

	//setup light matrices
	Mat4f lightMat4f = getIdentityMat4f();

	Mat4f_mulByMat4f(&lightMat4f, getLookAtMat4f(lightPos, lightDirection));

	//setup projection matrices
	Mat4f perspectiveMat4f = getIdentityMat4f();

	Mat4f_mulByMat4f(&perspectiveMat4f, getPerspectiveMat4f(fov, (float)Engine_clientWidth / (float)Engine_clientHeight, far, near));

	//render shadow map
	glDisable(GL_CULL_FACE);

	if(currentViewMode == VIEW_MODE_SHADOW_MAP){
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  
		glViewport(0, 0, Engine_clientWidth, Engine_clientHeight);
	}else{
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);  
		glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(int i = 0; i < world.entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world.entities, i);

		unsigned int currentShaderProgram = shadowMapShaderProgram;

		glUseProgram(currentShaderProgram);

		glBindBuffer(GL_ARRAY_BUFFER, entity_p->model.VBO);
		glBindVertexArray(entity_p->model.VAO);

		Mat4f modelRotationMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelRotationMat4f, getRotationMat4f(entity_p->rotation.x, entity_p->rotation.y, entity_p->rotation.z));

		Mat4f modelMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(entity_p->pos.x * 2.0 * 0.6, entity_p->pos.y * 2.0 * 0.6, entity_p->pos.z * 2.0 * 0.6));

		Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(entity_p->scale));

		GL3D_uniformMat4f(currentShaderProgram, "modelMatrix", modelMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "modelRotationMatrix", modelRotationMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "perspectiveMatrix", perspectiveMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "lightMatrix", lightMat4f);

		GL3D_uniformVec3f(currentShaderProgram, "lightPos", lightPos);
		GL3D_uniformVec3f(currentShaderProgram, "cameraPos", world.cameraPos);

		glDrawArrays(GL_TRIANGLES, 0, entity_p->model.numberOfTriangles * 3);
	
	}

	//render scene
	glEnable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);

	if(currentViewMode == VIEW_MODE_CAMERA){
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}else{
		glBindFramebuffer(GL_FRAMEBUFFER, 1);
	}

	glViewport(0, 0, Engine_clientWidth, Engine_clientHeight);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(int i = 0; i < world.entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world.entities, i);

		unsigned int currentShaderProgram = shaderProgram;

		glUseProgram(currentShaderProgram);

		glBindBuffer(GL_ARRAY_BUFFER, entity_p->model.VBO);
		glBindVertexArray(entity_p->model.VAO);

		Mat4f modelRotationMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelRotationMat4f, getRotationMat4f(entity_p->rotation.x, entity_p->rotation.y, entity_p->rotation.z));

		Mat4f modelMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(entity_p->pos.x * 2.0 * 0.6, entity_p->pos.y * 2.0 * 0.6, entity_p->pos.z * 2.0 * 0.6));

		Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(entity_p->scale));

		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);

		GL3D_uniformMat4f(currentShaderProgram, "modelMatrix", modelMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "modelRotationMatrix", modelRotationMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "cameraMatrix", cameraMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "perspectiveMatrix", perspectiveMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "lightMatrix", lightMat4f);

		GL3D_uniformVec3f(currentShaderProgram, "lightPos", lightPos);
		GL3D_uniformVec3f(currentShaderProgram, "cameraPos", world.cameraPos);

		GL3D_uniformVec4f(currentShaderProgram, "color", entity_p->color);

		glDrawArrays(GL_TRIANGLES, 0, entity_p->model.numberOfTriangles * 3);
	
	}

	glDisable(GL_DEPTH_TEST);

	//draw 2d stuff
	Renderer2D_updateDrawSize(&renderer2D, Engine_clientWidth, Engine_clientHeight);

	if(world.gameState == GAME_STATE_EDITOR){
		Renderer2D_drawColoredRectangle(&renderer2D, WIDTH / 2 - 3, HEIGHT / 2 - 3, 6, 6, Renderer2D_getColor(0.7, 0.7, 0.7), 1.0);
	}

	IGUI_render(&renderer2D);

	glEnable(GL_DEPTH_TEST);

}

void Engine_finnish(){

}
