@vs vs
@glsl_options flip_vert_y

layout(location = 0) in vec4 a_pos;
layout(location = 1) in vec4 a_col;
layout(location = 2) in vec2 a_uv;

layout(binding = 0) uniform vs_params {
    mat4 mvp;
    vec4 scale;
};

out vec4 col;
out vec2 uv;

void main() {
    gl_Position = mvp * scale * a_pos;
    col = a_col;
    uv = a_uv;
}

@end

@fs fs

layout (location = 0) in vec4 col;
layout (location = 1) in vec2 uv;

layout (binding = 0) uniform sampler2D u_tex;

out vec4 frag_color;

void main() {
    frag_color = texture(u_tex, uv) * col;
}

@end

@program quad vs fs
