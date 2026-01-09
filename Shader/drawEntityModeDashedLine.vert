#version 420 core
layout (location = 0) in vec3 aPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int toatalVertexCount;

out VS_OUT{
    vec3 worldPos;
    float cumulativeLength; // 累积长度
    int vertexID;
} vs_out;

layout(std140,binding = 0) uniform VertexPositions{
    vec3 positions[5000];
};

layout(std140,binding = 1) uniform CumulativeLengths{
    float lengths[5000];
};


void main()
{
    vec3 worldPos = vec3(model * vec4(aPosition,1.0));
    positions[gl_VertexID] = worldPos;

    if(gl_VertexID == 0)
    {
        lengths[gl_VertexID] = 0.0f;
    }else
    {
        vec3 prePos = positions[gl_VertexID - 1];
        lengths[gl_VertexID] = lengths[gl_VertexID - 1] + distance(worldPos,prePos);
    }

    vs_out.worldPos = worldPos;
    vs_out.cumulativeLength = lengths[gl_VertexID];
    vs_out.vertexID = gl_VertexID;

    gl_Position = projection * view * model * vec4(aPosition, 1.0f);
}