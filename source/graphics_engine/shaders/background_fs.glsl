#ifdef GL_ES
    precision mediump int;
    precision mediump float;
#endif

uniform sampler2D u_tex_background;
flat in int vtf_score_mode;
flat in float vtf_score;
in vec2 vtf_texcoord;

out highp vec4 frag_color;

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

vec4 map4(vec4 value, float min1, float max1, float min2, float max2) {
  vec4 ret;
  ret.r = map(value.r, min1, max1, min2, max2);
  ret.g = map(value.g, min1, max1, min2, max2);
  ret.b = map(value.b, min1, max1, min2, max2);
  ret.a = map(value.a, min1, max1, min2, max2);
  return ret;
}

void main()
{
    vec4 tex_sample = texture(u_tex_background, vtf_texcoord);
    vec2 range = vec2(0.0,1.0);
    if (vtf_score_mode == 1){
        if (vtf_texcoord.y < (1 - vtf_score)) {
          float grey = dot(tex_sample.rgb, vec3(0.299, 0.587, 0.114));
          grey = map(grey, 0.0, 1.0, 0.2, 0.8);
          frag_color = vec4(grey, grey, grey, 1.0);
        } else {
          frag_color = vec4(map4(tex_sample, 0.0, 1.0, 0.2, 0.9).rgb, 1.0);
        }

    }else{
      frag_color = tex_sample;
    }

}
