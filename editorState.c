#include "engine/engine.h"
#include "engine/array.h"
#include "engine/files.h"
#include "engine/shaders.h"
#include "engine/3d.h"
#include "engine/igui.h"

#include "game.h"

#include "stdio.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"
#include "dirent.h"

char *ENTITY_TYPE_NAMES[] = {
	"player",
	"rock",
	"sticky rock",
	"obstacle",
	"goal",
	"level door",
};

enum CurrentEditingTool{
	EDITING_TOOL_PLACE,
	EDITING_TOOL_REMOVE,
	EDITING_TOOL_EDIT,
};

IGUI_TextInputData levelNameInput;
IGUI_TextInputData levelDoorNameInput;

enum CurrentEditingTool currentEditingTool = EDITING_TOOL_PLACE;

enum EntityType currentPlacingEntityType = ENTITY_TYPE_OBSTACLE;

size_t currentEditingEntityID = -1;

float MOVEMENT_SPEED = 0.4;

bool openingLevel = false;

void World_initEditorState(World *world_p){

	IGUI_TextInputData_init(&levelNameInput, world_p->currentLevel, strlen(world_p->currentLevel));
	IGUI_TextInputData_init(&levelDoorNameInput, "", 0);

	currentEditingEntityID = -1;

	if(Engine_fpsModeOn){
		Engine_setFPSMode(false);
	}

}

