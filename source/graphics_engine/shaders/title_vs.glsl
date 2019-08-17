in vec2 pos;
in vec2 tex;
uniform mat4 transform;

out vec2 vtf_texcoord;

void main() {
    gl_Position = transform * vec4(pos, 0.0, 1.0);
    vtf_texcoord = tex;
}
