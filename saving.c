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

void SaveData_load(SaveData *saveData_p){

	Array_clear(&saveData_p->completedLevels);

	int numberOfLines;
	FileLine *lines = getFileLines_mustFree("saveData.txt", &numberOfLines);

	char *next_p;

	for(int i = 0; i < numberOfLines; i++){
		
		if(strcmp(lines[i], ":playerPos") == 0){
			saveData_p->playerPos.x = strtof(lines[i + 1], &next_p);
			saveData_p->playerPos.y = strtof(next_p + 1, &next_p);
			saveData_p->playerPos.z = strtof(next_p + 1, &next_p);
		}

		if(strcmp(lines[i], ":completedLevels") == 0){
			for(int j = 1; j < numberOfLines - i; j++){

				if(*lines[i + j] == *":"
				|| *lines[i + j] == *"\n"
				|| *lines[i + j] == *""){
					break;
				}

				char *levelName = Array_addItem(&saveData_p->completedLevels);
				String_set(levelName, lines[i + j], STRING_SIZE);


			}
		}

	}

	for(int i = 0; i < saveData_p->completedLevels.length; i++){
		char *levelName = Array_getItemPointerByIndex(&saveData_p->completedLevels, i);
		//printf("%s\n", levelName);
	}

}

void SaveData_write(SaveData *saveData_p){



}
