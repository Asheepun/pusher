#include "engine/engine.h"
#include "engine/array.h"
#include "engine/files.h"
#include "engine/shaders.h"
#include "engine/3d.h"

#include "game.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

int entityIndexGridWidth = 100;
int entityIndexGridHeight = 20;

int entityIndexGrid[100][100][20];

void World_resetEntityIndexGrid(World *world_p){
	for(int i = 0; i < entityIndexGridWidth; i++){
		for(int j = 0; j < entityIndexGridWidth; j++){
			for(int k = 0; k < entityIndexGridHeight; k++){
				entityIndexGrid[i][j][k] = -1;
			}
		}
	}
}

void World_putEntityIndexIntoGrid(World *world_p, Vec3f pos, int index){
	entityIndexGrid[(int)pos.x + entityIndexGridWidth / 2][(int)pos.y + entityIndexGridWidth / 2][(int)pos.z + entityIndexGridHeight / 2] = index;
}

Entity *World_getEntityFromGrid(World *world_p, Vec3f pos){
	int index = entityIndexGrid[(int)pos.x + entityIndexGridWidth / 2][(int)pos.y + entityIndexGridWidth / 2][(int)pos.z + entityIndexGridHeight / 2];

	if(index == -1){
		return NULL;
	}
	return Array_getItemPointerByIndex(&world_p->entities, index);
}

bool World_checkPosOub(World *world_p, Vec3f pos){
	return !(pos.x > -entityIndexGridWidth / 2
		&& pos.x < entityIndexGridWidth / 2 - 1
		&& pos.y > -entityIndexGridHeight / 2
		&& pos.y < entityIndexGridHeight / 2 - 1
		&& pos.z > -entityIndexGridWidth / 2
		&& pos.z < entityIndexGridWidth / 2 - 1);
}

void World_initLevelState(World *world_p){

	if(strcmp(world_p->currentLevel, "levelhub") == 0){

		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

			if(entity_p->type == ENTITY_TYPE_LEVEL_DOOR){

				for(int j = 0; j < world_p->saveData.completedLevels.length; j++){

					char *levelName = Array_getItemPointerByIndex(&world_p->saveData.completedLevels, j);
					
					if(strcmp(levelName, entity_p->levelName) == 0){
						Array_removeItemByIndex(&world_p->entities, i);
						i--;
						break;
					}

				}
			
			}

			if(entity_p->type == ENTITY_TYPE_PLAYER){
				entity_p->pos = world_p->saveData.playerPos;
			}

		}

	}

	//Engine_setFPSMode(true);

}

