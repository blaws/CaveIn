// CaveIn.c
// blaws, 7/25/13

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <GLUT/glut.h>
#include "font.h"
using namespace std;

int score=0;
char scoreStr[10];
int wsize=500;
int charW=16,charH=32,charFrames=4,stalacW=16,stalacH=32,stalacFrames=2;
int charCoords[2],keyStates[3];
int ground[1000][500];
int counter=0,framecounter=0;
float charFrameX=0,charFrameY=0;
float vel[2];
int groundW=512,groundH=128;
int numGround=1;
int spawnfreqV=1,spawnfreqH=0;
bool running=true,paused=false,jump=true;
vector<int> stalactites;  // contents: stalacX,stalacY,stalacFrame,stalacDir

GLubyte rectangleTexture[512][128][3],charTexture[16][128][3];
GLubyte stalacTexture[16][64][3];
GLuint rectangleObj,charObj,stalacObj;
//float groundWratio,groundHratio;
//float charWratio,charHratio;

void init(){
  int i,j;
  score=0;
  counter=0;
  framecounter=0;
  charW=16;
  charH=32;
  charFrameX=0;
  charFrameY=0;
  spawnfreqV=1;
  spawnfreqH=0;
  running=true;
  paused=false;
  jump=true;

  charCoords[0] = wsize;
  charCoords[1] = wsize/5+charH/2+1;
  vel[0] = vel[1] = 0;
  for(i=0;i<2*wsize;i++) for(j=0;j<wsize/5;j++) ground[i][j] = 1;
  stalactites.clear();
}

void createTextures(){
  char filler[54];
  int i,j,k;

  FILE* fp = fopen("charTexture.bmp","rb");
  if(!fp){
    printf("File error.\n");
    exit(1);
  }
  fread(filler,54,1,fp);  // ignore bmp header
  for(i=0;i<charW;i++){
    for(j=0;j<charH*charFrames;j++){
      for(k=2;k>=0;k--)	fread(&charTexture[i][j][k],1,1,fp);
    }
    while((j++)%4) fread(filler,1,1,fp);
  }
  fclose(fp);
  /*charWratio = charW / exp2(ceil(log2(charW)));  // ratio to next larger power of 2
    charHratio = charH / exp2(ceil(log2(charH)));*/
  //gluScaleImage(GL_RGB,10,20,GL_UNSIGNED_BYTE,tmpTexture,16,32,GL_UNSIGNED_BYTE,charTexture);

  fp = fopen("rectangleTexture.bmp","rb");
  if(!fp){
    printf("File error.\n");
    exit(1);
  }
  fread(filler,54,1,fp);  // ignore bmp header
  for(i=0;i<groundW;i++){
    for(j=0;j<groundH;j++){
      for(k=2;k>=0;k--)	fread(&rectangleTexture[i][j][k],1,1,fp);
    }
    while((j++)%4) fread(filler,1,1,fp);
  }
  fclose(fp);
  /*groundWratio = groundW / exp2(ceil(log2(groundW)));  // ratio to next larger power of 2
    groundHratio = groundH / exp2(ceil(log2(groundH)));*/

  fp = fopen("stalacTexture.bmp","rb");
  if(!fp){
    printf("File error.\n");
    exit(1);
  }
  fread(filler,54,1,fp);  // ignore bmp header
  for(i=0;i<stalacW;i++){
    for(j=0;j<stalacH*stalacFrames;j++){
      for(k=2;k>=0;k--){
	fread(&stalacTexture[i][j][k],1,1,fp);
	//stalacTexture[i][j][k] /= 255.0;
      }
      //if(stalacTexture[i][j][0]==0.0 && stalacTexture[i][j][1]==0.0 && stalacTexture[i][j][2]==0.0) stalacTexture[i][j][3]=0.0;
      //else stalacTexture[i][j][3]=1.0;
    }
    while((j++)%4) fread(filler,1,1,fp);
  }
  fclose(fp);


  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glGenTextures(1,&rectangleObj);
  glBindTexture(GL_TEXTURE_2D,rectangleObj);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,groundW,groundH,0,GL_RGB,GL_UNSIGNED_BYTE,rectangleTexture);

  glGenTextures(1,&charObj);
  glBindTexture(GL_TEXTURE_2D,charObj);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,charW,charH*charFrames,0,GL_RGB,GL_UNSIGNED_BYTE,charTexture);

  glGenTextures(1,&stalacObj);
  glBindTexture(GL_TEXTURE_2D,stalacObj);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,stalacW,stalacH*stalacFrames,0,GL_RGB,GL_UNSIGNED_BYTE,stalacTexture);
}

