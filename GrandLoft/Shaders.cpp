#include "stdafx.h"
#include "Shaders.h"

int Shaders::Init(char * fileVertexShader, char * fileFragmentShader)
{
	vertexShader = esLoadShader(GL_VERTEX_SHADER, fileVertexShader);
	if (vertexShader == 0)
		return -1;
	fragmentShader = esLoadShader(GL_FRAGMENT_SHADER, fileFragmentShader);
	if (fragmentShader == 0)
	{
		glDeleteShader(vertexShader);
		return -2;
	}
	program = esLoadProgram(vertexShader, fragmentShader);
	positionAttribute = glGetAttribLocation(program, "a_posL");
  colorAttribute = glGetAttribLocation(program, "a_color");
  worldUniform = glGetUniformLocation(program, "u_world");
  textureAttribute = glGetAttribLocation(program, "a_texCoord");
	return 0;
}

Shaders::~Shaders()
{
	glDeleteProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}