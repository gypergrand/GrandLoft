#pragma once
#include "vmath.h"

struct Vertex 
{
	VMATH::Vector3<GLfloat> pos, color, normal;
  GLfloat u,v;
};