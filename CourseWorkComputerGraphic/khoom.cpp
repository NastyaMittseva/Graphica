#include "BmpLoad.h"
#include "world.h"

#define ESCAPE 27
#define PAGE_UP 73
#define PAGE_DOWN 81

GLfloat fogColor[] = { 0.5, 0.5, 0.8 };
bool fog = true;
float start, end;

GLenum GLTexture[GL_MAX_TEXTURE_UNITS_ARB];

float something = 0.5f;
int divs;
float t = 0;

bool blend = false;
bool light = false;   

GLfloat xrot;            // x rotation
GLfloat yrot;            // y rotation

GLfloat walkbias = 0;
GLfloat walkbiasangle = 0;

GLfloat lookupdown = 0.0;
const float piover180 = 0.0174532925f;

float heading, xpos, zpos;

GLfloat z=0.0f;                       // depth into the screen.
GLfloat LightPosition1[] = {0.0f, 0.85f, 0.0f, 1.0f};

interior world("world.txt");
interior Floor("Floor.txt");
interior Ship("ship.txt");
interior Fish("fish.txt");
interior Bird("birds.txt");
interior bg("bg.txt");
interior top("bg-top.txt");
interior bottom("bg-bottom.txt");
interior ob("object.txt");

const int GroundSize = 5;
const double shipSize = 1.0;
const int shipCount = 10;
const double fishSize = 1.0;
const int fishCount = 5;
const double birdSize = 1.0;
const int birdCount = 2;
double ships[shipCount];
double fishs[fishCount];
double birds[fishCount];

int bk_width;
int bk_width2;
int bk_height;
int bk_height2;
unsigned char *bk_bits;
unsigned char *bk_bits1;
unsigned char *bk_bits2;
GLuint texture[100];     // storage for 100 textures;

vertex GetPos(int x, int y) {
//позволяет получить координаты вершины объекта
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	//извлечение массивов значений
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	//копирует информацию о пикселях из буфера кадров в память
	glReadPixels(x, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
	//преобразование координат окна в координаты объекта
	gluUnProject(winX, winY, winZ, modelview, projection, viewport,
		&posX, &posY, &posZ);
	//возвращение вершины объекта
	return vertex(posX, posY, posZ, 0, 0);
}

void init(int Width, int Height)
{
	using namespace std;
	ifstream file;
	string oneline;
	int total = 0;
	int pos;

	file.open("textures.txt");

	getline(file, oneline, '\n');
	sscanf(oneline.c_str(), "TEXTURES %d\n", &total);

	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < total; i++)
	{
		getline(file, oneline, '\n');
		if (oneline.length() > 0)
		{
			bk_bits1 = LoadBMPFile(oneline.c_str(), &bk_width, &bk_height);
			pos = oneline.find(".");
			oneline.replace(pos, 20, "_alpha.bmp");

			unsigned char *bk_bits = new unsigned char[bk_width*bk_height * 4];
			bk_bits2 = LoadBMPFile(oneline.c_str(), &bk_width2, &bk_height2);
			for (int j = 0; j < bk_width*bk_height; j++)
			{
				bk_bits[4 * j] = bk_bits1[3 * j];
				bk_bits[4 * j + 1] = bk_bits1[3 * j + 1];
				bk_bits[4 * j + 2] = bk_bits1[3 * j + 2];
				if (bk_bits2 == NULL)
					bk_bits[4 * j + 3] = (unsigned char)255;
				else
					bk_bits[4 * j + 3] = (unsigned char)(((int)bk_bits2[3 * j] + (int)bk_bits2[3 * j + 1] + (int)bk_bits2[3 * j + 2]) / 3);
			}

			glGenTextures(1, &texture[i]);
			glBindTexture(GL_TEXTURE_2D, texture[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, bk_width, bk_height, GL_RGBA, GL_UNSIGNED_BYTE, bk_bits);
		}
	}
	glDisable(GL_TEXTURE_2D);  

	if (bk_bits != NULL)
		delete[] bk_bits;
	if (bk_bits1 != NULL)
		delete[] bk_bits1;
	if (bk_bits2 != NULL)
		delete[] bk_bits2;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();			

	gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);

	for (int i = 0; i < shipCount; i++)
		ships[i] = -GroundSize + shipSize + 2*rand()*(GroundSize - shipSize)/RAND_MAX;

	for (int i = 0; i < fishCount; i++)
		fishs[i] = -GroundSize + fishSize + 2 * rand()*(GroundSize - fishSize) / RAND_MAX;

	for (int i = 0; i < birdCount; i++)
		birds[i] = -GroundSize + birdSize + 2 * rand()*(GroundSize - birdSize) / RAND_MAX;
	
	world.divide(100); // для тумана
}

