#version 420 core

out vec4 FragColor;
uniform vec4 PaintColor;

void main()
{
    // 将点坐标从[0,1]范围转换到[-1,1]范围
    vec2 pointCoord = gl_PointCoord * 2.0 - 1.0;
    float distance = length(pointCoord);

    // 丢弃圆外的像素
    if(distance > 1.0)
        discard;

    // 计算alpha值，创建柔和的边缘
    float alpha = 1.0 - smoothstep(0.95, 1.0, distance);
    FragColor = vec4(PaintColor.rgb, alpha * PaintColor.a);
}
