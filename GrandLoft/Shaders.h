#pragma once
#include "../Utilities/utilities.h" 
//#include "esUtil.h"

class Shaders 
{
public:
  GLuint program, vertexShader, fragmentShader;
  char fileVS[260];
  char fileFS[260];
  GLint positionAttribute;
  GLint colorAttribute;
  GLint worldUniform;
  GLint textureAttribute;
  GLint samplerUniform;
  GLint texturedUniform;
  int Init(char * fileVertexShader, char * fileFragmentShader);
  ~Shaders();
};