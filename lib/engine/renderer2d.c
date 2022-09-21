#include "engine/text.h"
#include "engine/renderer2d.h"
#include "engine/shaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

//#include "glad/glad.h"
#include "glad/wgl.h"
#include "glad/gl.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

//PRIVATE FUNCTIONS

char *readFile_mustFree(char *filePath){

	char *buffer = malloc(sizeof(char) * 1024);
	FILE *fd = NULL;
	long fileLength = 0;

	memset(buffer, 0, 1024);

	fd = fopen(filePath, "r");

	char c;
	while((c = fgetc(fd)) != EOF){
		buffer[fileLength] = c;
		fileLength++;
	}
	
	fclose(fd);

	return buffer;

}

//INIT FUNCTIONS

void Renderer2D_init(Renderer2D_Renderer *renderer_p, int width, int height){

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderer_p->width = width;
	renderer_p->height = height;

	static float rectangleVertices[] = {

		1, 1, 		1, 0,
		1, -1, 		1, 1,
		-1, -1, 	0, 1,

		1, 1, 		1, 0,
		-1, -1, 	0, 1,
		-1, 1, 		0, 0,

	};

	glGenBuffers(1, &renderer_p->rectangleTextureBufferID);

	glBindBuffer(GL_ARRAY_BUFFER, renderer_p->rectangleTextureBufferID);

	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &renderer_p->rectVAO);
	glBindVertexArray(renderer_p->rectVAO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	Font font = getFont("assets/fonts/times.ttf", 100);

	Renderer2D_Texture_initFromText(&renderer_p->textTexture, "", font);

	{
		Renderer2D_ShaderPathTypePair shaders[] = {
			"shaders/texture-vertex-shader.glsl", RENDERER2D_VERTEX_SHADER,
			"shaders/texture-fragment-shader.glsl", RENDERER2D_FRAGMENT_SHADER,
		};

		int shadersLength = sizeof(shaders) / sizeof(Renderer2D_ShaderPathTypePair);

		Renderer2D_ShaderProgram_init(&renderer_p->textureShaderProgram, "texture-shader", shaders, shadersLength);
	}

	{
		Renderer2D_ShaderPathTypePair shaders[] = {
			"shaders/color-vertex-shader.glsl", RENDERER2D_VERTEX_SHADER,
			"shaders/color-fragment-shader.glsl", RENDERER2D_FRAGMENT_SHADER,
		};

		int shadersLength = sizeof(shaders) / sizeof(Renderer2D_ShaderPathTypePair);

		Renderer2D_ShaderProgram_init(&renderer_p->colorShaderProgram, "color-shader", shaders, shadersLength);
	}
	
	{
		Renderer2D_ShaderPathTypePair shaders[] = {
			"shaders/circle-vertex-shader.glsl", RENDERER2D_VERTEX_SHADER,
			"shaders/circle-fragment-shader.glsl", RENDERER2D_FRAGMENT_SHADER,
		};

		int shadersLength = sizeof(shaders) / sizeof(Renderer2D_ShaderPathTypePair);

		Renderer2D_ShaderProgram_init(&renderer_p->circleShaderProgram, "circle-shader", shaders, shadersLength);
	}


}

void Renderer2D_Texture_init(Renderer2D_Texture *texture_p, char *name, char *data, int width, int height){

	String_set(texture_p->name, name, SMALL_STRING_SIZE);
	//texture_p->name = name;
	texture_p->width = width;
	texture_p->height = height;

	glGenTextures(1, &texture_p->ID);

	glBindTexture(GL_TEXTURE_2D, texture_p->ID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_p->width, texture_p->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}

void Renderer2D_Texture_initFromFile(Renderer2D_Texture *texture_p, char *path){

	int width, height, channels;
	char *data = stbi_load(path, &width, &height, &channels, 4);

	Renderer2D_Texture_init(texture_p, path, data, width, height);

	free(data);

}

void Renderer2D_Texture_initFromText(Renderer2D_Texture *texture_p, char *text, Font font){

	int width, height;
	char *data = getImageDataFromFontAndString_mustFree(font, text, &width, &height);

	Renderer2D_Texture_init(texture_p, text, data, width, height);

	free(data);
	
};

void Renderer2D_ShaderProgram_init(Renderer2D_ShaderProgram *shaderProgram_p, char *name, Renderer2D_ShaderPathTypePair *shaders, unsigned int shadersLength){

	shaderProgram_p->name = name;

	shaderProgram_p->ID = glCreateProgram();

	for(int i = 0; i < shadersLength; i++){

		unsigned int shaderID;
		
		if(shaders[i].type == RENDERER2D_VERTEX_SHADER){
			shaderID = getCompiledShader(shaders[i].path, GL_VERTEX_SHADER);
		}
		if(shaders[i].type == RENDERER2D_FRAGMENT_SHADER){
			shaderID = getCompiledShader(shaders[i].path, GL_FRAGMENT_SHADER);
		}

		glAttachShader(shaderProgram_p->ID, shaderID);

	}

	glLinkProgram(shaderProgram_p->ID);

}

//FREE FUNCTIONS
void Renderer2D_Texture_free(Renderer2D_Texture *texture_p){
	
	glDeleteTextures(1, &texture_p->ID);

}

//SETTINGS FUNCTIONS

void Renderer2D_updateDrawSize(Renderer2D_Renderer *renderer_p, int width, int height){

	float newWidth = width;
	float newHeight = height;

	float offsetX = 0;
	float offsetY = 0;

	float aspectRatio = (float)renderer_p->width / (float)renderer_p->height;

	if(newWidth / newHeight > aspectRatio){
		newWidth = height * aspectRatio;
	}else{
		newHeight = width / aspectRatio;
	}

	offsetX = (width - newWidth) / 2;
	offsetY = (height - newHeight) / 2;

	glViewport((int)offsetX, (int)offsetY, (int)newWidth, (int)newHeight);

}

//DRAWING FUNCTIONS

void Renderer2D_clear(Renderer2D_Renderer *renderer_p){

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);

}

