attribute vec3 a_posL;
attribute vec3 a_color;
varying vec3 v_color;
uniform mat4 a_projection;
uniform mat4 a_view;
uniform mat4 a_model;
void main()
{
  vec4 posL = vec4(a_posL, 1.0);
  v_color = a_color;
  gl_Position = a_projection * a_view * a_model * posL;
}
   