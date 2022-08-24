#include "engine/files.h"
#include "engine/shaders.h"

#include "stddef.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#ifdef _WIN32
#include "glad/wgl.h"
#endif

#include "glad/gl.h"

unsigned int getCompiledShader(char *shaderSourcePath, GLenum type){

	long int fileSize;
	char *shaderSource = getFileData_mustFree(shaderSourcePath, &fileSize);

	unsigned int shader;
	shader = glCreateShader(type);

	glShaderSource(shader, 1, (const GLchar * const *)&shaderSource, NULL);
	glCompileShader(shader);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("FAILED TO COMPILE SHADER: %s\n%s\n", shaderSourcePath, infoLog);
	}

	free(shaderSource);

	return shader;

}
