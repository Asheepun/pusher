#ifndef GAME_H_
#define GAME_H_

#include "engine/engine.h"
#include "engine/array.h"
#include "engine/strings.h"
#include "engine/3d.h"

#include "math.h"

//ENUMS

enum EntityType{
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_ROCK,
	ENTITY_TYPE_STICKY_ROCK,
	ENTITY_TYPE_OBSTACLE,
	ENTITY_TYPE_GOAL,
	NUMBER_OF_ENTITY_TYPES,
};

enum ViewMode{
	VIEW_MODE_SHADOW_MAP,
	VIEW_MODE_CAMERA,
	NUMBER_OF_VIEW_MODES,
};

enum GameState{
	GAME_STATE_LEVEL,
	GAME_STATE_EDITOR,
};

//STRUCTS

typedef struct Entity{
	EntityHeader header;
	float scale;
	Vec3f pos;
	Vec3f lastPos;
	Vec3f velocity;
	Vec3f rotation;
	bool onGround;
	Model model;
	enum EntityType type;
	enum EntityType targetType;
	Vec4f color;
}Entity;

typedef struct World{

	Array entities;

	Model boxModel;

	void (*levels[32])(struct World *);

	//int currentLevel;

	enum GameState gameState;

	Vec3f cameraPos;
	Vec3f lastCameraPos;
	Vec3f cameraRotation;
	Vec3f cameraDirection;

	Vec3f *boxMesh;
	int boxMeshNumberOfTriangles;

	char currentLevel[STRING_SIZE];

}World;

//GLOBAL VARIBALES

static float PLAYER_WALK_SPEED = 0.10;
static float PLAYER_GROUND_RESISTANCE = 0.9;
static float PLAYER_JUMP_SPEED = 0.5;
static float PLAYER_GRAVITY = 0.03;

static Vec4f PLAYER_COLOR = { 0.0, 0.0, 1.0, 1.0 };
static Vec4f ROCK_COLOR = { 0.7, 0.7, 0.5, 1.0 };
static Vec4f STICKY_ROCK_COLOR = { 0.2, 0.8, 0.2, 1.0 };
static Vec4f OBSTACLE_COLOR = { 0.7, 0.5, 0.3, 1.0 };
static float GOAL_ALPHA = 0.5;

static int WIDTH = 1920;
static int HEIGHT = 1080;

static float fov = M_PI / 4;
static float aspectRatio = 16.0 / 9.0;
static float far = 100.0;
static float near = 0.1;

static float CAMERA_MOVEMENT_SCALE = 0.0015;

static char *STORY_LEVEL_NAMES[] = {
	"level1",
	"level2",
	"level3",
	"level4",
	"level5",
	"level6",
	"level7",
	"level8",
	"level9",
	"level10",
};

//FUNCTIONS

//FILE: world.c

bool checkVec3fEquals(Vec3f, Vec3f);

bool checkEntityTouching(Entity, Entity);

Entity *World_addEntity(World *, Vec3f, Model, float, enum EntityType, Vec4f);

Entity *World_addPlayer(World *, Vec3f);

Entity *World_addRock(World *, Vec3f);

Entity *World_addStickyRock(World *, Vec3f);

Entity *World_addGoal(World *, Vec3f, Vec4f, enum EntityType);

Entity *World_addObstacle(World *, Vec3f);

//FILE: levelState.c

void World_initLevelState(World *);

void World_levelState(World *);

//FILE: editorState.c

void World_initEditorState(World *);

void World_editorState(World *);

//FILE: levels.c

void World_loadLevelFromFile(World *, char *);

void World_writeLevelToFile(World *, char *);

#endif