void Renderer2D_setShaderProgram(Renderer2D_Renderer *renderer_p, Renderer2D_ShaderProgram shaderProgram){

	renderer_p->currentShaderProgram = shaderProgram;
	
	glUseProgram(shaderProgram.ID);
	
}

void Renderer2D_setTexture(Renderer2D_Renderer *renderer_p, Renderer2D_Texture texture){

	glBindTexture(GL_TEXTURE_2D, texture.ID);

}

void Renderer2D_beginRectangle(Renderer2D_Renderer *renderer_p, float x, float y, float width, float height){

	//glBindBuffer(GL_ARRAY_BUFFER, renderer_p->rectangleTextureBufferID);

	glBindVertexArray(renderer_p->rectVAO);

	unsigned int posXUniformLocation = glGetUniformLocation(renderer_p->currentShaderProgram.ID, "posX");
	unsigned int posYUniformLocation = glGetUniformLocation(renderer_p->currentShaderProgram.ID, "posY");
	unsigned int widthUniformLocation = glGetUniformLocation(renderer_p->currentShaderProgram.ID, "width");
	unsigned int heightUniformLocation = glGetUniformLocation(renderer_p->currentShaderProgram.ID, "height");

	glUniform1f(posXUniformLocation, 2 * ((float)x + renderer_p->offset.x) / (float)renderer_p->width);
	glUniform1f(posYUniformLocation, 2 * ((float)y + renderer_p->offset.y) / (float)renderer_p->height);
	glUniform1f(widthUniformLocation, (float)width / (float)renderer_p->width);
	glUniform1f(heightUniformLocation, (float)height / (float)renderer_p->height);

}

void Renderer2D_beginCircle(Renderer2D_Renderer *renderer_p, float x, float y, float radius){

}

void Renderer2D_supplyUniform(Renderer2D_Renderer *renderer_p, void *data_p, char *locationName, enum Renderer2D_UniformTypeEnum type){

	unsigned int location = glGetUniformLocation(renderer_p->currentShaderProgram.ID, locationName);

	if(type == RENDERER2D_UNIFORM_TYPE_INT){
		glUniform1i(location, *((int *)data_p));
	}
	if(type == RENDERER2D_UNIFORM_TYPE_FLOAT){
		glUniform1f(location, *((float *)data_p));
	}
	if(type == RENDERER2D_UNIFORM_TYPE_COLOR){

		Renderer2D_Color color = *(Renderer2D_Color *)data_p;
		
		glUniform3f(location, color.r, color.g, color.b);

	}

}

void Renderer2D_drawRectangle(Renderer2D_Renderer *renderer_p){

	glDrawArrays(GL_TRIANGLES, 0, 6);

}

void Renderer2D_beginText(Renderer2D_Renderer *renderer_p, char *text, int x, int y, int fontSize, Font font){

	glDeleteTextures(1, &renderer_p->textTexture.ID);

	Renderer2D_Texture_initFromText(&renderer_p->textTexture, text, font);

	int height = fontSize;
	int width = renderer_p->textTexture.width * fontSize / renderer_p->textTexture.height;

	Renderer2D_beginRectangle(renderer_p, x, y, width, height);

	Renderer2D_setTexture(renderer_p, renderer_p->textTexture);

}

Renderer2D_Color Renderer2D_getColor(float r, float g, float b){
	Renderer2D_Color color = { r, g, b };
	return color;
}

void Renderer2D_drawColoredRectangle(Renderer2D_Renderer *renderer_p, float x, float y, float w, float h, Renderer2D_Color color, float alpha){

	Renderer2D_setShaderProgram(renderer_p, renderer_p->colorShaderProgram);

	Renderer2D_beginRectangle(renderer_p, x, y, w, h);

	Renderer2D_supplyUniform(renderer_p, &color, "color", RENDERER2D_UNIFORM_TYPE_COLOR);

	Renderer2D_supplyUniform(renderer_p, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

	Renderer2D_drawRectangle(renderer_p);

}

void Renderer2D_drawText(Renderer2D_Renderer *renderer_p, char *text, float x, float y, int fontSize, Font font, float alpha){

	Renderer2D_setShaderProgram(renderer_p, renderer_p->textureShaderProgram);

	Renderer2D_beginText(renderer_p, text, x, y, fontSize, font);


	Renderer2D_supplyUniform(renderer_p, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

	Renderer2D_drawRectangle(renderer_p);

}

void Renderer2D_drawTexture(Renderer2D_Renderer *renderer_p, float x, float y, float width, float height, Renderer2D_Texture texture, float alpha){

	Renderer2D_setShaderProgram(renderer_p, renderer_p->textureShaderProgram);

	Renderer2D_beginRectangle(renderer_p, x, y, width, height);

	Renderer2D_setTexture(renderer_p, texture);

	Renderer2D_supplyUniform(renderer_p, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

	Renderer2D_drawRectangle(renderer_p);

}