void display(){
  int i;
  glClear(GL_COLOR_BUFFER_BIT);

  // ground 
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
  glBindTexture(GL_TEXTURE_2D,rectangleObj);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0); glVertex2i(0,wsize/5);
  glTexCoord2f(0.0,1.0); glVertex2i(0,wsize/5-128);
  glTexCoord2f(2.0,1.0); glVertex2i(2*wsize,wsize/5-128);
  glTexCoord2f(2.0,0.0); glVertex2i(2*wsize,wsize/5);
  glEnd();

  // character
  glBindTexture(GL_TEXTURE_2D,charObj);
  glBegin(GL_QUADS);
  if(running){
    glTexCoord2f(charFrameX,trunc(charFrameY)/charFrames);
    glVertex2i(charCoords[0]-charW/2,charCoords[1]-charH/2);
    glTexCoord2f(charFrameX,(trunc(charFrameY)+1)/charFrames);
    glVertex2i(charCoords[0]-charW/2,charCoords[1]+charH/2);
    glTexCoord2f(!charFrameX,(trunc(charFrameY)+1)/charFrames);
    glVertex2i(charCoords[0]+charW/2,charCoords[1]+charH/2);
    glTexCoord2f(!charFrameX,trunc(charFrameY)/charFrames);
    glVertex2i(charCoords[0]+charW/2,charCoords[1]-charH/2);
  }
  else{
    glTexCoord2f(charFrameX,trunc(charFrameY)/charFrames);
    glVertex2i(charCoords[0]-charW/2,charCoords[1]+charH/2);
    glTexCoord2f(charFrameX,(trunc(charFrameY)+1)/charFrames);
    glVertex2i(charCoords[0]+charW/2,charCoords[1]+charH/2);
    glTexCoord2f(!charFrameX,(trunc(charFrameY)+1)/charFrames);
    glVertex2i(charCoords[0]+charW/2,charCoords[1]-charH/2);
    glTexCoord2f(!charFrameX,trunc(charFrameY)/charFrames);
    glVertex2i(charCoords[0]-charW/2,charCoords[1]-charH/2);
  }
  glEnd();

  // stalactites
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D,stalacObj);
  int v1[2],v2[2],v3[2],v4[2],v5[2],dir;
  for(i=0;i<stalactites.size();i+=4){
    if(stalactites[i+3]<0) dir=-1;
    else dir=1;
    // assign vertices according to direction
    v1[0] = stalactites[i]-(stalactites[i+3]?0:stalacW/2);
    v1[1] = stalactites[i+1]-(stalactites[i+3]?stalacW/2:0);
    v2[0] = stalactites[i]+(stalactites[i+3]?0:stalacW/2);
    v2[1] = stalactites[i+1]+(stalactites[i+3]?stalacW/2:0);
    v3[0] = stalactites[i]+(stalactites[i+3]?dir*stalacH:0);
    v3[1] = stalactites[i+1]-(stalactites[i+3]?0:stalacH);
    if(stalactites[i+2]){
      v4[0] = stalactites[i] + stalactites[i+3]?.625*dir*stalacH:-stalacW;
      v4[1] = stalactites[i+1] + stalactites[i+3]?-stalacW:-.625*stalacH;
      v5[0] = stalactites[i] + stalactites[i+3]?.625*dir*stalacH:stalacW;
      v5[1] = stalactites[i+1] + stalactites[i+3]?stalacW:-.625*stalacH;
    }
    else v4[0]=v4[1]=v5[0]=v5[1]=0;

    // draw
    glBegin(GL_POLYGON);
    glTexCoord2f(0.0,0.5+.5*stalactites[i+2]); glVertex2iv(v1);
    glTexCoord2f(1.0,0.5+.5*stalactites[i+2]); glVertex2iv(v2);
    /*if(stalactites[i+2]){
      glTexCoord2f(1.0,.8125); glVertex2iv(v5);
    }*/
    glTexCoord2f(0.5,0.0+.5*stalactites[i+2]); glVertex2iv(v3);
    /*if(stalactites[i+2]){
      glTexCoord2f(0.0,.8125); glVertex2iv(v4);
      }*/
    glEnd();
  }
  //glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);

  // score
  i=0;
  sprintf(scoreStr,"%d",score);
  while(score/(int)pow(10.0,i)) i++;  // find # of digits of score
  glRasterPos2i(2*wsize-10*i,wsize-20);
  printString(scoreStr);

  // difficulty
  i=0;
  while(spawnfreqV/(int)pow(10.0,i)) i++;  // find # of digits of score
  sprintf(scoreStr,"%d",spawnfreqV);
  glRasterPos2i(2*wsize-10*i,wsize-40);
  printString(scoreStr);
  i=0;
  while(spawnfreqH/(int)pow(10.0,i)) i++;  // find # of digits of score
  sprintf(scoreStr,"%d",spawnfreqH);
  glRasterPos2i(2*wsize-10*i,wsize-60);
  printString(scoreStr);

  // Flush
  glutSwapBuffers();
}

