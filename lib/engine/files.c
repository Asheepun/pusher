#include "engine/files.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

char *getFileData_mustFree(char *path, long int *fileSizeOut){

	char *data = NULL;

	FILE *fileHandle = NULL;

	fileHandle = fopen(path, "r");
  
    fseek(fileHandle, 0L, SEEK_END);
    long int fileSize = ftell(fileHandle);
    fseek(fileHandle, 0L, 0);

	data = malloc(sizeof(char) * fileSize + 1);
	memset(data, 0, sizeof(char) * fileSize + 1);

	for(int i = 0; i < fileSize; i++){
		data[i] = fgetc(fileHandle);
	}

	fclose(fileHandle);

	*fileSizeOut = fileSize;

	return data;

}

void writeDataToFile(char *path, char *data_p, long int fileSize){

	FILE *fileHandle = NULL;

	fileHandle = fopen(path, "w");

	for(int i = 0; i < fileSize; i++){
		fputc(data_p[i], fileHandle);
	}
	
	fclose(fileHandle);

}
