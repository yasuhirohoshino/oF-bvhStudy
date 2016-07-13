#version 330

#define PI 3.14159265358979

#pragma include "util.glsl"
#pragma include "noise.glsl"

in vec3 inPosition;
in vec3 inVelocity;
in float inAge;
in float inLifetime;

out vec3 outPosition;
out vec3 outVelocity;
out float outAge;
out float outLifetime;

uniform float time;
uniform float timestep;
uniform float scale;
uniform float maxLifetime;

uniform samplerBuffer joints;
uniform samplerBuffer prevJoints;

void main() {
    vec3 pos = inPosition;
    vec3 vel = inVelocity;
    float age = inAge;
    float lifetime = inLifetime;
    
    if(age >= lifetime){
        lifetime = 1.0 + maxLifetime * random(pos.xy);
        age = 0;
		int jointIndex = int(floor(random(pos.yz) * textureSize(joints)));
		vec3 joint = texelFetch(joints, jointIndex).xyz;
		vec3 prevJoint = texelFetch(prevJoints, jointIndex).xyz;
		pos = joint + normalize(vel.xyz) * 10.0;
        vel.xyz = normalize((prevJoint - joint) + normalize(vel.xyz));
    }else{
        vel.x += snoise(vec4(pos.x * scale, pos.y * scale, pos.z * scale, 0.1352 * time)) * timestep;
        vel.y += snoise(vec4(pos.x * scale, pos.y * scale, pos.z * scale, 1.2814 * time)) * timestep;
        vel.z += snoise(vec4(pos.x * scale, pos.y * scale, pos.z * scale, 2.5564 * time)) * timestep;
        pos += vel;
        age++;
    }
    
    outPosition = pos;
    outVelocity = vel;
    outAge = age;
    outLifetime = lifetime;
}