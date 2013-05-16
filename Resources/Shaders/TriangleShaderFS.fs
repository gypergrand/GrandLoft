precision mediump float;
varying vec3 v_color;
varying vec2 v_texCoord;
uniform int u_textured;
uniform sampler2D s_texture;
void main()
{
  vec4 color = vec4(v_color, 1.0);
  if (u_textured==0)
  gl_FragColor = color; else
  gl_FragColor = texture2D(s_texture, v_texCoord);    
}