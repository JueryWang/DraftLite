#version 420 core

layout (location = 0) in vec3 aPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float PointSize;

void main()
{
    gl_Position = projection * view * model * vec4(aPosition, 1.0f);
    gl_PointSize = PointSize; // 必须设置点大小才能正确显示
}