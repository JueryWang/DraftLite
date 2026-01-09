#version 420 core
out vec4 FragColor;

uniform vec4 PaintColor;

void main()
{
    FragColor = PaintColor;
}