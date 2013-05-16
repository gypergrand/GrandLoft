#pragma once
#include "vmath.h"
//#include "esUtil.h"
#include "../Utilities/utilities.h"

struct Vertex 
{
  VMATH::Vector3<GLfloat> pos, color, normal, binormal, tgt;
  GLfloat u,v;
};

struct Index 
{
  VMATH::Vector3<GLushort> pos;
};

struct Model 
{
  Vertex* vert;
  Index* ind;
};