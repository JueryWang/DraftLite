#version 420 core

layout (lines) in;
layout (line_strip, max_vertices = 32) out;

in VS_OUT {
    vec3 worldPos;
    float bulge;
} gs_in[2];

uniform mat4 projection;
uniform mat4 view;
uniform vec2 viewportSize;  // Viewport size (width, height), e.g., 800x600
uniform float minVisibleLength = 1.0;  // Minimum visible screen length (pixels), special handling when smaller
uniform float baseScreenDash = 3.0f;   // Base screen dash length (pixels)
uniform float baseScreenGap = 2.0f;    // Base screen gap length (pixels)

// Convert clip space to screen pixel coordinates
vec2 clipToScreen(vec4 clipPos) {
    vec3 ndc = clipPos.xyz / clipPos.w;
    return vec2(
        (ndc.x + 1.0) * 0.5 * viewportSize.x,
        (ndc.y + 1.0) * 0.5 * viewportSize.y
    );
}

void main() {
    // Get line segment world space coordinates
    vec3 startWorld = gs_in[0].worldPos;
    vec3 endWorld = gs_in[1].worldPos;
    vec3 dirWorld = endWorld - startWorld;
    float totalWorldLen = length(dirWorld);
    if (totalWorldLen <= 0.0) return;  // Avoid zero-length segments

    vec3 unitDir = dirWorld / totalWorldLen;

    // Calculate screen space length
    vec4 startClip = projection * view * vec4(startWorld, 1.0);
    vec4 endClip = projection * view * vec4(endWorld, 1.0);
    vec2 startScreen = clipToScreen(startClip);
    vec2 endScreen = clipToScreen(endClip);
    float totalScreenLen = length(endScreen - startScreen);

    // Handle extreme case: segment screen length is too short
    if (totalScreenLen < minVisibleLength) {
        float halfLen = totalWorldLen * 0.5;
        // Generate first solid segment
        vec3 v1 = startWorld;
        vec3 v2 = startWorld + unitDir * halfLen;
        gl_Position = projection * view * vec4(v1, 1.0);
        EmitVertex();
        gl_Position = projection * view * vec4(v2, 1.0);
        EmitVertex();
        EndPrimitive();
        return;
    }

    // -------------------------- Core Optimization --------------------------
    // 1. Calculate base period and theoretical segment count
    float baseRatio = baseScreenDash / (baseScreenDash + baseScreenGap);  // Dash ratio (key to visual consistency)
    float screenPeriodBase = baseScreenDash + baseScreenGap;
    float theoreticalSegmentCount = totalScreenLen / screenPeriodBase;  // Segment count without restrictions

    // 2. Calculate maximum allowed segment count (2 vertices per segment, reserve 2 vertices to prevent overflow)
    const int maxSegments = (32 - 2) / 2;  // 256 vertices → max 127 segments (with redundancy)

    // 3. Dynamically adjust period: if theoretical count exceeds max, recalculate period based on max count
    float actualPeriodMultiplier = 1.0;
    if (theoreticalSegmentCount > maxSegments) {
        actualPeriodMultiplier = theoreticalSegmentCount / maxSegments;  // Period scaling factor (maintain ratio)
    }

    // 4. Calculate final world space period and lengths (scaled proportionally, maintaining dash/gap ratio)
    float screenPeriod = screenPeriodBase * actualPeriodMultiplier;  // Adjusted screen period
    float worldPeriod = (totalWorldLen / totalScreenLen) * screenPeriod;  // World space period
    float worldDash = worldPeriod * baseRatio;  // Dash length (proportionally allocated)
    float worldGap = worldPeriod - worldDash;   // Gap length
    // -----------------------------------------------------------------------

    // Split segment into dashed lines (ensure not exceeding max_vertices)
    float currentPos = 0.0;
    int segmentCount = 0;
    while (currentPos < totalWorldLen && segmentCount < maxSegments) {
        float segmentEnd = currentPos + worldDash;
        if (segmentEnd > totalWorldLen) segmentEnd = totalWorldLen;

        // Output current dash segment
        vec3 v1 = startWorld + unitDir * currentPos;
        vec3 v2 = startWorld + unitDir * segmentEnd;
        gl_Position = projection * view * vec4(v1, 1.0);
        EmitVertex();
        gl_Position = projection * view * vec4(v2, 1.0);
        EmitVertex();
        EndPrimitive();

        // Move to next segment start
        currentPos = segmentEnd + worldGap;
        segmentCount++;
    }
}