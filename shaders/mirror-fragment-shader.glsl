#version 330 core
in vec3 fragmentPosition;
in vec3 fragmentNormal;

out vec4 FragColor;

uniform sampler2D shadowMapTexture;

uniform mat4 modelMatrix;
uniform mat4 modelRotationMatrix;
uniform mat4 cameraMatrix;
uniform mat4 perspectiveMatrix;
uniform mat4 lightMatrix;

uniform vec3 lightPos;

void main(){

	vec4 normal = vec4(fragmentNormal.xyz, 1.0);
	vec4 position = vec4(fragmentPosition.xyz, 1.0);

	vec4 modelNormal = normal * modelRotationMatrix;
	vec4 modelPosition = position * modelRotationMatrix * modelMatrix;
	vec4 projectedPosition = position * modelRotationMatrix * modelMatrix * cameraMatrix * perspectiveMatrix;

	float depth = abs(projectedPosition.z / 100);

	float alpha = 1.0;

    FragColor = vec4(0.0, 1.0, 1.0, alpha);

	gl_FragDepth = depth;

} 
