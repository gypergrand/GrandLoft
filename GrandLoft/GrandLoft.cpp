#include "stdafx.h"
#include "vmath.h"
#include "../Utilities/utilities.h" 
#include "Vertex.h"
#include "Shaders.h"
#include "Globals.h"
//#include "esUtil.h"
#include <stdio.h>
#include <conio.h>
#include <windows.h>

GLuint vboId, ax_vbo;
Shaders myShaders;
static ESContext esContext;
static VMATH::Vector3<GLfloat> *v = 0;
static VMATH::Matrix4<GLfloat> projection, modelview, world;
static GLfloat translateX = 0, translateY = 0, translateZ = 0, 
               rotateX = 0, rotateY = 0, rotateZ = 0, 
               scaleX = 1, scaleY = 1, scaleZ = 1,
               fovy = 90.0f, zNear = 0.01f, zFar = 10.0f, scl=0.1f;

#pragma region funcs

enum ConsoleColor
{
    Black         = 0,
    Blue          = 1,
    Green         = 2,
    Cyan          = 3,
    Red           = 4,
    Magenta       = 5,
    Brown         = 6,
    LightGray     = 7,
    DarkGray      = 8,
    LightBlue     = 9,
    LightGreen    = 10,
    LightCyan     = 11,
    LightRed      = 12,
    LightMagenta  = 13,
    Yellow        = 14,
    White         = 15
};

void SetColor(int text, int background)
{
   HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}

void wherexy(int& x, int& y){ 
	CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE); 
	if (!GetConsoleScreenBufferInfo(hStd, &screenBufferInfo)) 
		printf("GetConsoleScreenBufferInfo (%d)\n", GetLastError()); 
	x = screenBufferInfo.dwCursorPosition.X; 
	y = screenBufferInfo.dwCursorPosition.Y;
}

void gotoxy(int x,int y){
	COORD c={x,y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),c);
}

void printstat()
{
  int lx, ly;
  wherexy(lx,ly);
  SetColor(LightRed,Black);
  gotoxy(34,1);
  printf("Translate X: %f", translateX);
  gotoxy(59,1);
  printf("Rotate X: %f", rotateX);
  gotoxy(38,5);
  printf("Scale X: %f", scaleX);

  SetColor(LightGreen,Black);
  gotoxy(34,2);
  printf("Translate Y: %f", translateY);  
  gotoxy(59,2);
  printf("Rotate Y: %f", rotateY);
  gotoxy(38,6);
  printf("Scale Y: %f", scaleY);

  SetColor(LightCyan,Black);
  gotoxy(34,3);
  printf("Translate Z: %f", translateZ);
  gotoxy(59,3);
  printf("Rotate Z: %f", rotateZ); 
  gotoxy(38,7);
  printf("Scale Z: %f", scaleZ);

  SetColor(Yellow,Black);
  gotoxy(59,5);
  printf("Fovy: %f", fovy);
  gotoxy(59,6);
  printf("Near: %f", zNear);
  gotoxy(59,7);
  printf("Far: %f", zFar);

  SetColor(LightMagenta,Black);
  gotoxy(0,0);
  printf("[Controls]:\n");
  printf("Rotation [X]: [W,S]\n");
  printf("Rotation [Y]: [A,D]\n"); 
  printf("Rotation [Z]: [Q,E]\n"); 
  printf("Translation [X]: [Left,Right]\n");
  printf("Translation [Y]: [Up,Down]\n"); 
  printf("Translation [Z]: [PageUp,PageDown]\n"); 
  printf("Scale [X]: [U,J]\n");
  printf("Scale [Y]: [I,K]\n");
  printf("Scale [Z]: [O,L]\n");
  printf("Fovy: [R,F]\n");
  printf("Near: [T,G]\n");
  printf("Far: [Y,H]\n");

  if (ly<14) gotoxy(lx,14); else
  gotoxy(lx,ly);
  SetColor(White,Black);
}

#pragma endregion

void ResizeScreen(GLsizei screenWidth, GLsizei screenHeight)
{
  glViewport(0, 0, screenWidth, screenHeight);
  GLfloat aspect = screenWidth > screenHeight ? (GLfloat)screenWidth/(GLfloat)screenHeight : (GLfloat)screenHeight/(GLfloat)screenWidth;
  projection.identity();
  projection.perspective(fovy, aspect, zNear, zFar);
}

int Init(ESContext *esContext)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepthf(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  ResizeScreen(Globals::screenWidth, Globals::screenHeight);

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
	axisData[4].color.r = 0.0f;  axisData[4].color.g = 1.0f;  axisData[4].color.b = 1.0f;
	axisData[5].color.r = 0.0f;  axisData[5].color.g = 1.0f;  axisData[5].color.b = 1.0f;

	glGenBuffers(1, &vboId); 
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesData), verticesData, GL_STATIC_DRAW); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &ax_vbo); 
	glBindBuffer(GL_ARRAY_BUFFER, ax_vbo); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(axisData), axisData, GL_STATIC_DRAW); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return myShaders.Init("../Resources/Shaders/TriangleShaderVS.vs", "../Resources/Shaders/TriangleShaderFS.fs");
}

