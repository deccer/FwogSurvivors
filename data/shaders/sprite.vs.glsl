#version 460 core

#extension GL_ARB_gpu_shader_int64 : enable

layout(location = 0) out flat uint v_instance_id;
layout(location = 1) out vec4 v_color;
layout(location = 2) out vec2 v_uv;

layout(binding = 0, std140) uniform SGpuCameraInformationBuffer
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
} camera_information;

struct SGpuSprite
{
    vec4 PositionAndRotation;
    vec4 Color;
};

layout(binding = 1, std430) readonly buffer SGpuSpriteBuffer
{
    SGpuSprite GpuSprites[];
};

void main() {

    vec2 positions[4] = vec2[](
        vec2(0.5, -0.5),        
        vec2(-0.5, -0.5),
        vec2(0.5, 0.5),
        vec2(-0.5, 0.5)
    );
    vec2 uvs[4] = vec2[](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    int vertex_id = gl_VertexID % 4;

    SGpuSprite sprite = GpuSprites[gl_InstanceID];
    v_instance_id = gl_InstanceID;
    v_color = sprite.Color;
    v_uv = uvs[vertex_id];

    gl_Position = camera_information.ProjectionMatrix *
                  camera_information.ViewMatrix *
                  vec4(positions[vertex_id] * 32.0f + sprite.PositionAndRotation.xy, -0.25, 1.0);
}