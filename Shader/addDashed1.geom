#version 420 core
layout(lines) in;

uniform int toatalVertexCount;
uniform int dashCount;

out VS_OUT{
    vec3 worldPos;
    float cumulativeLength; // 累积长度
    int vertexID;
} gs_in[];

out GS_OUT{
    float segmentPos; //在当前线段上的位置
    float totalLength; //总长度
    float dashUnit;     //虚线单元长度
    float dashLength;   //间隙长度
} gs_out;

layout(std140,binding = 1) uniform CumulativeLengths{
    float lengths[5000];
};

void main()
{
    float totalLength = lengths[toatalVertexCount-1];

    float dashUnit = totalLength / dashCount;
    float dashLength = dashUnit * 0.6;
    float gapLength = dashUnit * 0.4;

    gs_out.totalLength = totalLength;
    gs_out.dashUnit = dashUnit;
    gs_out.dashLength = dashLength;
    gs_out.gapLength = gapLength;
    gs_out.clipEnd = clipEnd;

    // 处理第一个顶点
    gs_out.segmentPos = gs_in[0].cumulativeLength;
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    
    // 处理第二个顶点
    gs_out.segmentPos = gs_in[1].cumulativeLength;
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    
    EndPrimitive();
}



