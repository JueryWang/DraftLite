#version 420 core
layout (lines) in; 
layout (line_strip, max_vertices = 6) out;

uniform float arrowSize = 0.01f;

out vec3 fragColor;

void main() {
    vec4 start = gl_in[0].gl_Position;
    vec4 end = gl_in[1].gl_Position;

    vec2 direction = normalize(end.xy - start.xy);
    vec2 normal = vec2(-direction.y, direction.x) * arrowSize;

    fragColor = vec3(1.0, 0.5, 0.0);

    gl_Position = start;
    EmitVertex();
    gl_Position = end;
    EmitVertex();
    EndPrimitive();

    gl_Position = end;
    EmitVertex();
    gl_Position = end - vec4(direction * arrowSize * 2.0 + normal,0.0f,0.0f);
    EmitVertex();
    EndPrimitive();

    gl_Position = end;
    EmitVertex();
    gl_Position = end - vec4(direction * arrowSize * 2.0 - normal,0.0f,0.0f);
    EmitVertex();
    EndPrimitive();

    EndPrimitive();
}