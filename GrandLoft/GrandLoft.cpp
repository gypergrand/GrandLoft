#include "stdafx.h"
#include "vmath.h"
#include "../Utilities/utilities.h" 
#include "Vertex.h"
#include "Shaders.h"
#include "Globals.h"
#include <conio.h>

GLuint vboId, ax_vbo;
Shaders myShaders;

static ESContext esContext;

static VMATH::Vector3<GLfloat> *v = 0;
static VMATH::Matrix4<GLfloat> projMat;
static VMATH::Matrix4<GLfloat> viewMat;
static VMATH::Matrix4<GLfloat> modelMat;

static GLfloat translateX = 0;

int Init(ESContext *esContext)
{
  glViewport(0, 0, (GLsizei)Globals::screenWidth, (GLsizei)Globals::screenHeight);
  GLfloat aspect = Globals::screenWidth > Globals::screenHeight ? Globals::screenWidth/Globals::screenHeight : Globals::screenHeight/Globals::screenWidth;
  projMat.perspective(90.0f, aspect, 0.1f, 10000.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepthf(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

	// triangle data (heap)
	Vertex verticesData[3], axisData[6];
	verticesData[0].pos.x =  0.0f;  verticesData[0].pos.y =  0.5f;  verticesData[0].pos.z = 0.0f;
	verticesData[1].pos.x = -0.5f;  verticesData[1].pos.y = -0.5f;  verticesData[1].pos.z = 0.0f;
	verticesData[2].pos.x =  0.5f;  verticesData[2].pos.y = -0.5f;  verticesData[2].pos.z = 0.0f;

  verticesData[0].color.r = 1.0f;  verticesData[0].color.g = 0.0f;  verticesData[0].color.b = 0.0f;
	verticesData[1].color.r = 0.0f;  verticesData[1].color.g = 1.0f;  verticesData[1].color.b = 0.0f;
	verticesData[2].color.r = 0.0f;  verticesData[2].color.g = 0.0f;  verticesData[2].color.b = 1.0f;

  axisData[0].pos.x = -1.0f;  axisData[0].pos.y =  0.0f;  axisData[0].pos.z =  0.0f;
	axisData[1].pos.x =  1.0f;  axisData[1].pos.y =  0.0f;  axisData[1].pos.z =  0.0f;
	axisData[2].pos.x =  0.0f;  axisData[2].pos.y = -1.0f;  axisData[2].pos.z =  0.0f;
  axisData[3].pos.x =  0.0f;  axisData[3].pos.y =  1.0f;  axisData[3].pos.z =  0.0f;
	axisData[4].pos.x =  0.0f;  axisData[4].pos.y =  0.0f;  axisData[4].pos.z = -1.0f;
	axisData[5].pos.x =  0.0f;  axisData[5].pos.y =  0.0f;  axisData[5].pos.z =  1.0f;

  axisData[0].color.r = 1.0f;  axisData[0].color.g = 0.0f;  axisData[0].color.b = 0.0f;
	axisData[1].color.r = 1.0f;  axisData[1].color.g = 0.0f;  axisData[1].color.b = 0.0f;
	axisData[2].color.r = 0.0f;  axisData[2].color.g = 1.0f;  axisData[2].color.b = 0.0f;
  axisData[3].color.r = 0.0f;  axisData[3].color.g = 1.0f;  axisData[3].color.b = 0.0f;
	axisData[4].color.r = 0.0f;  axisData[4].color.g = 0.0f;  axisData[4].color.b = 1.0f;
	axisData[5].color.r = 0.0f;  axisData[5].color.g = 0.0f;  axisData[5].color.b = 1.0f;

	glGenBuffers(1, &vboId); // buffer object name generation
	glBindBuffer(GL_ARRAY_BUFFER, vboId); // buffer object binding
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesData), verticesData, GL_STATIC_DRAW); // creation and initializion of buffer object storage
	glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &ax_vbo); // buffer object name generation
	glBindBuffer(GL_ARRAY_BUFFER, ax_vbo); // buffer object binding
	glBufferData(GL_ARRAY_BUFFER, sizeof(axisData), axisData, GL_STATIC_DRAW); // creation and initializion of buffer object storage
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// creation of shaders and program 
	return myShaders.Init("../Resources/Shaders/TriangleShaderVS.vs", "../Resources/Shaders/TriangleShaderFS.fs");
}

