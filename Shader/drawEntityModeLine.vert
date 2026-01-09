#version 420 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in float aBulge;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out VS_OUT{
    vec3 worldPos;
    float bulge;
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(aPosition, 1.0f);
    vs_out.worldPos = vec3(model * vec4(aPosition, 1.0f));
    vs_out.bulge = aBulge;
}