void World_levelState(World *world_p){

	bool didAction = false;

	printf("---\n");

	if(Engine_keys[ENGINE_KEY_G].downed){
		world_p->gameState = GAME_STATE_EDITOR;
		World_initEditorState(world_p);
		return;
	}

	if(Engine_keys[ENGINE_KEY_R].downed){

		//World_loadLevelFromFile(world_p, world_p->currentLevel);
		World_loadLevelFromFile(world_p, "CURRENT_WORKING_LEVEL");
		World_initLevelState(world_p);
	
	}

	//control player velocity
	Vec3f playerVelocity = { 0, -1.0, 0 };

	if(Engine_keys[ENGINE_KEY_W].downed){
		playerVelocity.z += 1.0;
		didAction = true;
	}
	if(Engine_keys[ENGINE_KEY_S].downed){
		playerVelocity.z -= 1.0;
		didAction = true;
	}
	if(Engine_keys[ENGINE_KEY_A].downed){
		playerVelocity.x -= 1.0;
		didAction = true;
	}
	if(Engine_keys[ENGINE_KEY_D].downed){
		playerVelocity.x += 1.0;
		didAction = true;
	}


	//apply forces to entities
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->lastPos = entity_p->pos;

		entity_p->velocity = getVec3f(0, 0, 0);

		//apply gravity
		if(entity_p->type == ENTITY_TYPE_PLAYER
		|| entity_p->type == ENTITY_TYPE_ROCK
		|| entity_p->type == ENTITY_TYPE_STICKY_ROCK){
		//|| entity_p->type == ENTITY_TYPE_RISER){
			entity_p->velocity.y -= 1.0;
		}

		//control players
		if(entity_p->type == ENTITY_TYPE_PLAYER){
			entity_p->velocity = playerVelocity;
		}

	}

	//reset grid and place entity indices in grid
	World_resetEntityIndexGrid(world_p);

	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(!World_checkPosOub(world_p, entity_p->pos)
		&& !(entity_p->type == ENTITY_TYPE_GOAL
		|| entity_p->type == ENTITY_TYPE_RISER
		|| entity_p->type == ENTITY_TYPE_LEVEL_DOOR)){
			World_putEntityIndexIntoGrid(world_p, entity_p->pos, i);
		}

	}

	//handle risers
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_RISER
		&& !World_checkPosOub(world_p, entity1_p->pos)){

			Entity *entity2_p = NULL;

			bool inside = false;
			bool above = false;

			//check inside
			{
				entity2_p = World_getEntityFromGrid(world_p, entity1_p->pos);

				if(entity2_p != NULL){
					entity2_p->velocity.y = 1.0;
					inside = true;
				}
			}
			//check above
			if(!inside){

				entity2_p = World_getEntityFromGrid(world_p, getAddVec3f(entity1_p->pos, getVec3f(0.0, 1.0, 0.0)));

				if(entity2_p != NULL){
					entity2_p->velocity.y = 0.0;
					above = true;
				}

			}

			if(inside || above){

				Vec3f checkPos = entity2_p->pos;

				checkPos.y += 1.0;

				Entity *checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

				bool blocked = false;

				while(checkEntity_p != NULL){

					if(checkEntity_p->type == ENTITY_TYPE_OBSTACLE){
						blocked = true;
						break;
					}

					checkEntity_p->velocity.y = entity2_p->velocity.y;

					checkPos.y += 1.0;

					checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

				}

				if(entity2_p->type == ENTITY_TYPE_PLAYER){
					playerVelocity.y = entity2_p->velocity.y;
				}
			
			}
			
		}
		
	}

	//set players velocities to current player velocity
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity_p->type == ENTITY_TYPE_PLAYER){
			entity_p->velocity = playerVelocity;
		}

	}

	//check if a player is blocked
	for(int j = 0; j < 3; j++){

		//find pointer offset to coordinate
		Vec3f tmp;
		long int pointerOffset;
		if(j == 0){
			pointerOffset = ((float*)&tmp.x - (float *)&tmp);
		}
		if(j == 1){
			pointerOffset = ((float*)&tmp.y - (float *)&tmp);
		}
		if(j == 2){
			pointerOffset = ((float*)&tmp.z - (float *)&tmp);
		}

		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

			float *entityVelocity_p = ((float *)&entity_p->velocity) + pointerOffset;

			if(entity_p->type == ENTITY_TYPE_PLAYER
			&& *entityVelocity_p != 0
			&& !World_checkPosOub(world_p, entity_p->pos)){

				Vec3f dir = { 0, 0, 0 };
				*(((float *)&dir) + pointerOffset) = *entityVelocity_p;

				Vec3f checkPos = entity_p->pos;
				Vec3f_add(&checkPos, dir);

				Entity *checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

				bool blocked = false;

				//check for obstacles
				while(!World_checkPosOub(world_p, checkPos)
				&& checkEntity_p != NULL){

					if(checkEntity_p->type == ENTITY_TYPE_OBSTACLE){
						blocked = true;
						break;
					}

					Vec3f_add(&checkPos, dir);

					checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

				}

				if(blocked){
					*(((float *)&playerVelocity) + pointerOffset) = 0;
				}

			}

		}

	}

	//set players velocities to checked player velocity
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity_p->type == ENTITY_TYPE_PLAYER){
			entity_p->velocity = playerVelocity;
		}

	}

	//check if entities can move
	for(int j = 0; j < 3; j++){

		//find pointer offset to coordinate
		Vec3f tmp;
		long int pointerOffset;
		if(j == 0){
			pointerOffset = ((float*)&tmp.x - (float *)&tmp);
		}
		if(j == 1){
			pointerOffset = ((float*)&tmp.y - (float *)&tmp);
		}
		if(j == 2){
			pointerOffset = ((float*)&tmp.z - (float *)&tmp);
		}

		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

			float *entityVelocity_p = ((float *)&entity_p->velocity) + pointerOffset;

			if((entity_p->type == ENTITY_TYPE_PLAYER
			|| entity_p->type == ENTITY_TYPE_ROCK
			|| entity_p->type == ENTITY_TYPE_STICKY_ROCK)
			&& *entityVelocity_p != 0
			&& !World_checkPosOub(world_p, entity_p->pos)){

				Vec3f dir = { 0, 0, 0 };
				*(((float *)&dir) + pointerOffset) = *entityVelocity_p;

				Vec3f checkPos = entity_p->pos;
				Vec3f_add(&checkPos, dir);

				Entity *checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

				bool blocked = false;

				//propogate velocities
				while(!World_checkPosOub(world_p, checkPos)
				&& checkEntity_p != NULL){

					if(checkEntity_p->type == ENTITY_TYPE_OBSTACLE
					|| (checkEntity_p->type == ENTITY_TYPE_PLAYER && entity_p->type != ENTITY_TYPE_PLAYER)){
						blocked = true;
						break;
					}

					*(((float *)&checkEntity_p->velocity) + pointerOffset) = *entityVelocity_p;

					Vec3f_add(&checkPos, dir);

					checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

				}

				//reset if blocked
				if(blocked){
					while(!checkEqualsVec3f(checkPos, entity_p->pos, 0.01)){

						Vec3f_sub(&checkPos, dir);

						checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

						*(((float *)&checkEntity_p->velocity) + pointerOffset) = 0;

					}
				}

			}

		}

	}

	//handle friction
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->velocity.x != 0
		|| entity1_p->velocity.z != 0){

			Vec3f checkPos = entity1_p->pos;
			checkPos.y += 1.0;

			Entity *entity2_p = World_getEntityFromGrid(world_p, checkPos);
			
			while(entity2_p != NULL
			&& (entity2_p->type == ENTITY_TYPE_ROCK
			|| entity2_p->type == ENTITY_TYPE_STICKY_ROCK)){

				entity2_p->velocity.x = entity1_p->velocity.x;
				entity2_p->velocity.z = entity1_p->velocity.z;

				checkPos.y += 1.0;

				entity2_p = World_getEntityFromGrid(world_p, checkPos);

			}

		}

	}

	//check if entities can move second time
	for(int j = 0; j < 3; j++){

		//find pointer offset to coordinate
		Vec3f tmp;
		long int pointerOffset;
		if(j == 0){
			pointerOffset = ((float*)&tmp.x - (float *)&tmp);
		}
		if(j == 1){
			pointerOffset = ((float*)&tmp.y - (float *)&tmp);
		}
		if(j == 2){
			pointerOffset = ((float*)&tmp.z - (float *)&tmp);
		}

		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

			float *entityVelocity_p = ((float *)&entity_p->velocity) + pointerOffset;

			if((entity_p->type == ENTITY_TYPE_PLAYER
			|| entity_p->type == ENTITY_TYPE_ROCK
			|| entity_p->type == ENTITY_TYPE_STICKY_ROCK)
			&& *entityVelocity_p != 0
			&& !World_checkPosOub(world_p, entity_p->pos)){

				Vec3f dir = { 0, 0, 0 };
				*(((float *)&dir) + pointerOffset) = *entityVelocity_p;

				Vec3f checkPos = entity_p->pos;
				Vec3f_add(&checkPos, dir);

				Entity *checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

				bool blocked = false;

				//propogate velocities
				while(!World_checkPosOub(world_p, checkPos)
				&& checkEntity_p != NULL){

					if(checkEntity_p->type == ENTITY_TYPE_OBSTACLE
					|| (checkEntity_p->type == ENTITY_TYPE_PLAYER && entity_p->type != ENTITY_TYPE_PLAYER)){
						blocked = true;
						break;
					}

					*(((float *)&checkEntity_p->velocity) + pointerOffset) = *entityVelocity_p;

					Vec3f_add(&checkPos, dir);

					checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

				}

				//reset if blocked
				if(blocked){
					while(!checkEqualsVec3f(checkPos, entity_p->pos, 0.01)){

						Vec3f_sub(&checkPos, dir);

						checkEntity_p = World_getEntityFromGrid(world_p, checkPos);

						*(((float *)&checkEntity_p->velocity) + pointerOffset) = 0;

					}
				}

			}

		}

	}

	//move entities
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		Vec3f_add(&entity_p->pos, entity_p->velocity);

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

			char *completedLevelName = Array_addItem(&world_p->saveData.completedLevels);

			String_set(completedLevelName, world_p->currentLevel, STRING_SIZE);

			String_set(world_p->currentLevel, "levelhub", STRING_SIZE);

			World_loadLevelFromFile(world_p, world_p->currentLevel);

			World_initLevelState(world_p);

			return;

		}
	}

	//check player collision with level doors
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity1_p->type == ENTITY_TYPE_PLAYER){
			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(i != j
				&& entity2_p->type == ENTITY_TYPE_LEVEL_DOOR
				&& checkVec3fEquals(entity1_p->pos, entity2_p->pos)){

					String_set(world_p->currentLevel, entity2_p->levelName, STRING_SIZE);

					world_p->saveData.playerPos = entity1_p->pos;

					World_initLevelState(world_p);

					World_loadLevelFromFile(world_p, world_p->currentLevel);

					return;
				

				}

			}
		}

	}

	//handle undo
	if(Engine_keys[ENGINE_KEY_Z].downed
	&& world_p->undos.length > 0){
		
		Array_free(&world_p->entities);
		Array_free(&world_p->lastEntities);

		Array *undo_p = Array_getItemPointerByIndex(&world_p->undos, world_p->undos.length - 1);

		world_p->entities = Array_getCopy_mustFree(undo_p);
		world_p->lastEntities = Array_getCopy_mustFree(undo_p);

		Array_free(undo_p);

		Array_removeItemByIndex(&world_p->undos, world_p->undos.length - 1);

	}

	//add undo
	if(didAction){

		Array *undo_p = Array_addItem(&world_p->undos);

		*undo_p = Array_getCopy_mustFree(&world_p->lastEntities);

	}

	//update last entities
	{
		Array_free(&world_p->lastEntities);

		world_p->lastEntities = Array_getCopy_mustFree(&world_p->entities);
	}

	//update camera pos
	world_p->cameraPos = getVec3f(0, 10.0, -10.0);
	world_p->cameraRotation = getVec3f(M_PI / 2, -M_PI / 4, 0.0);

	if(strcmp(world_p->currentLevel, "levelhub") == 0){
		for(int i = 0; i < world_p->entities.length; i++){

			Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

			if(entity_p->type == ENTITY_TYPE_PLAYER){
				Vec3f_add(&world_p->cameraPos, entity_p->pos);
			}
		
		}
	}

}
