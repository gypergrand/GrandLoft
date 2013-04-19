#pragma once
#include "vmath.h"
//#include "esUtil.h"
#include "../Utilities/utilities.h"

struct Vertex 
{
	VMATH::Vector3<GLfloat> pos, color, normal;
  GLfloat u,v;
};