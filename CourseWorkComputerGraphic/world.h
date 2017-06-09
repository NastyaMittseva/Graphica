#include <glew.h>
#include <glut.h>
#include <glu.h>
#include <glext.h>
#include <glaux.h>
#include <windows.h>
#include "mmsystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "BmpLoad.h"

class vertex;
class interior;

extern GLuint texture[100];
extern GLenum GLTexture[GL_MAX_TEXTURE_UNITS_ARB];
extern float start, end;
extern bool fog;

class vertex 
{
	GLfloat x, y, z; //3d
	GLfloat u, v;    //texture
	vertex* normal;
	GLfloat spacelight[3];
public:
  vertex(const vertex& vert);
	vertex(float c_x = 0, float c_y = 0, float c_z = 0,
				 float c_u = 0, float c_v = 0);
	~vertex () {};
	void glCoords(int tex = 0);
	vertex& operator=(const vertex vert);
	friend float distance(vertex v1, vertex v2);
	vertex operator-(const vertex v1) const; // точка = радиус-вектор
	vertex operator+(const vertex v1) const;
	vertex operator*(const vertex v1); //векторное произведение
	GLfloat operator/(const vertex v1); //скалярноe произведение
	vertex operator/(float lambda);
	friend class interior;
	void Normal();
};

class triangle;

class polygon 
{
	int vert_count;
	vertex* vertices;
	vertex normal;
	int D;
	bool flag;
	int* neighbours;
public:
	polygon (int count = 3);
	virtual ~polygon ();
	void addVertex(int index, const vertex add_v);
	void glVertices(int tex = 0);
	virtual void glPolygon(int tex = 0);
	void Normal();
	friend class bpatch;
	triangle** divide(int& vert);
	float operator-(vertex A);
	friend class interior;
	bool visible;
};

class triangle : public polygon 
{
	int vert_count;
	vertex *vertices;
	vertex normal;
	int D;
	bool flag;
	int* neighbours;
public:
	triangle () : polygon(3), vert_count(3){};
	~triangle (){};
	virtual void glPolygon(int tex = 0);
	friend class interior;
	bool visible;
};

class quad : public polygon
{
	int vert_count;
	vertex *vertices;
	vertex normal;
	int D;
	bool flag;
	int* neighbours;
public:
	quad () : polygon(4), vert_count(4) {};
	~quad () {};
	virtual void glPolygon(int tex = 0);
	friend class interior;
	bool visible;
};

float distance(vertex v1, vertex v2);

class interior 
{
	polygon** frames;
	int total;
	struct data
	{
		float key;
		int index;
	};
	int save_total;
	data* base;
public:
	interior(){};
	interior (const char* path);
	virtual ~interior ();
	virtual void display(vertex* viewer = NULL);
	void divide(int n = 1);
	void organize(vertex viewer);
	virtual void TexturesOn();
	virtual void TexturesOff();
	virtual GLuint gettex(int n);
public:
	int* textures;
	int tex;
};						 