void Draw(ESContext *esContext)
{
	glClear(GL_COLOR_BUFFER_BIT); // || GL_DEPTH_BUFFER_BIT

  //viewMat.identity();
  //viewMat.translateX(translateX);

	glUseProgram(myShaders.program);

  if(myShaders.projectionUniform != -1)
	glUniformMatrix4fv(myShaders.projectionUniform, 1, GL_FALSE, (GLfloat*)&projMat);
  if(myShaders.viewUniform != -1)
	glUniformMatrix4fv(myShaders.viewUniform, 1, GL_FALSE, (GLfloat*)&viewMat);
  if(myShaders.modelUniform != -1)
	glUniformMatrix4fv(myShaders.modelUniform, 1, GL_FALSE, (GLfloat*)&modelMat);

  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  if(myShaders.positionAttribute != -1) // attribute passing to shader, for uniforms use glUniform1f(time, deltaT); glUniformMatrix4fv( m_pShader->matrixWVP, 1, false, (GLfloat *)&rotationMat );
	{   
		glEnableVertexAttribArray(myShaders.positionAttribute);
		glVertexAttribPointer(myShaders.positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), v);
	}
  if(myShaders.colorAttribute != -1)
	{   
		glEnableVertexAttribArray(myShaders.colorAttribute);
		glVertexAttribPointer(myShaders.colorAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), v+1);
	}
  
	glDrawArrays(GL_TRIANGLES, 0, 3);
  if(myShaders.positionAttribute != -1)
  glDisableVertexAttribArray(myShaders.positionAttribute);
  if(myShaders.colorAttribute != -1)
  glDisableVertexAttribArray(myShaders.colorAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, ax_vbo);
  if(myShaders.positionAttribute != -1)
	{   
		glEnableVertexAttribArray(myShaders.positionAttribute);
		glVertexAttribPointer(myShaders.positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), v);
	}
  if(myShaders.colorAttribute != -1)
	{   
		glEnableVertexAttribArray(myShaders.colorAttribute);
		glVertexAttribPointer(myShaders.colorAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), v+1);
	}
  glDrawArrays(GL_LINES, 0, 6);
  if(myShaders.positionAttribute != -1)
  glDisableVertexAttribArray(myShaders.positionAttribute);
  if(myShaders.colorAttribute != -1)
  glDisableVertexAttribArray(myShaders.colorAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

void Update(ESContext *esContext, float deltaTime)
{
  //modelMat.rotateZ(deltaTime*10);
  //modelMat.rotateX(deltaTime*5);  
}

void Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
  if (bIsPressed)
	{
    switch (key)
    {
      case VK_LEFT: viewMat.translateX(1); break;
      case VK_RIGHT: viewMat.translateX(-1); break;
      case VK_UP: viewMat.translateY(1); break;
      case VK_DOWN: viewMat.translateY(-1); break;
      case '+': viewMat.translateZ(1); break;
      case '-': viewMat.translateZ(-1); break;

      case 'W': viewMat.rotateX(1); break;
      case 'S': viewMat.rotateX(-1); break;
      case 'A': viewMat.rotateY(1); break;
      case 'D': viewMat.rotateY(-1); break;
      case 'Q': viewMat.rotateZ(1); break;
      case 'E': viewMat.rotateZ(-1); break;

      case VK_ESCAPE: esRelease(esContext);
    }
  }
}

void CleanUp()
{
	glDeleteBuffers(1, &vboId);
  glDeleteBuffers(1, &ax_vbo);
}

int _tmain(int argc, _TCHAR* argv[])
{ 
  esInitContext(&esContext);
	esCreateWindow(&esContext, "Hello Triangle", Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB | ES_WINDOW_DEPTH);
	if (Init(&esContext) != 0)
		return 0;
	esRegisterDrawFunc(&esContext, Draw );
	esRegisterUpdateFunc(&esContext, Update);
	esRegisterKeyFunc(&esContext, Key);
	esMainLoop(&esContext);
	CleanUp();  

  //VMATH::Matrix4d md;
  //md.translateX(55);
  //printf("Orthogonal factor: %d \n",md.isOrtogonal()); 

	// identifying memory leaks
	MemoryDump();
	printf("Press any key...\n");
	_getch();
	return 0;
}

