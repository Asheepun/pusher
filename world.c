#include "engine/engine.h"
#include "engine/array.h"
#include "engine/files.h"
#include "engine/shaders.h"
#include "engine/3d.h"

#include "game.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

bool checkVec3fEquals(Vec3f v1, Vec3f v2){
	return fabs(v1.x - v2.x) < 0.0001
	&& fabs(v1.y - v2.y) < 0.0001
	&& fabs(v1.z - v2.z) < 0.0001;
}

bool checkEntityTouching(Entity e1, Entity e2){
	return fabs(fabs(e1.pos.x - e2.pos.x) - 1.0) < 0.0001
	&& fabs(e1.pos.y - e2.pos.y) < 0.0001
	&& fabs(e1.pos.z - e2.pos.z) < 0.0001
	|| fabs(fabs(e1.pos.y - e2.pos.y) - 1.0) < 0.0001
	&& fabs(e1.pos.x - e2.pos.x) < 0.0001
	&& fabs(e1.pos.z - e2.pos.z) < 0.0001
	|| fabs(fabs(e1.pos.z - e2.pos.z) - 1.0) < 0.0001
	&& fabs(e1.pos.x - e2.pos.x) < 0.0001
	&& fabs(e1.pos.y - e2.pos.y) < 0.0001;
}

Entity *World_addEntity(World *world_p, Vec3f pos, Model model, float scale, enum EntityType type, Vec4f color){

	Entity *entity_p = Array_addItem(&world_p->entities);

	EntityHeader_init(&entity_p->header);

	entity_p->pos = pos;
	entity_p->scale = scale;

	entity_p->model = model;
	entity_p->color = color;

	entity_p->type = type;

	entity_p->velocity = getVec3f(0, 0, 0);
	entity_p->rotation = getVec3f(0, 0, 0);

	entity_p->targetType = ENTITY_TYPE_PLAYER;

	return entity_p;

}

Entity *World_addPlayer(World *world_p, Vec3f pos){

	Entity *entity_p = World_addEntity(world_p, pos, world_p->boxModel, 0.6, ENTITY_TYPE_PLAYER, PLAYER_COLOR);

	return entity_p;

}

Entity *World_addRock(World *world_p, Vec3f pos){

	Entity *entity_p = World_addEntity(world_p, pos, world_p->boxModel, 0.6, ENTITY_TYPE_ROCK, ROCK_COLOR);

	return entity_p;

}

Entity *World_addStickyRock(World *world_p, Vec3f pos){

	Entity *entity_p = World_addEntity(world_p, pos, world_p->boxModel, 0.6, ENTITY_TYPE_STICKY_ROCK, STICKY_ROCK_COLOR);

	return entity_p;

}

Entity *World_addGoal(World *world_p, Vec3f pos, Vec4f color, enum EntityType targetType){

	Entity *entity_p = World_addEntity(world_p, pos, world_p->boxModel, 0.6, ENTITY_TYPE_GOAL, getVec4f(color.x, color.y, color.z, GOAL_ALPHA));

	entity_p->targetType = targetType;

	return entity_p;

}

Entity *World_addObstacle(World *world_p, Vec3f pos){

	Entity *entity_p = World_addEntity(world_p, pos, world_p->boxModel, 0.6, ENTITY_TYPE_OBSTACLE, OBSTACLE_COLOR);

	return entity_p;

}

Entity *World_addLevelDoor(World *world_p, Vec3f pos){

	Entity *entity_p = World_addEntity(world_p, pos, world_p->boxModel, 0.6, ENTITY_TYPE_LEVEL_DOOR, LEVEL_DOOR_COLOR);

	String_set(entity_p->levelName, "", STRING_SIZE);

	return entity_p;

}
