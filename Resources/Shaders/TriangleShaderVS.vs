attribute vec3 a_posL;
attribute vec3 a_color;
varying vec3 v_color;
uniform mat4 u_world;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main()
{
  vec4 posL = vec4(a_posL, 1.0);
  v_color = a_color;
  v_texCoord = a_texCoord;
  gl_Position = u_world * posL; 
}