void reshape(int w,int h){
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,2*wsize,0,wsize);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void delay(int);  // referenced in keyboard()

void keyboard(unsigned char key,int x,int y){
  switch(key){
  case 'q':
  case 'Q':
  case 27:  // 'ESC' key
    exit(0);
    break;
  case 'r':
  case 'R':
    init();
    break;
  case 'p':
  case 'P':
    if(paused){
      glutTimerFunc(50,delay,0);  // reset timer
      paused=false;
    }
    else paused=true;
  default:
    break;
  }
}

void keyboardSpecials(int key,int x,int y){
  if(running){
    switch(key){
    case GLUT_KEY_UP:
      keyStates[0] = 1;
      break;
    case GLUT_KEY_LEFT:
      keyStates[1] = 1;
      break;
    case GLUT_KEY_RIGHT:
      keyStates[2] = 1;
      break;
    default:
      break;
    }
  }
}

void keyboardSpecialsUp(int key,int x,int y){
  if(running){
    switch(key){
    case GLUT_KEY_UP:
      keyStates[0] = 0;
      jump = 1;
      break;
    case GLUT_KEY_LEFT:
      keyStates[1] = 0;
      break;
    case GLUT_KEY_RIGHT:
      keyStates[2] = 0;
      break;
    default:
      break;
    }
  }
}

void movement(){
  // speed up / slow down
  if(ground[charCoords[0]][charCoords[1]-charH/2-1]){  // if on ground
    if(charFrameY>2) charFrameY=0;

    if(keyStates[0] && jump){
      vel[1] += 10.0;  // jump
      charFrameY = 3;
      jump = 0;
      counter = 0;
    }

    // speed up
    else if(keyStates[1] && !keyStates[2]){
      vel[0]-=1.0;
      if(vel[0]<-15.0) vel[0]=-15.0;
      charFrameX = 1;
      charFrameY += .5;
      if(charFrameY>=3) charFrameY=0;
    }
    else if(keyStates[2] && !keyStates[1]){
      vel[0]+=1.0;
      if(vel[0]>15.0) vel[0]=15.0;
      charFrameX = 0;
      charFrameY += .5;
      if(charFrameY>=3) charFrameY=0;
    }
    // slow down
    else if(vel[0]>0.0) vel[0]-=.5;
    else if(vel[0]<0.0) vel[0]+=.5;
  }
  else{  // in air
    if(keyStates[0] && counter<10) counter++;  // allows higher jump when key is held
    else vel[1]-=1.0;
  }

  // movement
  charCoords[0] += vel[0];
  charCoords[1] += vel[1];

  // bounds checking
  if(charCoords[0]<charW/2){
    charCoords[0] = charW/2;
    vel[0] = 0;
  }
  else if(charCoords[0]>2*wsize-charW/2){
    charCoords[0] = 2*wsize-charW/2;
    vel[0] = 0;
  }
  if(ground[charCoords[0]][charCoords[1]-charH/2]){
    int i;
    while(ground[charCoords[0]][charCoords[1]-charH/2+i]) i++;  // make sure char is not in ground
    charCoords[1] += i;
    vel[1] = 0;
  }
}

