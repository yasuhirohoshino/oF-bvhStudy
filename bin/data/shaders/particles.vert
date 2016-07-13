#version 330

#define MAX_LIGHTS 8
#pragma include "util.glsl"

// default
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

uniform samplerBuffer posTex;
uniform samplerBuffer velTex;
uniform samplerBuffer ageTex;
uniform samplerBuffer lifetimeTex;
uniform vec3 maxSize;

// passThrough
// in
in vec4 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color;
// out
out vec4 positionVarying;
out vec3 normalVarying;
out vec2 texCoordVarying;
out vec4 colorVarying;

out mat4 normalMatrix;

void main() {
	int coord = gl_InstanceID;
	vec3 inPosition = texelFetch(posTex, coord).xyz;
	vec3 inVelocity = texelFetch(velTex, coord).xyz;
	float inAge = texelFetch(ageTex, coord).x;
	float inLifetime = texelFetch(lifetimeTex, coord).x;

	float life = (inAge / max(inLifetime, 1.0));
	vec3 size = vec3(0.0);
	if(life <= 0.1){
		size = map(vec3(life), vec3(0.0), vec3(0.1), vec3(0.0), maxSize);
	}else{
		size = map(vec3(life), vec3(0.1), vec3(1.0), maxSize, vec3(0.0));
	}
	vec3(1.0 - (inAge / inLifetime));
	mat4 lookAt = makeLookAt(inVelocity.xyz, vec3(0.0), vec3(0.0, 1.0, 0.0));
    normalMatrix = inverse(transpose(modelViewMatrix));
	normalVarying = mat3(lookAt) * normal;
    positionVarying = vec4(vec3(lookAt * (vec4(size, 1.0) * position + vec4(0.0, 0.0, maxSize.z / 2.0, 0.0))) + inPosition, 1.0);
    texCoordVarying = texcoord; 
    colorVarying = color;
    gl_Position = modelViewProjectionMatrix * positionVarying;
}