void display()
{
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  GLfloat xtrans, ztrans, ytrans;
  GLfloat sceneroty;

  xtrans = -xpos;
  ztrans = -zpos;
  ytrans = -walkbias-0.5f;
  sceneroty = 360.0f - yrot;
   	
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	
  double eqr[] = { 0.0f,-1.0f, 0.0f, 0.0f };
  glLoadIdentity();
  
  //кнопки вверх и вниз
  glRotatef(lookupdown, 1.0f, 0, 0);
  //влево и вправо
  glRotatef(sceneroty, 0, 1.0f, 0);
  //вперед и назад
  glTranslatef(xtrans, ytrans, ztrans);    
  
  glLightfv(GL_LIGHT1, GL_POSITION, LightPosition1);
	
  //рисуем фон
	glPushMatrix();
	glScalef(50, 50, 50);	
	bg.display();
	top.display();
	bottom.display();
	glScalef(0.02, 0.02, 0.02);	
	glPopMatrix();                       
	
	//комната
	//рисует отражение окна от пола
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glDisable(GL_DEPTH_TEST);
	Floor.display();

	//маскирует нижнюю комнату
	glEnable(GL_DEPTH_TEST);
	glColorMask(1, 1, 1, 1);
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, eqr);

	//рисует нижнюю комнату
	glPushMatrix();
	glScalef(1.0f, -1.0f, 1.0f);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition1);
	world.display();
	glPopMatrix();
	
	//создает зеркальный пол
	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_STENCIL_TEST);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition1);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Floor.display();
	if (light) glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	//рисует верхнюю комнату
	world.display();

	// слоистый туман
	if (fog) {
		start = 0.4;
		end = 0.0;
		glEnable(GL_FOG);
		//glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_START, start);
		glFogf(GL_FOG_END, end);
		glFogi(GL_FOG_HINT, GL_NICEST); // должен быть попиксельным
		glHint(GL_FOG_HINT, GL_NICEST); // должен быть попиксельным
		glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
	}

	//корабли из billboards
	glPushMatrix();	
	glTranslatef(0, -2.0, -3.0);
	glRotatef(0, 0.0, 1.0, 0.0);	
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05);
	
	for (int i = 0; i < shipCount; i++) 
	{
		int index = shipCount - 1 - i;
		double x = -GroundSize + shipSize + 2*(GroundSize - shipSize)*index/(shipCount - 1);
		double z = ships[index];

		glPushMatrix();
		glTranslated(0,0,-4*GroundSize);
		glRotated(xrot,1,0,0);
		glRotated(yrot,0,1,0);
		glTranslated(x,0,z-4);
		glRotated(-yrot,0,1,0);
		glRotated(-xrot,1,0,0);
		Ship.display();
		glPopMatrix();
	}
	glDisable(GL_ALPHA_TEST);
	glPopMatrix();

	//дельфины из billboards
	glPushMatrix();
	glTranslatef(-3.0, -1.0, -3.0);
	glRotatef(-180, 0.0, 1.0, 0.0);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05);

	for (int i = 0; i < fishCount; i++)
	{
		int index = fishCount - 1 - i;
		double x = -GroundSize + fishSize + 2 * (GroundSize - fishSize)*index / (fishCount - 1);
		double z = fishs[index];

		glPushMatrix();
		glTranslated(0, 0, -3 * GroundSize);
		glRotated(xrot, 1, 0, 0);
		glRotated(yrot, 0, 1, 0);
		glTranslated(x, 0, z+4);
		glRotated(-yrot, 0, 1, 0);
		glRotated(-xrot, 1, 0, 0);
		Fish.display();
		glPopMatrix();
	}
	glDisable(GL_ALPHA_TEST);
	glPopMatrix();

	//чайки из billboards
	glPushMatrix();
	glTranslatef(-2.0, 2.0, -3.0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05);

	for (int i = 0; i < birdCount; i++)
	{
		int index = birdCount - 1 - i;
		double x = -GroundSize + birdSize + 2 * (GroundSize - birdSize)*index / (birdCount - 1);
		double z = birds[index];

		glPushMatrix();
		glTranslated(0, 0, -3 * GroundSize);
		glRotated(xrot, 1, 0, 0);
		glRotated(yrot, 0, 1, 0);
		glTranslated(x, 0, z);
		glRotated(-yrot, 0, 1, 0);
		glRotated(-xrot, 1, 0, 0);
		Bird.display();
		glPopMatrix();
	}
	glDisable(GL_ALPHA_TEST);
	glPopMatrix();

	//чайки из billboards
	glPushMatrix();
	glTranslatef(-2.0, 2.0, -3.0);
	glRotatef(-90, 0.0, 1.0, 0.0);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05);

	for (int i = 0; i < birdCount; i++)
	{
		int index = birdCount - 1 - i;
		double x = -GroundSize + birdSize + 2 * (GroundSize - birdSize)*index / (birdCount - 1);
		double z = birds[index];

		glPushMatrix();
		glTranslated(0, 0, -3 * GroundSize);
		glRotated(xrot, 1, 0, 0);
		glRotated(yrot, 0, 1, 0);
		glTranslated(x, 0, z);
		glRotated(-yrot, 0, 1, 0);
		glRotated(-xrot, 1, 0, 0);
		Bird.display();
		glPopMatrix();
	}
	glDisable(GL_ALPHA_TEST);
	glPopMatrix();

	if (blend) 
	{
		//полупрозрачный объект с упорядоченным выводом граней
		glEnable(GL_BLEND);
		vertex* viewer = new vertex(GetPos(0, 0));
		ob.display(viewer);
		delete viewer;
		glDisable(GL_BLEND);
	}

	glFlush();
  glutSwapBuffers();
  
}