void Draw(ESContext *esContext)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(myShaders.program);
 
  world.identity();
  //world=projection;
 // world.ortho(0, 0, (GLsizei)Globals::screenWidth, (GLsizei)Globals::screenHeight, zNear, zFar);
  world.rotateX(45.0);
  world.rotateY(-30.0);
  //world.rotateZ(-45.0);
  //world.scale(scl,scl,scl);
  //VMATH::Vector3<GLfloat> eye(0,1,-1), up(0,1,0), center(0,0,0);
  //world.lookAt(eye, center, up);

  world=world*projection;

  // Вывод осей координат
  {
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
  if(myShaders.worldUniform != -1)
	glUniformMatrix4fv(myShaders.worldUniform, 1, GL_FALSE, (GLfloat*)&world);
  glDrawArrays(GL_LINES, 0, 6);
  if(myShaders.positionAttribute != -1)
  glDisableVertexAttribArray(myShaders.positionAttribute);
  if(myShaders.colorAttribute != -1)
  glDisableVertexAttribArray(myShaders.colorAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  world.identity();
  //world.scale(scl,scl,scl);
  
  world.rotateXc(rotateX);
  world.rotateYc(rotateY);
  world.rotateZc(rotateZ);
  //world.scale(scaleX*scl,scaleY*scl,scaleZ*scl);
  world.scale(scaleX,scaleY,scaleZ);
  world.translate(translateX,translateY,translateZ);
  world=world*projection;

  // Вывод треугольника
  {
  glBindBuffer(GL_ARRAY_BUFFER, vboId);
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
  if(myShaders.worldUniform != -1)
	glUniformMatrix4fv(myShaders.worldUniform, 1, GL_FALSE, (GLfloat*)&world);
	glDrawArrays(GL_TRIANGLES, 0, 3);
  if(myShaders.positionAttribute != -1)
  glDisableVertexAttribArray(myShaders.positionAttribute);
  if(myShaders.colorAttribute != -1)
  glDisableVertexAttribArray(myShaders.colorAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  world.rotateY(90);

  // Вывод треугольника 2
  {
  glBindBuffer(GL_ARRAY_BUFFER, vboId);
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
  if(myShaders.worldUniform != -1)
	glUniformMatrix4fv(myShaders.worldUniform, 1, GL_FALSE, (GLfloat*)&world);
	glDrawArrays(GL_TRIANGLES, 0, 3);
  if(myShaders.positionAttribute != -1)
  glDisableVertexAttribArray(myShaders.positionAttribute);
  if(myShaders.colorAttribute != -1)
  glDisableVertexAttribArray(myShaders.colorAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  world.identity();

	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

void Update(ESContext *esContext, float deltaTime)
{
 // world=world*projection;
}

void Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
  if (bIsPressed)
	{
    switch (key)
    {
      case VK_LEFT: translateX-=1; printstat(); break;
      case VK_RIGHT: translateX+=1; printstat(); break;
      case VK_UP: translateY+=1; printstat(); break;
      case VK_DOWN: translateY-=1; printstat(); break;
      case VK_NEXT: translateZ+=1; printstat(); break;
      case VK_PRIOR: translateZ-=1; printstat(); break;

      case 'W': rotateX+=1; printstat(); break;
      case 'S': rotateX-=1; printstat(); break;
      case 'A': rotateY+=1; printstat(); break;
      case 'D': rotateY-=1; printstat(); break;
      case 'Q': rotateZ+=1; printstat(); break;
      case 'E': rotateZ-=1; printstat(); break;

      case 'U': scaleX+=1; printstat(); break;
      case 'J': scaleX-=1; printstat(); break;
      case 'I': scaleY+=1; printstat(); break;
      case 'K': scaleY-=1; printstat(); break;
      case 'O': scaleZ+=1; printstat(); break;
      case 'L': scaleZ-=1; printstat(); break;

      case 'R': fovy+=1; printstat(); ResizeScreen(Globals::screenWidth, Globals::screenHeight); break;
      case 'F': fovy-=1; printstat(); ResizeScreen(Globals::screenWidth, Globals::screenHeight); break;
      case 'T': zNear+=1; printstat(); ResizeScreen(Globals::screenWidth, Globals::screenHeight); break;
      case 'G': zNear-=1; printstat(); ResizeScreen(Globals::screenWidth, Globals::screenHeight); break;
      case 'Y': zFar+=1; printstat(); ResizeScreen(Globals::screenWidth, Globals::screenHeight); break;
      case 'H': zFar-=1; printstat(); ResizeScreen(Globals::screenWidth, Globals::screenHeight); break;

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
  SetColor(White,Black);
  printstat();
  esInitContext(&esContext);
	esCreateWindow(&esContext, "Hello Triangle", Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB | ES_WINDOW_DEPTH);
	if (Init(&esContext) != 0) return 0;
	esRegisterDrawFunc(&esContext, Draw );
	esRegisterUpdateFunc(&esContext, Update);
	esRegisterKeyFunc(&esContext, Key);
	esMainLoop(&esContext);
	CleanUp();  

  //VMATH::Matrix4d md;
  //md.translateX(55);
  //printf("Orthogonal factor: %d \n",md.isOrtogonal()); 

	MemoryDump(); // identifying memory leaks
	printf("Press any key...\n");
	_getch();
	return 0;
}