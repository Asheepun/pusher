#ifndef FILES_H_
#define FILES_H_

#include "engine/strings.h"

typedef char FileLine[STRING_SIZE];

char *getFileData_mustFree(char *, long int *);

FileLine *getFileLines_mustFree(char *, int *);

void writeDataToFile(char *, char *, long int);

#endif
