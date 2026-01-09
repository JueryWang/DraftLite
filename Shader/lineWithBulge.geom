#version 420 core
//有精度损失
layout (lines) in;
layout (line_strip, max_vertices = 256) out;

in VS_OUT {
    vec3 worldPos;
    float bulge;
} gs_in[2];

// 声明MVP矩阵
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct ArcParams {
    vec3 center;       // 圆心
    float startAngle;  // 起始角度（度）
    float endAngle;    // 终止角度（度）
};

ArcParams CalculateArcParamsByBulge(vec3 S, vec3 E, float bulge) {
   ArcParams result;
    const float PI = 3.141592653589793;

    // 计算两点间的delta和距离
    float dx = E.x - S.x;
    float dy = E.y - S.y;
    float L = sqrt(dx * dx + dy * dy);  // 两点距离（实际未使用，保留原逻辑）

    // 计算角度theta
    float theta = 4.0 * atan(abs(bulge));  // GLSL中atan接受单参数时表示atan(y/x)的简化

    // 计算圆心坐标
    float b = bulge;
    // 避免除以0（当bulge为0时视为直线，圆心设为中点）
    if (b == 0.0) {
        result.center = (S + E) * 0.5;
    } else {
        result.center.x = (S.x + E.x) * 0.5 - (dy * 0.5) * (1.0 - b * b) / (2.0 * b);
        result.center.y = (S.y + E.y) * 0.5 + (dx * 0.5) * (1.0 - b * b) / (2.0 * b);
    }
    result.center.z = 0.0;  // 假设2D场景，z坐标设为0

    // 计算起始角和终止角（弧度）
    float dx_s = S.x - result.center.x;
    float dy_s = S.y - result.center.y;
    float startAngleRad = atan(dy_s, dx_s);  // GLSL中atan(y, x)更准确

    float dx_e = E.x - result.center.x;
    float dy_e = E.y - result.center.y;
    float endAngleRad = atan(dy_e, dx_e);

    // 处理顺时针/逆时针方向（根据bulge符号）
    if (bulge < 0.0) {
        // 顺时针：交换起始角和终止角
        float temp = startAngleRad;
        startAngleRad = endAngleRad;
        endAngleRad = temp;
        // 确保角度差为负（顺时针）
        if (endAngleRad < startAngleRad) {
            endAngleRad += 2.0 * PI;
        }
    } else {
        // 逆时针：确保终止角大于起始角
        if (endAngleRad < startAngleRad) {
            endAngleRad += 2.0 * PI;
        }
    }

    // 转换为角度（度）
    result.startAngle = startAngleRad * 180.0 / PI;
    result.endAngle = endAngleRad * 180.0 / PI;

    return result;
}

void main(){
    vec3 startPos = gs_in[0].worldPos;
    vec3 endPos = gs_in[1].worldPos;
    float bulge = gs_in[0].bulge;

    ArcParams arc = CalculateArcParamsByBulge(startPos, endPos, bulge);
    float radius = length(startPos - arc.center);
    
    // 计算起点的3维坐标
    vec3 startArcPos3D = vec3(
        arc.center.x + radius * cos(radians(arc.startAngle)), 
        arc.center.y + radius * sin(radians(arc.startAngle)),
        0.0
    );
    // 构造4维向量并赋值给gl_Position
    gl_Position = projection * view * vec4(startArcPos3D, 1.0);
    
    // 计算终点的3维坐标
    vec3 endArcPos3D = vec3(
        arc.center.x + radius * cos(radians(arc.endAngle)), 
        arc.center.y + radius * sin(radians(arc.endAngle)),
        0.0
    );
    // 构造4维向量并赋值给gl_Position
    gl_Position = projection * view * vec4(endArcPos3D, 1.0);
    
    if(abs(bulge) < 0.0001) {
        // 直线段：应用MVP变换
        gl_Position = projection * view * vec4(startArcPos3D, 1.0);
        EmitVertex();
        gl_Position = projection * view * vec4(endArcPos3D, 1.0);
        EmitVertex();
        EndPrimitive();
        return;
    }
    
    int count = 0;

    float angle = arc.startAngle;
    float step = (arc.endAngle - arc.startAngle)/254.0f;
    
    gl_Position = projection * view * vec4(startArcPos3D, 1.0);
    EmitVertex();

    while(angle <= arc.endAngle) { // 包含终点
        
        angle += step;
        float rad = radians(angle);

        vec3 pointOnArc;
        pointOnArc.x = arc.center.x + radius * cos(rad);
        pointOnArc.y = arc.center.y + radius * sin(rad);
        pointOnArc.z = 0.0;
        // 应用MVP变换
        gl_Position = projection * view * vec4(pointOnArc, 1.0);
        EmitVertex();
    }

    // 绘制终点
    gl_Position = projection * view * vec4(endArcPos3D, 1.0);
    EmitVertex();

    EndPrimitive();
}