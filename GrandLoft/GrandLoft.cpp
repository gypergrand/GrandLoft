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

GLuint vboId, ax_vbo, textureId1, mvert1, mind1, mtex1;
Shaders myShaders;
static ESContext esContext;
static VMATH::Vector3<GLfloat> *v = 0;
static VMATH::Matrix4<GLfloat> projection, model, view, world;
static GLfloat translateX = 0, translateY = 0, translateZ = 0, 
         rotateX = 0, rotateY = 0, rotateZ = 0, 
         scaleX = 1, scaleY = 1, scaleZ = 1,
         fovy = 30.0f, zNear = 1.0f, zFar = 100.0f, scl=0.1f;

#pragma region funcs

enum ConsoleColor
{
  Black     = 0,
  Blue      = 1,
  Green     = 2,
  Cyan      = 3,
  Red       = 4,
  Magenta     = 5,
  Brown     = 6,
  LightGray   = 7,
  DarkGray    = 8,
  LightBlue   = 9,
  LightGreen  = 10,
  LightCyan   = 11,
  LightRed    = 12,
  LightMagenta  = 13,
  Yellow    = 14,
  White     = 15
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
  GLfloat aspect = screenWidth > screenHeight ? (GLfloat)screenWidth/(GLfloat)screenHeight : (GLfloat)screenHeight/(GLfloat)screenWidth;
  projection.identity();
  projection.perspective(fovy, aspect, zNear, zFar);
}

Model* LoadNFG(const char* file)
{
  FILE* f=fopen(file,"rt");
  if (f==NULL) return NULL;
  int cnt=0, cni=0;
  cni = fscanf_s(f, "NrVertices: %d",&cnt);
  if (cni<1) 
  {
    fclose(f);
    return NULL;
  }
  Model* m = new Model;
  m->ind = NULL;
  m->vert = new Vertex[cnt];
  cni=0;
  for (int i=0;i<cnt;i++)
  cni+=fscanf_s(f, " %*d. pos:[%f, %f, %f]; norm:[%f, %f, %f]; binorm:[%f, %f, %f]; tgt:[%f, %f, %f]; uv:[%f, %f];",
  &m->vert[i].pos.x, &m->vert[i].pos.y, &m->vert[i].pos.z, 
  &m->vert[i].normal.x, &m->vert[i].normal.y, &m->vert[i].normal.z,
  &m->vert[i].binormal.x, &m->vert[i].binormal.y, &m->vert[i].binormal.z,
  &m->vert[i].tgt.x, &m->vert[i].tgt.y, &m->vert[i].tgt.z,
  &m->vert[i].u, &m->vert[i].v);
  if (cni<1) goto err;
  cni = fscanf_s(f, "\nNrIndices: %d",&cnt);
  if (cni<1) goto err;
  m->ind = new Index[cnt+1];
  cni=0;
  for(int i=0;i<cnt;i+=3)
  cni+=fscanf_s(f, " %*d.    %d, %d, %d", &m->ind[i], &m->ind[i+1], &m->ind[i+2]); 
  if (cni<1)
  {
  err:
    delete [] m->vert;
    if (m->ind!=NULL)
    delete [] m->ind;
    delete m;
    m = NULL;
  }
  fclose(f);
  return m;
}

GLuint LoadTexture2DFromTGA(const char* TGA, GLint fmin, GLint fmax, GLint wrap_s, GLint wrap_t)
{
  GLuint tex;
  glGenTextures(1, &tex);  
  glBindTexture(GL_TEXTURE_2D, tex);  
  int width, height, bpp; 
  char * bufferTGA = LoadTGA(TGA, &width, &height, &bpp);
  if (bufferTGA == NULL) return 0;  
  if (bpp == 24)  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bufferTGA); else 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferTGA); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fmin);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fmax); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
  glBindTexture(GL_TEXTURE_2D, 0);  
  delete [] bufferTGA;
  return tex;
}
                                                                
int Init(ESContext *esContext)
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClearDepthf(1.0f);
  glDisable(GL_DITHER);

  ResizeScreen(Globals::screenWidth, Globals::screenHeight);

  Vertex verticesData[3], axisData[6];
  verticesData[0].pos.x =  0.0f;  verticesData[0].pos.y =  0.5f;  verticesData[0].pos.z = 0.0f;
  verticesData[1].pos.x = -0.5f;  verticesData[1].pos.y = -0.5f;  verticesData[1].pos.z = 0.0f;
  verticesData[2].pos.x =  0.5f;  verticesData[2].pos.y = -0.5f;  verticesData[2].pos.z = 0.0f;
  verticesData[0].color.r = 1.0f;  verticesData[0].color.g = 0.0f;  verticesData[0].color.b = 0.0f;
  verticesData[1].color.r = 0.0f;  verticesData[1].color.g = 1.0f;  verticesData[1].color.b = 0.0f;
  verticesData[2].color.r = 0.0f;  verticesData[2].color.g = 0.0f;  verticesData[2].color.b = 1.0f;
  verticesData[0].u = 0.5; verticesData[0].v = 1.0;
  verticesData[1].u = 0.0; verticesData[1].v = 0.0;
  verticesData[2].u = 1.0; verticesData[2].v = 0.0;

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

  Model* bus = LoadNFG("../Resources/Models/bus.nfg");

  glGenBuffers(1, &vboId); 
  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verticesData), verticesData, GL_STATIC_DRAW); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &ax_vbo); 
  glBindBuffer(GL_ARRAY_BUFFER, ax_vbo); 
  glBufferData(GL_ARRAY_BUFFER, sizeof(axisData), axisData, GL_STATIC_DRAW); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &mvert1); 
  glBindBuffer(GL_ARRAY_BUFFER, mvert1); 
  glBufferData(GL_ARRAY_BUFFER, 683*sizeof(Vertex), bus->vert, GL_STATIC_DRAW); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &mind1);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mind1);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1902*sizeof(Index), bus->ind, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  delete [] bus->vert;
  delete [] bus->ind;
  delete bus;

  textureId1 = LoadTexture2DFromTGA("../Resources/Textures/Rock.tga",GL_NEAREST,GL_NEAREST,GL_REPEAT,GL_REPEAT);
  if (textureId1==0) return -1;
  mtex1 = LoadTexture2DFromTGA("../Resources/Textures/Bus.tga",GL_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE);
  if (mtex1==0) return -1;
  return myShaders.Init("../Resources/Shaders/TriangleShaderVS.vs", "../Resources/Shaders/TriangleShaderFS.fs");
}

