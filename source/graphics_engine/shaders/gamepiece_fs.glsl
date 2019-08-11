#ifdef GL_ES
    precision mediump int;
    precision mediump float;
#endif

uniform sampler2D u_tex_background;
flat in int vtf_is_gold;
in vec2 vtf_texcoord;

out highp vec4 frag_color;

void main()
{
    vec4 tex_sample = texture(u_tex_background, vtf_texcoord);
    /* if vtf_is_gold == 1)
    float luminance = dot(tex_sample.rgb, vec3(0.2125, 0.7154, 0.0721));
        frag_color = vec4(luminance, luminance, luminance, tex_sample.r);
    else { */
        frag_color = tex_sample;
    /* } */
}
