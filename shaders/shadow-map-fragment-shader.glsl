#version 330 core
in vec3 fragmentPosition;
in vec3 fragmentNormal;

out vec4 FragColor;

uniform mat4 modelMatrix;
uniform mat4 modelRotationMatrix;
uniform mat4 cameraMatrix;
uniform mat4 perspectiveMatrix;

uniform vec3 lightPos;

float ambientLightComponent = 0.1;
float diffuseLightComponent = 0.9;

void main(){

	vec4 normal = vec4(fragmentNormal.xyz, 1.0);
	vec4 position = vec4(fragmentPosition.xyz, 1.0);

	vec4 modelNormal = normal * modelRotationMatrix;
	vec4 modelPosition = position * modelRotationMatrix * modelMatrix;
	vec4 projectedPosition = position * modelRotationMatrix * modelMatrix * perspectiveMatrix;

	float depth = 1 - 1 / projectedPosition.z;

	gl_FragDepth = depth;

	FragColor = vec4(depth, depth, depth, 1.0);

} 

