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
uniform vec3 cameraPos;

uniform vec4 color;

float ambientLightComponent = 0.3;
float diffuseLightComponent = 0.7;

void main(){

	vec4 position = vec4(fragmentPosition.xyz, 1.0);
	vec4 normal = vec4(fragmentNormal.xyz, 1.0);
	vec4 modelNormal = normal * modelRotationMatrix;
	vec4 modelPosition = position * modelRotationMatrix * modelMatrix;
	vec4 projectedPosition = position * modelRotationMatrix * modelMatrix * cameraMatrix * perspectiveMatrix;
	vec4 lightProjectedPosition = position * modelRotationMatrix * modelMatrix * lightMatrix * perspectiveMatrix;

	lightProjectedPosition.x /= lightProjectedPosition.w;
	lightProjectedPosition.y /= lightProjectedPosition.w;

	projectedPosition.x /= projectedPosition.w;
	projectedPosition.y /= projectedPosition.w;

	vec4 color = color;
	float alpha = color.w;

	//color.x = fragmentPosition.x;
	//color.y = fragmentPosition.y;
	//color.z = 0;

	float depth = abs(projectedPosition.z / 100.0);

	float shadowMapDepth = texture(shadowMapTexture, vec2(0.5, 0.5) + lightProjectedPosition.xy / 2).x;
	float shadowDepth = lightProjectedPosition.z / 100.0;

	float diffuseLight = abs(dot(normalize(lightPos - modelPosition.xyz), normalize(modelNormal.xyz)));

	float shadowMargin = 0.005;
	//shadowMargin = 0.00001;
	//shadowMargin = 1.00;

	if(shadowDepth - shadowMargin > shadowMapDepth
	|| dot(normalize(modelNormal.xyz), normalize(modelPosition.xyz - lightPos)) > 0){
		diffuseLight = 0.0;
	}

	color *= ambientLightComponent + diffuseLight * diffuseLightComponent;

	//color = vec3(depth, depth, depth);
	//color = vec3(shadowMapDepth, shadowMapDepth, shadowMapDepth);
	//color = vec3(shadowDepth, shadowDepth, shadowDepth);
	//color = vec3(projectedPosition.x, projectedPosition.y, 0.0);

    FragColor = vec4(color.xyz, alpha);

    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);

	gl_FragDepth = depth;

} 
