#version 420 core
in GS_OUT {
    float segmentPos;
    float totalLength;
    float dashUnit;
    float dashLength;
    float gapLength;
    float clipEnd;
} fs_in;

out vec4 FragColor;

in GS_OUT {
    float segmentPos;
    float totalLength;
    float dashUnit;
    float dashLength;
    float gapLength;
} fs_in;

out vec4 FragColor;
// 计算当前片段在线条上的精确位置
float calculateExactPosition() {
    // 计算线段长度和片段在线段上的比例
    float segmentLength = fs_in.segmentPos[1] - fs_in.segmentPos[0];
    vec2 texCoord = gl_FragCoord.xy;  // 简化处理，实际需要更精确的插值
    
    // 计算片段在线段上的精确位置（简化版）
    return mix(fs_in.segmentPos[0], fs_in.segmentPos[1], texCoord.x);
}

void main() {
    // 计算当前片段在线条上的位置
    float exactPos = calculateExactPosition();
    
    // 计算在虚线单元中的位置
    float localPos = mod(exactPos, fs_in.dashUnit);
    
    // 虚线效果：间隙部分不绘制
    if (localPos > fs_in.dashLength) {
        discard;
    }
    
    // 绘制实线部分（添加轻微的透明度和颜色）
    FragColor = vec4(0.2, 0.5, 1.0, 0.9);
}