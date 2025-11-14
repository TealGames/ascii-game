#version 430 core

layout(std140) uniform ViewerBlock
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 worldPos;
    vec3 forwardDir;
    vec3 rightDir;
    vec3 upDir;
    float yFov;
} uViewerBlock;

out vec3 vWorldDir;

const vec3 positions[36] = vec3[](
    vec3(-1.0,  1.0, -1.0), vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0,  1.0), vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0), vec3(-1.0,  1.0,  1.0), vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0), vec3(-1.0,  1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3( 1.0, -1.0,  1.0), vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3(-1.0,  1.0,  1.0), vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0), vec3(-1.0, -1.0,  1.0), vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0), vec3(-1.0, -1.0,  1.0), vec3( 1.0, -1.0,  1.0)
);

void main()
{
    vec3 position = positions[gl_VertexID];
    vWorldDir = position;
    gl_Position = uViewerBlock.projectionMatrix * uViewerBlock.viewMatrix * vec4(position, 1.0);
}