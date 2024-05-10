#version 460 core

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : require

layout(location = 0) flat in uint v_instance_id;
layout(location = 1) in vec4 v_color;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

layout(binding = 2, std430) readonly buffer SGpuSpriteTextureHandleBuffer
{
    sampler2D Textures[];
};

void main() {
    sampler2D tex = Textures[v_instance_id];
    vec3 t = texture(tex, v_uv).rgb;
    o_color = vec4(t, 1.0) * v_color;
}