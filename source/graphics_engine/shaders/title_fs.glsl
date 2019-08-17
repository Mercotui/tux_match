#ifdef GL_ES
    precision mediump int;
    precision mediump float;
#endif

uniform sampler2D tex_title;
in vec2 vtf_texcoord;

out highp vec4 frag_color;

void main()
{
    frag_color = texture(tex_title, vtf_texcoord);
}