void reshape(int width, int height)
{
  if (height==0)				
    height=1;

  glViewport(0, 0, width, height);		

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);
}

void keyPressed(unsigned char key, int x, int y) 
{
	int i;
	switch (key) {    
   		case ESCAPE: 
			exit(1);                   	
			break; 
		case 'l': case 'L': {
			light = !light; 
			if (!light) 
	    	glDisable(GL_LIGHTING);
	 		else 
	   	 	glEnable(GL_LIGHTING);
		}
		break;
		case 'b': case 'B': {
			blend = !blend;
		}
		break;
	}
}

void specialKeyPressed(int key, int x, int y) 
{
    switch (key) 
		{    
    case GLUT_KEY_PAGE_UP: 
			z -= 0.2f;
			lookupdown -= 0.2f;
		break;
    
    case GLUT_KEY_PAGE_DOWN: 
			z += 0.2f;
			lookupdown += 1.0f;
		break;

    case GLUT_KEY_UP: 
			xpos -= (float)sin(yrot*piover180) * 0.05f;
			zpos -= (float)cos(yrot*piover180) * 0.05f;	
			if (walkbiasangle >= 359.0f)
	    		walkbiasangle = 0.0f;	
		else 
	  	  walkbiasangle+= 10;
			walkbias = (float)sin(walkbiasangle * piover180)/20.0f;
		break;

    case GLUT_KEY_DOWN: 
			xpos += (float)sin(yrot*piover180) * 0.05f;
			zpos += (float)cos(yrot*piover180) * 0.05f;	
			if (walkbiasangle <= 1.0f)
	    	walkbiasangle = 359.0f;	
		else 
	  	  walkbiasangle-= 10;
			walkbias = (float)sin(walkbiasangle * piover180)/20.0f;
		break;

    case GLUT_KEY_LEFT: 
			yrot += 1.5f;
		break;
    
    case GLUT_KEY_RIGHT: 
			yrot -= 1.5f;
		break;
    }	
}

void idle()
{
	something += 0.1;
	t+=0.1;
	divs = 7 + (int) (7.0*sin(t));
	display();
}

int main(int argc, char *argv[])
{
		using namespace std;
		GLTexture[0] = GL_TEXTURE0_ARB;

		for (int i = 1; i < 32; i++)
		{
			GLTexture[i] = GLTexture[i-1] + 1;
		}

		glutInit(&argc, argv);  
		glutInitWindowSize(640, 480);  
		glutInitWindowPosition(0, 0);  
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA);  
		glutCreateWindow("House by the sea");  
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			exit(42);
		}
		glutDisplayFunc(display);  
		glutFullScreen();
		glutIdleFunc(&idle); 
		glutReshapeFunc(&reshape);
		glutKeyboardFunc(&keyPressed);
		glutSpecialFunc(&specialKeyPressed);
		init(540, 380);
		PlaySound(TEXT("nature.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
		glutMainLoop();
    return 1;
}