void World_editorState(World *world_p){

	if(!Engine_fpsModeOn){

		//tool gui
		if(IGUI_textButton_click("Remove", getVec2f(50, 50), 60, currentEditingTool == EDITING_TOOL_REMOVE)){
			currentEditingTool = EDITING_TOOL_REMOVE;
		}
		if(IGUI_textButton_click("Place", getVec2f(50, 150), 60, currentEditingTool == EDITING_TOOL_PLACE)){
			currentEditingTool = EDITING_TOOL_PLACE;
		}
		if(IGUI_textButton_click("Edit", getVec2f(50, 250), 60, currentEditingTool == EDITING_TOOL_EDIT)){
			currentEditingTool = EDITING_TOOL_EDIT;
		}

		if(currentEditingTool == EDITING_TOOL_PLACE){

			for(int i = 0; i < NUMBER_OF_ENTITY_TYPES; i++){
				if(IGUI_textButton_click(ENTITY_TYPE_NAMES[i], getVec2f(350, 50 + 70 * i), 60, currentPlacingEntityType == i)){
					currentPlacingEntityType = i;
				}
			}
			
		}

		if(currentEditingTool == EDITING_TOOL_EDIT
		&& currentEditingEntityID != -1){

			Entity *entity_p = Array_getItemPointerByID(&world_p->entities, currentEditingEntityID);

			if(entity_p->type == ENTITY_TYPE_LEVEL_DOOR){

				IGUI_textInput(getVec2f(300, 50), &levelDoorNameInput);

				String_set(entity_p->levelName, levelDoorNameInput.text, STRING_SIZE);

			}
		}

		//file gui
		if(IGUI_textButton_click("Save", getVec2f(WIDTH - 300, 50), 60, false)){
			World_writeLevelToFile(world_p, world_p->currentLevel);
		}
		
		if(IGUI_textButton_click("Open", getVec2f(WIDTH - 300, 150), 60, false)){
			openingLevel = true;
		}

		IGUI_textInput(getVec2f(WIDTH - 900, 50), &levelNameInput);

		String_set(world_p->currentLevel, levelNameInput.text, STRING_SIZE);

		if(openingLevel){

			int posX = WIDTH - 900;
			int posY = 150;

			char dirPath[STRING_SIZE];
			String_set(dirPath, "levels/", STRING_SIZE);

			DIR *dataDir = opendir(dirPath);
			struct dirent* dirEntry;

			while((dirEntry = readdir(dataDir)) != NULL){

				if(strcmp(dirEntry->d_name, ".") != 0
				&& strcmp(dirEntry->d_name, "..") != 0){

					char fileName[STRING_SIZE];
					String_set(fileName, dirEntry->d_name, STRING_SIZE);

					char path[STRING_SIZE];
					String_set(path, dirPath, STRING_SIZE);
					String_append(path, fileName);
					
					char levelName[STRING_SIZE];
					String_set(levelName, fileName, STRING_SIZE);
					memset(strrchr(levelName, *"."), *"\0", 1);

					if(IGUI_textButton_click(levelName, getVec2f(posX, posY), 60, false)){

						String_set(levelNameInput.text, levelName, STRING_SIZE);
						String_set(world_p->currentLevel, levelNameInput.text, STRING_SIZE);

						World_loadLevelFromFile(world_p, world_p->currentLevel);

						openingLevel = false;

						//edited = true;

						//Level_loadFromFile(&world_p->currentLevel, path);

						//String_set(levelTextInput.text, world_p->currentLevel.name, STRING_SIZE);

					}

					posY += 80;

					if(posY > HEIGHT - 80){
						posY = 150;
						posX += 200;
					}

				}
			
			}
		
		}

	}

	//handle movement and looking
	if(Engine_fpsModeOn){

		world_p->cameraRotation.x += -Engine_pointer.movement.x * CAMERA_MOVEMENT_SCALE;
		world_p->cameraRotation.y += -Engine_pointer.movement.y * CAMERA_MOVEMENT_SCALE;

		if(world_p->cameraRotation.y > M_PI / 2 - 0.01){
			world_p->cameraRotation.y = M_PI / 2 - 0.01;
		}
		if(world_p->cameraRotation.y < -(M_PI / 2 - 0.01)){
			world_p->cameraRotation.y = -(M_PI / 2 - 0.01);
		}

		Vec3f up = getVec3f(0, 1, 0);
		Vec3f cameraLeft = getCrossVec3f(world_p->cameraDirection, up);
		Vec3f cameraForward = getCrossVec3f(up, cameraLeft);
		Vec3f_normalize(&cameraLeft);

		if(Engine_keys[ENGINE_KEY_W].down){
			Vec3f_add(&world_p->cameraPos, getMulVec3fFloat(cameraForward, MOVEMENT_SPEED));
		}
		if(Engine_keys[ENGINE_KEY_S].down){
			Vec3f_add(&world_p->cameraPos, getMulVec3fFloat(cameraForward, -MOVEMENT_SPEED));
		}
		if(Engine_keys[ENGINE_KEY_A].down){
			Vec3f_add(&world_p->cameraPos, getMulVec3fFloat(cameraLeft, MOVEMENT_SPEED));
		}
		if(Engine_keys[ENGINE_KEY_D].down){
			Vec3f_add(&world_p->cameraPos, getMulVec3fFloat(cameraLeft, -MOVEMENT_SPEED));
		}
		if(Engine_keys[ENGINE_KEY_SPACE].down){
			Vec3f_add(&world_p->cameraPos, getMulVec3fFloat(up, MOVEMENT_SPEED));
		}
		if(Engine_keys[ENGINE_KEY_SHIFT].down){
			Vec3f_add(&world_p->cameraPos, getMulVec3fFloat(up, -MOVEMENT_SPEED));
		}

	}

	if(!levelNameInput.focused
	&& !levelDoorNameInput.focused){

		if(Engine_keys[ENGINE_KEY_G].downed){
			world_p->gameState = GAME_STATE_LEVEL;
			World_initLevelState(world_p);
			Engine_setFPSMode(false);
			return;
		}

		if(Engine_keys[ENGINE_KEY_C].downed){
			Engine_setFPSMode(!Engine_fpsModeOn);
		}

		if(Engine_keys[ENGINE_KEY_B].down){
			world_p->cameraPos = getVec3f(0, 10.0, -10.0);
			world_p->cameraRotation = getVec3f(M_PI / 2, -M_PI / 4, 0.0);
		}
	}

	if(Engine_fpsModeOn){

		bool foundPoint = false;
		Vec3f closestPoint;
		Vec3f closestNormal;
		Vec3f closestEntityPos;
		size_t foundEntityID;

		for(int i = 0; i < world_p->entities.length; i++){

			Vec2f normalizedMousePos = Engine_pointer.pos;
			normalizedMousePos.x /= (float)Engine_clientWidth / 2;
			normalizedMousePos.y /= (float)Engine_clientHeight / 2;
			normalizedMousePos.x -= 1.0;
			normalizedMousePos.y -= 1.0;

			//Vec2f_log(normalizedMousePos);

			Vec3f up = getVec3f(0.0, 1.0, 0.0);
			Vec3f cameraLeft = getCrossVec3f(world_p->cameraDirection, up);
			Vec3f cameraUp = getCrossVec3f(cameraLeft, world_p->cameraDirection);

			//Vec3f_normalize(&cameraLeft);
			//Vec3f_normalize(&cameraUp);

			Vec3f mouseDirection = world_p->cameraDirection;

			Vec3f_add(&mouseDirection, getMulVec3fFloat(cameraUp, normalizedMousePos.y));
			Vec3f_add(&mouseDirection, getMulVec3fFloat(cameraLeft, -normalizedMousePos.x));

			//Vec3f_normalize(&mouseDirection);

			//Vec3f_log(cameraDirection);
			//Vec3f_log(cameraLeft);
			//Vec3f_log(cameraUp);

			//Vec3f mouseRotation = cameraRotation;

			//mouseRotation.x += atan(normalizedMousePos.x);
			//mouseRotation.y += atan(normalizedMousePos.y);

			//Vec3f mouseDirection;
			//mouseDirection.y = sin(mouseRotation.y);
			//mouseDirection.x = cos(mouseRotation.x) * cos(mouseRotation.y);
			//mouseDirection.z = sin(mouseRotation.x) * cos(mouseRotation.y);
			//Vec3f_normalize(&mouseDirection);

			//Vec3f_log(mouseDirection);
			//Vec3f_log(cameraDirection);

			//Vec2f_log(normalizedMousePos);

			Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

			//setup matrices
			Mat4f cameraMat4f = getIdentityMat4f();
			Mat4f_mulByMat4f(&cameraMat4f, getLookAtMat4f(world_p->cameraPos, world_p->cameraDirection));

			Mat4f modelRotationMat4f = getIdentityMat4f();
			Mat4f_mulByMat4f(&modelRotationMat4f, getRotationMat4f(entity_p->rotation.x, entity_p->rotation.y, entity_p->rotation.z));

			Mat4f modelMat4f = getIdentityMat4f();
			Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(entity_p->pos.x * 2.0 * 0.6, entity_p->pos.y * 2.0 * 0.6, entity_p->pos.z * 2.0 * 0.6));
			Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(entity_p->scale));

			Mat4f perspectiveMat4f = getIdentityMat4f();
			Mat4f_mulByMat4f(&perspectiveMat4f, getPerspectiveMat4f(fov, (float)Engine_clientWidth / (float)Engine_clientHeight, far, near));

			//printf("---\n");

			//Vec3f_log(entity_p->pos);

			//printf("---\n");
			for(int j = 0; j < world_p->boxMeshNumberOfTriangles; j++){

				Vec3f t1 = world_p->boxMesh[j * 3 + 0];
				Vec3f t2 = world_p->boxMesh[j * 3 + 1];
				Vec3f t3 = world_p->boxMesh[j * 3 + 2];

				Vec3f_mulByMat4f(&t1, modelMat4f, 1.0);

				Vec3f_mulByMat4f(&t2, modelMat4f, 1.0);

				Vec3f_mulByMat4f(&t3, modelMat4f, 1.0);

				//Vec3f_log(t1);
				//Vec3f_log(t2);
				//Vec3f_log(t3);


				Vec3f l1 = world_p->cameraPos;
				Vec3f l2 = getAddVec3f(world_p->cameraPos, world_p->cameraDirection);
				//Vec3f l2 = getAddVec3f(cameraPos, mouseDirection);
				//Vec3f l1 = getVec3f(0, 0, 0);
				//Vec3f l2 = getVec3f(normalizedMousePos.x, normalizedMousePos.y, 1);

				Vec3f P;

				if(checkLineToTriangleIntersectionVec3f(l1, l2, t1, t2, t3, &P)){

					if(!foundPoint
					|| getMagVec3f(getSubVec3f(P, world_p->cameraPos)) < getMagVec3f(getSubVec3f(closestPoint, world_p->cameraPos))){

						foundPoint = true;

						closestPoint = P;
						closestNormal = getCrossVec3f(getSubVec3f(t1, t2), getSubVec3f(t1, t3));
						Vec3f_normalize(&closestNormal);
						closestEntityPos = entity_p->pos;

						foundEntityID = entity_p->header.ID;

					}

				}
			
			}

		}

		if(foundPoint
		&& Engine_pointer.downed){

			if(currentEditingTool == EDITING_TOOL_PLACE){

				if(currentPlacingEntityType == ENTITY_TYPE_PLAYER){
					World_addPlayer(world_p, getAddVec3f(closestEntityPos, closestNormal));
				}
				if(currentPlacingEntityType == ENTITY_TYPE_ROCK){
					World_addRock(world_p, getAddVec3f(closestEntityPos, closestNormal));
				}
				if(currentPlacingEntityType == ENTITY_TYPE_STICKY_ROCK){
					World_addStickyRock(world_p, getAddVec3f(closestEntityPos, closestNormal));
				}
				if(currentPlacingEntityType == ENTITY_TYPE_GOAL){
					World_addGoal(world_p, getAddVec3f(closestEntityPos, closestNormal), PLAYER_COLOR, ENTITY_TYPE_PLAYER);
				}
				if(currentPlacingEntityType == ENTITY_TYPE_OBSTACLE){
					World_addObstacle(world_p, getAddVec3f(closestEntityPos, closestNormal));
				}
				if(currentPlacingEntityType == ENTITY_TYPE_LEVEL_DOOR){
					World_addLevelDoor(world_p, getAddVec3f(closestEntityPos, closestNormal));
				}
			}

			if(currentEditingTool == EDITING_TOOL_REMOVE){
				Array_removeItemByID(&world_p->entities, foundEntityID);
			}

			if(currentEditingTool == EDITING_TOOL_EDIT){
	
				Entity *entity_p = Array_getItemPointerByID(&world_p->entities, foundEntityID);
				currentEditingEntityID = foundEntityID;

				if(entity_p->type == ENTITY_TYPE_LEVEL_DOOR){
					Engine_setFPSMode(false);
					String_set(levelDoorNameInput.text, entity_p->levelName, STRING_SIZE);
				}

			}

		}
	
	}

}
