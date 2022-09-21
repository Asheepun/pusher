#include "engine/engine.h"
#include "engine/array.h"
#include "engine/files.h"
#include "engine/shaders.h"
#include "engine/3d.h"

#include "game.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

void World_initLevelState(World *world_p){

	//Engine_setFPSMode(true);

}

void World_levelState(World *world_p){

	printf("---\n");

	if(Engine_keys[ENGINE_KEY_G].downed){
		world_p->gameState = GAME_STATE_EDITOR;
		World_initEditorState(world_p);
		return;
	}

	world_p->cameraPos = getVec3f(0, 10.0, -10.0);
	world_p->cameraRotation = getVec3f(M_PI / 2, -M_PI / 4, 0.0);

	if(Engine_keys[ENGINE_KEY_R].downed){

		World_loadLevelFromFile(world_p, world_p->currentLevel);
	
	}

	//move entities
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->lastPos = entity_p->pos;

		entity_p->velocity = getVec3f(0, 0, 0);

		//apply gravity
		if(entity_p->type == ENTITY_TYPE_PLAYER
		|| entity_p->type == ENTITY_TYPE_ROCK
		|| entity_p->type == ENTITY_TYPE_STICKY_ROCK){
			entity_p->velocity.y -= 1.0;
		}

		//control player
		if(entity_p->type == ENTITY_TYPE_PLAYER){

			if(Engine_keys[ENGINE_KEY_W].downed){
				entity_p->velocity.z += 1.0;
			}
			if(Engine_keys[ENGINE_KEY_S].downed){
				entity_p->velocity.z -= 1.0;
			}
			if(Engine_keys[ENGINE_KEY_A].downed){
				entity_p->velocity.x -= 1.0;
			}
			if(Engine_keys[ENGINE_KEY_D].downed){
				entity_p->velocity.x += 1.0;
			}

		}

	}

	//check if players are stuck x
	{
		bool playersAreStuck = false;
		float playerVelocityX = 0.0;

		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

			if(entity1_p->type == ENTITY_TYPE_PLAYER){

				playerVelocityX = entity1_p->velocity.x;

				for(int j = 0; j < world_p->entities.length; j++){

					Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

					if(entity1_p->header.ID != entity2_p->header.ID
					&& checkVec3fEquals(getAddVec3f(getVec3f(playerVelocityX, 0, 0), entity1_p->pos), entity2_p->pos)){

						if(entity2_p->type == ENTITY_TYPE_OBSTACLE){
							playersAreStuck = true;
						}
						if(entity2_p->type == ENTITY_TYPE_ROCK
						|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK){
							entity1_p = entity2_p;
							j = -1;
							continue;
						}
					}
				}
			}

		}

		if(playersAreStuck){
			for(int i = 0; i < world_p->entities.length; i++){

				Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

				if(entity1_p->type == ENTITY_TYPE_PLAYER){
					entity1_p->velocity.x = 0.0;
				}
			}
		
		}
	}

	//check if players are stuck y
	{
		bool playersAreStuck = false;
		float playerVelocityY = 0.0;

		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

			if(entity1_p->type == ENTITY_TYPE_PLAYER){

				playerVelocityY = entity1_p->velocity.y;

				for(int j = 0; j < world_p->entities.length; j++){

					Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

					if(entity1_p->header.ID != entity2_p->header.ID
					&& checkVec3fEquals(getAddVec3f(getVec3f(0.0, playerVelocityY, 0.0), entity1_p->pos), entity2_p->pos)){

						if(entity2_p->type == ENTITY_TYPE_OBSTACLE){
							playersAreStuck = true;
						}
						if(entity2_p->type == ENTITY_TYPE_ROCK
						|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK){
							entity1_p = entity2_p;
							j = -1;
							continue;
						}
					}
				}
			
			}

		}

		if(playersAreStuck){
			for(int i = 0; i < world_p->entities.length; i++){

				Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

				if(entity1_p->type == ENTITY_TYPE_PLAYER){
					entity1_p->velocity.y = 0.0;
				}
			}
		
		}
	}

	//check if players are stuck z
	{
		bool playersAreStuck = false;
		float playerVelocityZ = 0.0;

		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

			if(entity1_p->type == ENTITY_TYPE_PLAYER){

				playerVelocityZ = entity1_p->velocity.z;

				for(int j = 0; j < world_p->entities.length; j++){

					Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

					if(entity1_p->header.ID != entity2_p->header.ID
					&& checkVec3fEquals(getAddVec3f(getVec3f(0.0, 0.0, playerVelocityZ), entity1_p->pos), entity2_p->pos)){

						if(entity2_p->type == ENTITY_TYPE_OBSTACLE){
							playersAreStuck = true;
						}
						if(entity2_p->type == ENTITY_TYPE_ROCK
						|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK){
							entity1_p = entity2_p;
							j = -1;
							continue;
						}
					}
				}
			
			}

		}

		if(playersAreStuck){
			for(int i = 0; i < world_p->entities.length; i++){

				Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

				if(entity1_p->type == ENTITY_TYPE_PLAYER){
					entity1_p->velocity.z = 0.0;
				}
			}
		
		}
	}

	//handle player and rock pushing
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_PLAYER
		|| entity1_p->type == ENTITY_TYPE_ROCK
		|| entity1_p->type == ENTITY_TYPE_STICKY_ROCK){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity1_p->header.ID != entity2_p->header.ID
				&& (entity2_p->type == ENTITY_TYPE_ROCK || entity2_p->type == ENTITY_TYPE_STICKY_ROCK)
				&& (checkVec3fEquals(getAddVec3f(entity1_p->pos, getVec3f(entity1_p->velocity.x, 0.0, 0.0)), entity2_p->pos)
				|| checkVec3fEquals(getAddVec3f(entity1_p->pos, getVec3f(0.0, 0.0, entity1_p->velocity.z)), entity2_p->pos))
				&& (fabs(entity1_p->velocity.x) > 0.0001
				|| fabs(entity1_p->velocity.z) > 0.0001)){

					entity2_p->velocity = entity1_p->velocity;

					entity1_p = entity2_p;
					j = -1;
					continue;

				}

			}

		}

	}

	//handle player and rock carrying
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_PLAYER
		|| entity1_p->type == ENTITY_TYPE_ROCK
		|| entity1_p->type == ENTITY_TYPE_STICKY_ROCK){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity1_p->header.ID != entity2_p->header.ID
				&& (entity2_p->type == ENTITY_TYPE_ROCK || entity2_p->type == ENTITY_TYPE_STICKY_ROCK)
				&& checkVec3fEquals(getAddVec3f(entity1_p->pos, getVec3f(0.0, 1.0, 0.0)), entity2_p->pos)
				&& (fabs(entity1_p->velocity.x) > 0.0001
				|| fabs(entity1_p->velocity.z) > 0.0001)){

					entity2_p->velocity = entity1_p->velocity;

					entity1_p = entity2_p;
					j = -1;
					continue;

				}

			}

		}

	}

	//move entities x
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->pos.x += entity_p->velocity.x;

	}

	//handle collision with players x
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_PLAYER){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity1_p->header.ID != entity2_p->header.ID
				&& (entity2_p->type == ENTITY_TYPE_ROCK
				|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK)
				&& checkVec3fEquals(entity1_p->pos, entity2_p->pos)){

					entity2_p->pos.x -= entity2_p->pos.x - entity2_p->lastPos.x;

					entity1_p = entity2_p;
					j = -1;
					continue;

				}

			}
		
		}

	}

	//handle collision with obstacles x
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_OBSTACLE){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity1_p->header.ID != entity2_p->header.ID
				&& (entity2_p->type == ENTITY_TYPE_PLAYER
				|| entity2_p->type == ENTITY_TYPE_ROCK
				|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK)
				&& checkVec3fEquals(entity1_p->pos, entity2_p->pos)){

					entity2_p->pos.x -= entity2_p->pos.x - entity2_p->lastPos.x;

					entity1_p = entity2_p;
					j = -1;
					continue;

				}

			}
		
		}

	}

	//move entities y
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->pos.y += entity_p->velocity.y;

	}

	//handle collision with players y
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_PLAYER){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity1_p->header.ID != entity2_p->header.ID
				&& (entity2_p->type == ENTITY_TYPE_ROCK
				|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK)
				&& checkVec3fEquals(entity1_p->pos, entity2_p->pos)){

					entity2_p->pos.y -= entity2_p->pos.y - entity2_p->lastPos.y;

					entity1_p = entity2_p;
					j = -1;
					continue;

				}

			}
		
		}

	}

	//handle collision with obstacles y
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_OBSTACLE){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity1_p->header.ID != entity2_p->header.ID
				&& (entity2_p->type == ENTITY_TYPE_PLAYER
				|| entity2_p->type == ENTITY_TYPE_ROCK
				|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK)
				&& checkVec3fEquals(entity1_p->pos, entity2_p->pos)){

					entity2_p->pos.y -= entity2_p->pos.y - entity2_p->lastPos.y;

					entity1_p = entity2_p;
					j = -1;
					continue;

				}

			}
		
		}

	}

	//move entities z
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->pos.z += entity_p->velocity.z;

	}

	//handle collision with players z
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_PLAYER){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity1_p->header.ID != entity2_p->header.ID
				&& (entity2_p->type == ENTITY_TYPE_ROCK
				|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK)
				&& checkVec3fEquals(entity1_p->pos, entity2_p->pos)){

					entity2_p->pos.z -= entity2_p->pos.z - entity2_p->lastPos.z;

					entity1_p = entity2_p;
					j = -1;
					continue;

				}

			}
		
		}

	}

	//handle collision with obstacles z
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_OBSTACLE){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity1_p->header.ID != entity2_p->header.ID
				&& (entity2_p->type == ENTITY_TYPE_PLAYER
				|| entity2_p->type == ENTITY_TYPE_ROCK
				|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK)
				&& checkVec3fEquals(entity1_p->pos, entity2_p->pos)){

					entity2_p->pos.z -= entity2_p->pos.z - entity2_p->lastPos.z;

					entity1_p = entity2_p;
					j = -1;
					continue;

				}

			}
		
		}

	}

	//handle player vs sticky rock collision
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity_p->type == ENTITY_TYPE_PLAYER){

			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);
				
				if(i != j
				&& entity2_p->type == ENTITY_TYPE_STICKY_ROCK
				&& checkEntityTouching(*entity_p, *entity2_p)){
					entity2_p->type = ENTITY_TYPE_PLAYER;
					entity2_p->color = PLAYER_COLOR;
				}
				
			}
		
		}
	
	}

	//handle entities vs goal
	{
		int numberOfGoals = 0;
		int numberOfHitGoals = 0;

		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

			if(entity_p->type == ENTITY_TYPE_GOAL){

				numberOfGoals++;

				for(int j = 0; j < world_p->entities.length; j++){

					Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);
					
					if(i != j
					&& entity2_p->type == entity_p->targetType
					&& (checkVec3fEquals(entity_p->pos, entity2_p->pos)
					|| checkVec3fEquals(entity_p->pos, getAddVec3f(entity2_p->lastPos, getVec3f(entity2_p->velocity.x, 0.0, entity2_p->velocity.z)))
					&& checkVec3fEquals(entity_p->pos, getAddVec3f(entity2_p->pos, getVec3f(0.0, 1.0, 0.0))))){
						numberOfHitGoals++;
						break;
					}
					
				}
			
			}

		}

		if(numberOfGoals > 0
		&& numberOfGoals == numberOfHitGoals){

			for(int i = 0; i < sizeof(STORY_LEVEL_NAMES) / sizeof(char *); i++){
				if(strcmp(world_p->currentLevel, STORY_LEVEL_NAMES[i]) == 0){


					String_set(world_p->currentLevel, STORY_LEVEL_NAMES[i + 1], STRING_SIZE);

					World_loadLevelFromFile(world_p, world_p->currentLevel);

					return;
				}
			}

		}
	}

}
