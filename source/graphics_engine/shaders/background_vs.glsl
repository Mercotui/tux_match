in vec2 position;
in vec2 tex;

uniform int score_mode;
uniform float score;

flat out int vtf_score_mode;
flat out float vtf_score;
out vec2 vtf_texcoord;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    vtf_texcoord = tex;
    vtf_score_mode = score_mode;
    vtf_score = score;
}