void DrawArrays(GLuint vbuffer, GLuint ibuffer, GLenum mode, GLint first, GLsizei count, GLuint texture)
{
  int tex_on = 0;
  if (texture != 0) 
    tex_on = 1;
  if (vbuffer == 0) return;
  glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
  if (ibuffer != 0)
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
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
  if(myShaders.textureAttribute != -1)
  {
    glEnableVertexAttribArray(myShaders.textureAttribute);
    glVertexAttribPointer(myShaders.textureAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), v+5);
  }
  if(myShaders.worldUniform != -1)
    glUniformMatrix4fv(myShaders.worldUniform, 1, GL_FALSE, (GLfloat*)&world);
  if(myShaders.texturedUniform != -1)
    glUniform1i(myShaders.texturedUniform, tex_on);
  if (tex_on==1)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(myShaders.samplerUniform, 0);
  }
  if (ibuffer != 0)
  glDrawElements(mode, count, GL_UNSIGNED_SHORT, v); else
  glDrawArrays(mode, first, count);
  if(myShaders.positionAttribute != -1)
    glDisableVertexAttribArray(myShaders.positionAttribute);
  if(myShaders.colorAttribute != -1)
    glDisableVertexAttribArray(myShaders.colorAttribute);
  if(myShaders.textureAttribute != -1)
    glDisableVertexAttribArray(myShaders.textureAttribute);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  if (ibuffer != 0)
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  if (tex_on==1)
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Draw(ESContext *esContext)
{
  glViewport(0, 0, Globals::screenWidth, Globals::screenHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(myShaders.program);
  
  model.identity();
 // model.scale(5);
  world=projection*view*model;

 // DrawArrays(ax_vbo, 0, GL_LINES, 0, 6, 0);

  model.identity();
 // model.translateX(1);
  world=projection*view*model;

 // DrawArrays(vboId, 0, GL_TRIANGLES, 0, 3, 0);

 // model.rotateY(90);
  world=projection*view*model;

 // DrawArrays(vboId, 0, GL_TRIANGLES, 0, 3, textureId1);

  model.translateY(-5);
  model.scale(0.1,0.1,0.1);
  world=projection*view*model;

  DrawArrays(mvert1, mind1, GL_TRIANGLES, 0, 1902, mtex1);

  eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

void Update(ESContext *esContext, float deltaTime)
{
  view.identity();
  VMATH::Vector3f eye(0,0,10), center(0,0,9), up(0,1,0);
  view.lookAt(eye, center, up);
  view.rotateXc(rotateX);
  view.rotateYc(rotateY);
  view.rotateZc(rotateZ);
  view.scale(scaleX,scaleY,scaleZ);  
  view.translate(translateX,translateY,translateZ-20);
  world=projection*view*model;
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
  else
  {
  //switch (key)
  //{
  //  default: break;
  //}
  }
}

void CleanUp()
{
  glDeleteBuffers(1, &vboId);
  glDeleteBuffers(1, &ax_vbo);
  glDeleteBuffers(1, &mvert1);
  glDeleteBuffers(1, &mind1);
  glDeleteTextures(1, &mtex1);
  glDeleteTextures(1, &textureId1);
}

int _tmain(int argc, _TCHAR* argv[])
{ 
  SetColor(White,Black);
  printstat();
  esInitContext(&esContext);
  esCreateWindow(&esContext, "Hello Triangle", Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB | ES_WINDOW_DEPTH);
  if (Init(&esContext) != 0) goto end_x;
  esRegisterDrawFunc(&esContext, Draw );
  esRegisterUpdateFunc(&esContext, Update);
  esRegisterKeyFunc(&esContext, Key);
 // esre
  esMainLoop(&esContext);
  goto dmp;
  //VMATH::Matrix4d md;
  //md.translateX(55);
  //printf("Orthogonal factor: %d \n",md.isOrtogonal()); 
  end_x:
  printf("Error initializing scene!\n");  
  esRelease(&esContext);
  dmp:
  CleanUp(); 
  MemoryDump(); // identifying memory leaks
  printf("Press any key...\n");
  _getch();
  return 0;
}