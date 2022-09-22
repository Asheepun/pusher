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

//#define PRINT_STUFF_LEVELS

void World_loadLevelFromFile(World *world_p, char *name){

	Array_clear(&world_p->entities);
	Array_clear(&world_p->lastEntities);
	for(int i = 0; i < world_p->undos.length; i++){
		Array_clear(Array_getItemPointerByIndex(&world_p->undos, i));
	}
	Array_clear(&world_p->undos);

	char path[STRING_SIZE];
	String_set(path, "levels/", STRING_SIZE);
	String_append(path, name);
	String_append(path, ".level");

	size_t dataSize;
	char *data = getFileData_mustFree(path, &dataSize);

	int numberOfLines = 1;
	for(int i = 0; i < dataSize; i++){
		if(data[i] == *"\n"){
			numberOfLines++;
		}
	}

	char (*lines)[STRING_SIZE] = calloc(numberOfLines, STRING_SIZE);
	memset(lines, 0, numberOfLines * STRING_SIZE);

	int currentLine = 0;
	int currentChar = 0;
	for(int i = 0; i < dataSize; i++){

		if(data[i] == *"\n"){
			currentLine++;
			currentChar = 0;
		}else{
			lines[currentLine][currentChar] = data[i];
			currentChar++;
		}

	}

	for(int i = 0; i < numberOfLines; i++){

		if(strcmp(lines[i], ":entity") == 0){

			char *next_p;

			Vec3f pos;
			pos.x = strtof(lines[i + 1], &next_p);
			pos.y = strtof(next_p + 1, &next_p);
			pos.z = strtof(next_p + 1, &next_p);

			char *modelName = lines[i + 2];

			float scale = strtof(lines[i + 3], &next_p);

			int entityType = strtol(lines[i + 4], &next_p, 10);

			Vec4f color;
			color.x = strtof(lines[i + 5], &next_p);
			color.y = strtof(next_p + 1, &next_p);
			color.z = strtof(next_p + 1, &next_p);
			color.w = strtof(next_p + 1, &next_p);

			char entityLevelName[STRING_SIZE];
			String_set(entityLevelName, lines[i + 6], STRING_SIZE);

#ifdef PRINT_STUFF_LEVELS
			printf("entity!\n");
			printf("pos:\n");
			Vec3f_log(pos);
			printf("modelName:\n%s\n", modelName);
			printf("scale:\n%f\n", scale);
			printf("entityType:\n%i\n", entityType);
			printf("color:\n");
			Vec4f_log(color);
#endif

			Model model = world_p->boxModel;

			Entity *entity_p = World_addEntity(world_p, pos, model, scale, entityType, color);
			String_set(entity_p->levelName, entityLevelName, STRING_SIZE);

		}

	}

	free(data);
	free(lines);

}

void World_writeLevelToFile(World *world_p, char *name){

	char *data = malloc(10 * STRING_SIZE * world_p->entities.length);
	memset(data, 0, 10 * STRING_SIZE * world_p->entities.length);

	for(int i = 0; i < world_p->entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		String_append(data, ":entity\n");

		String_append_float(data, entity_p->pos.x);
		String_append(data, " ");
		String_append_float(data, entity_p->pos.y);
		String_append(data, " ");
		String_append_float(data, entity_p->pos.z);
		String_append(data, "\n");

		String_append(data, "box");
		String_append(data, "\n");

		String_append_float(data, entity_p->scale);
		String_append(data, "\n");

		String_append_int(data, entity_p->type);
		String_append(data, "\n");

		String_append_float(data, entity_p->color.x);
		String_append(data, " ");
		String_append_float(data, entity_p->color.y);
		String_append(data, " ");
		String_append_float(data, entity_p->color.z);
		String_append(data, " ");
		String_append_float(data, entity_p->color.w);
		String_append(data, "\n");
		String_append(data, entity_p->levelName);
		String_append(data, "\n");

	}

#ifdef PRINT_STUFF_LEVELS
	printf("\n---\n\n", data);

	printf("%s\n", data);
#endif

	int dataSize = strlen(data);

	char path[STRING_SIZE];
	String_set(path, "levels/", STRING_SIZE);
	String_append(path, name);
	String_append(path, ".level");

	writeDataToFile(path, data, dataSize);
	
	free(data);

}
