#version 330 core
layout (location = 0) in vec3 attribute_vertexPosition;
layout (location = 1) in vec3 attribute_vertexNormal;

out vec3 fragmentPosition;
out vec3 fragmentNormal;

uniform mat4 modelMatrix;
uniform mat4 modelRotationMatrix;
uniform mat4 cameraMatrix;
uniform mat4 perspectiveMatrix;
uniform mat4 lightMatrix;

void main(){

	fragmentPosition = attribute_vertexPosition;
	fragmentNormal = attribute_vertexNormal;

	vec4 vertexPosition = vec4(attribute_vertexPosition.xyz, 1.0);

	vec4 projectedPosition = vertexPosition * modelRotationMatrix * modelMatrix * cameraMatrix * perspectiveMatrix;

	gl_Position = projectedPosition;

}