void enemies(){
  int i,j;
  // generate
  if(rand()%10 < spawnfreqV){  // random downward
    stalactites.push_back(rand()%(2*wsize));  // X
    stalactites.push_back(wsize+stalacH);     // Y
    stalactites.push_back(0);                 // frame (not broken / broken)
    stalactites.push_back(0);                 // X direction
  }
  if(running && rand()%20<spawnfreqV-5){  // aimed downward
    stalactites.push_back(charCoords[0]-100+rand()%200);
    stalactites.push_back(wsize+stalacH);
    stalactites.push_back(0);
    stalactites.push_back(0);
  }
  if(rand()%30 < spawnfreqH-1){  // random rightward
    stalactites.push_back(-stalacH);
    stalactites.push_back(rand()%(4*wsize/5)+wsize/5+stalacW/2);
    stalactites.push_back(0);
    stalactites.push_back(10);
  }
  if(rand()%30 < spawnfreqH-1){  // random leftward
    stalactites.push_back(2*wsize+stalacH);
    stalactites.push_back(rand()%(4*wsize/5)+wsize/5+stalacW/2);
    stalactites.push_back(0);
    stalactites.push_back(-10);
  }

  // move
  for(i=0;i<stalactites.size();i+=4){
    if(!stalactites[i+2]){
      if(stalactites[i+3]) stalactites[i]+=stalactites[i+3];
      else stalactites[i+1]-=10;
    }
  }

  // destroy
  for(i=0;i<stalactites.size();i+=4){
    if(stalactites[i+2] || stalactites[i]<(-stalacH) || stalactites[i]>2*wsize+stalacH){
      for(j=0;j<4;j++) stalactites.erase(stalactites.begin()+i);
      i-=4;
    }
  }

  // increase difficulty
  if(running && spawnfreqV<35 && rand()%100==0){
    spawnfreqV++;
    if(spawnfreqH) spawnfreqH++;
  }
  if(running && spawnfreqV>10 && !spawnfreqH){
    spawnfreqV = 0;
    spawnfreqH = 1;
  }
}

void hitDetection(){
  int i,j;
  for(i=0;i<stalactites.size();i+=4){
    // with character
    if((!stalactites[i+3] &&
	(abs(charCoords[0]-stalactites[i]) < (charW+stalacW)/2) &&
	(abs(charCoords[1]-stalactites[i+1]) < (charH+stalacH)/2)) ||
       (stalactites[i+3] &&
	(abs(charCoords[0]-stalactites[i]) < (charH+stalacH)/2) &&
	(abs(charCoords[1]-stalactites[i+1]) < (charW+stalacW)/2))){
      if(running){
	int tmp = charW;
	charW = charH;
	charH = tmp;
	running = false;
      }
      stalactites[i+2] = 1;
      keyStates[0] = keyStates[1] = keyStates[2] = 0;
      break;
    }

    // with other stalactites
    for(j=0;j<stalactites.size();j+=4){
      if(j==i) continue;
      if((!stalactites[j+3] && !stalactites[i+3] && abs(stalactites[i]-stalactites[j])<stalacW && abs(stalactites[i+1]-stalactites[j+1])<stalacH) ||
	 (stalactites[j+3] && stalactites[i+3] && abs(stalactites[i]-stalactites[j])<2*stalacH && abs(stalactites[i+1]-stalactites[j+1])<stalacW) ||
	 (stalactites[j+3] && !stalactites[i+3] && abs(stalactites[i]-stalactites[j])<(stalacW+stalacH)/2 && abs(stalactites[i+1]-stalactites[j+1])<(stalacH+stalacH)/2) ||
	 (!stalactites[j+3] && stalactites[i+3] && abs(stalactites[i]-stalactites[j])<(stalacW+stalacH)/2 && abs(stalactites[i+1]-stalactites[j+1])<(stalacH+stalacH)/2) ){
	stalactites[i+2] = stalactites[j+2] = 1;
      }
    }

    // with ground
    if(stalactites[i]>=0 && stalactites[i]<=2*wsize){  // prevent seg fault
      if(ground[stalactites[i]][stalactites[i+1]-stalacH] && !stalactites[i+3] || stalactites[i+2]) stalactites[i+2]=1;
    }
  }
}

void delay(int t){
  movement();
  enemies();
  hitDetection();

  glutPostRedisplay();
  if(running) score++;
  if(!paused) glutTimerFunc(50,delay,0);  // reset timer
}

int main(int argc,char* argv[]){
  int i,j;
  srand(time(NULL));

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(2*wsize,wsize);
  glutInitWindowPosition(1280/2-wsize,(800-wsize)/2);
  glutCreateWindow("Runner");

  makeRasterFont();
  createTextures();

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(keyboardSpecials);
  glutSpecialUpFunc(keyboardSpecialsUp);
  glutReshapeFunc(reshape);

  init();

  glutTimerFunc(50,delay,0);  // start framerate timer (50 ms delay)
  glutMainLoop();

  return 0;
}
