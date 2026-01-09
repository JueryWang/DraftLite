#version 420 core

layout (location = 0) in vec3 aPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec2 p1;
uniform vec2 p2;

vec2 reflectPoint(vec2 point, float A, float B, float C)
{
    float denominator = A * A + B * B;
    if (denominator < 0.00001) {
        return point;
    }
    
    float factor = (A * point.x + B * point.y + C) / denominator;
    
    return vec2(point.x - 2 * A * factor,point.y - 2 * B * factor);
}

vec2 reflectPoint(vec2 point, vec2 linePoint1, vec2 linePoint2)
{
    float A = linePoint2.y - linePoint1.y;
    float B = linePoint1.x - linePoint2.x;
    float C = linePoint2.x * linePoint1.y - linePoint1.x * linePoint2.y;

    return reflectPoint(point, A, B, C);
}

void main()
{
    vec4 worldPos = model * vec4(aPosition, 1.0f);
    
    vec2 position2D = worldPos.xy;
    
    vec2 reflected = reflectPoint(position2D, p1, p2);

    gl_Position = projection * view * vec4(reflected,0.0f, 1.0f);
}