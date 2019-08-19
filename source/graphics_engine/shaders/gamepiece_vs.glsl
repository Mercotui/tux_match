in vec2 position;
in vec3 offset;
in vec2 tex;

uniform int is_gold;
uniform mat4 transform;

flat out int vtf_is_gold;
out vec2 vtf_texcoord;

void main()
{
    gl_Position = transform * vec4(position + offset.xy, offset.z, 1.0);
    vtf_texcoord = tex;
    vtf_is_gold = is_gold;
}
