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
uniform vec3 treePos;
uniform vec3 cameraPos;

uniform int playerDimension;
uniform int modelDimension;

float ambientLightComponent = 0.1;
float diffuseLightComponent = 0.9;

void main(){


	vec4 normal = vec4(fragmentNormal.xyz, 1.0);
	vec4 position = vec4(fragmentPosition.xyz, 1.0);

	vec4 modelNormal = normal * modelRotationMatrix;
	vec4 modelPosition = position * modelRotationMatrix * modelMatrix;
	vec4 projectedPosition = position * modelRotationMatrix * modelMatrix * cameraMatrix * perspectiveMatrix;
	vec4 lightProjectedPosition = vec4(lightPos, 1.0) * modelRotationMatrix * modelMatrix * lightMatrix * perspectiveMatrix;

	projectedPosition.x /= projectedPosition.w;
	projectedPosition.y /= projectedPosition.w;

	if(cameraPos.z < treePos.z){
		if(cameraPos.x > treePos.x
		&& (modelDimension != playerDimension
		&& modelDimension != playerDimension - 1
		|| cross(treePos - cameraPos, modelPosition.xyz - cameraPos).y > 0
		&& modelDimension != playerDimension
		|| cross(treePos - cameraPos, modelPosition.xyz - cameraPos).y < 0
		&& modelDimension != playerDimension - 1)){
			discard;
		}
		if(cameraPos.x < treePos.x
		&& (modelDimension != playerDimension
		&& modelDimension != playerDimension + 1
		|| cross(treePos - cameraPos, modelPosition.xyz - cameraPos).y < 0
		&& modelDimension != playerDimension
		|| cross(treePos - cameraPos, modelPosition.xyz - cameraPos).y > 0
		&& modelDimension != playerDimension + 1)){
			discard;
		}
	}

	//lightProjectedPosition.y /= lightProjectedPosition.w;
	//lightProjectedPosition.y /= lightProjectedPosition.w;

	vec3 color = vec3(1.0, 1.0, 1.0);

	float depth = abs(projectedPosition.z / 100);

	float shadowMapDepth = texture(shadowMapTexture, vec2(0.5, 0.5) + lightProjectedPosition.xy / lightProjectedPosition.w / 2).x;
	float shadowDepth = lightProjectedPosition.z / 100;
	//float shadowMapDepth = texture(shadowMapTexture, gl_FragCoord.xy).x;

	float diffuseLight = abs(dot(normalize(lightPos - modelPosition.xyz), normalize(modelNormal.xyz)));

	if(shadowDepth - 0.002 > shadowMapDepth
	|| dot(normalize(modelNormal.xyz), normalize(modelPosition.xyz - lightPos)) > 0){
		diffuseLight = 0.0;
	}

	color *= ambientLightComponent + diffuseLight * diffuseLightComponent;

	//color = vec3(depth, depth, depth);
	//color = vec3(shadowMapDepth, shadowMapDepth, shadowMapDepth);
	//color = vec3(projectedPosition.x, projectedPosition.y, 0.0);

	float alpha = 1.0;

    FragColor = vec4(color.xyz, alpha);

	gl_FragDepth = depth;

} 
