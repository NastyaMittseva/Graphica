#include "world.h"
#include "BmpLoad.h"

bool count_flag = false;

vertex::vertex(const vertex& vert){
//инициализация вершины 
	x = vert.x;
	y = vert.y;
	z = vert.z;
	u = vert.u;
	v = vert.v;
	normal = NULL;
}

vertex::vertex(float c_x, float c_y, float c_z, float c_u, float c_v) {
//инициализация вершины с явными параметрами
	x = c_x;
	y = c_y;
	z = c_z;
	u = c_u;
	v = c_v;
	normal = NULL;
}

void vertex::glCoords(int tex) {
	if (tex >= 1)
	{
		for (int i = 0; i < tex; i++)
			glMultiTexCoord2fARB(GLTexture[i], u, v); //при наложение нескольких тестур на полигон
	}
	if (normal != NULL)
		glNormal3f(normal->x, normal->y, normal->z); //установление нормали
	if (fog)
	{
		if (y < start && y > end && x < 3.0 && x > -3.0 && z < 3.0 && z > -3.0)
			glFogCoordf((start - y) / (start - end)); //задание затуманивания в вершине
		else
			glFogCoordf(0.0);
	}
	glVertex3f(x, y, z);


}

vertex& vertex::operator=(const vertex vert) {
//перегрузка оператора присваивания
	x = vert.x;
	y = vert.y;
	z = vert.z;
	u = vert.u;
	v = vert.v;
	return *this;
}
	
vertex vertex::operator-(const vertex v1) const {
//перегрузка оператора вычитания
	vertex aux;
	aux = vertex(x - v1.x, y - v1.y, z - v1.z, u - v1.u, v - v1.v);
	return aux;
}

vertex vertex::operator+(const vertex v1) const{
//перегрузка оператора сложения
	vertex aux;
	aux = vertex(x + v1.x, y + v1.y, z + v1.z, u + v1.u, v + v1.v);
	return aux;
}

vertex vertex::operator*(const vertex v1) {
//перегрузка оператора умножения
	vertex aux;
	aux = vertex(y*v1.z - z*v1.y, z*v1.x - x*v1.z, x*v1.y - y*v1.x, 0, 0);
	return aux;
}

GLfloat vertex::operator/(const vertex v1) {
//перегрузка оператора деления
	return x*v1.x+y*v1.y+z*v1.z;
}

vertex vertex::operator/(float lambda) {
//перегрузка оператора деления
	vertex aux;
	aux = vertex(x/lambda, y/lambda, z/lambda,  u/lambda, v/lambda);
	return aux;
}
	

void vertex::Normal() {
//установление нормали
	glNormal3f(x, y, z);
}
	
polygon::polygon(int count) {
// инициализация многоугольника
	vert_count = count;
	vertices = new vertex[count];
	flag = false;
	neighbours = new int[vert_count];
	for (int i = 0; i < vert_count; i++)
	{
	  neighbours[i] = -1;
	}
}
	
polygon::~polygon() {
// деструктор многоугольника
	vert_count = 0;
	delete [] vertices;
	delete [] neighbours;
}
	
void polygon::Normal(){
//вычисление нормали многоугольника
	if (!flag) {
		vertex vector1, vector2, inception;
		float d;
		vector1 = vertices[1] - vertices[0];
		vector2 = vertices[2] - vertices[1];
		normal = vector1*vector2;
		d = distance(normal, inception);
		normal = normal/d;
		if (!count_flag) // если для точки не посчитана нормаль
			normal.Normal();
		flag = !flag;
	}
	else if (!count_flag) 
		normal.Normal();
	D = - (vertices[0] / normal);
}

void polygon::addVertex(int index, const vertex add_v) {
//добавление точки
	vertices[index] = add_v;
}

void polygon::glVertices(int tex) {
//установление нормали полигона, затуманивания
	Normal();
	for (int i = 0; i < vert_count; i++)
	{
		vertices[i].glCoords(tex);
	}
}


void polygon::glPolygon(int tex) {
//инициализация полигона с текстурой
	glBegin(GL_POLYGON);
			glVertices(tex);
	glEnd();
}

triangle** polygon::divide(int& vert) {
// деление на треугольники для тумана
	vertex new_vertex;
	vertex summ;
	for (int i = 0; i < vert_count; i++) {
		summ = summ + vertices[i];	
	}
	new_vertex = summ / vert_count;
	triangle** result = new triangle* [vert_count];
	for (int j = 0; j < vert_count; j++)
	{
		result[j] = new triangle;
		result[j]->addVertex(0, vertices[j]);
		result[j]->addVertex(1, vertices[(j+1)%vert_count]);
		result[j]->addVertex(2, new_vertex); 
	}	
	vert = vert_count;
	return result;
}

float polygon::operator-(vertex A) {
//перегрузка оператора вычитания для упорядоченного вывода граней
	float d;
	vertex inception;
	if ((d = distance(normal, inception)) > 0.00001)
		return (A/normal + D)/d;
	else 
	  return 0;
}

void triangle::glPolygon(int tex){
//инициализация треугольника с текстурой
	glBegin(GL_TRIANGLES);
			glVertices(tex);
	glEnd();
}

void quad::glPolygon(int tex){
//инициализация квадрата с текстурой
	glBegin(GL_QUADS);
			glVertices(tex);
	glEnd();
}

float distance(vertex v1, vertex v2) {
//считает расстояние
	return sqrt((v1.x - v2.x)*(v1.x - v2.x) 
				+ (v1.y - v2.y)*(v1.y - v2.y)
				+ (v1.z - v2.z)*(v1.z - v2.z));
}

interior::interior(const char* path) {
//инициализация объекта интерьера
		using namespace std;
		float x, y, z, u, v, r, g, b;
		ifstream file;      
		string oneline;
		int i = 0, j = 0, k = 0;
		int h1 = 0, l = 0; 
		int f = 0;
		char s[20];
		int num = 0;

		file.open(path);
		getline(file, oneline, '\n');
		sscanf(oneline.c_str(), "TOTAL %d\n", &total);

		save_total = total;
		base = new data[total];
		for (int w = 0; w < total; w++)
		{
			base[w].index = w;
		}

		frames = new polygon* [total]; 

		while (!file.eof())
		{
			getline(file, oneline, '\n');
			if (oneline.length() > 0)
			{
				if (sscanf(oneline.c_str(), "%s %d %d\n", s, &i, &j))
				{
					if (!strcmp("QUADS", s))
						j = 4;
					else if (!strcmp("TRIANGLES", s))
						j = 3;
					else if (!strcmp("TEXTURES", s))
					{
						//текстуры
						tex = i;
						if (tex > 0)
							textures = new int[tex];
							for (f = 0; f < tex; f++)
							{
								getline(file, oneline, '\n');
								if (oneline.length() > 0)
								{
									sscanf(oneline.c_str(), "%d\n", &num);
									textures[f] = num;
								}
							}
						i = 0;
					}
					//полигоны		
					for (h1 = 0; h1 < i; h1++)
					{
						switch (j)
						{
							case 3:
								frames[k] = new triangle;
							break;
							case 4:
								frames[k] = new quad;
							break;
						}
						for (l = 0; l < j; l++)
						{
							do
								getline(file, oneline, '\n');
							while (oneline[0] == '/');

							if (sscanf(oneline.c_str(),"%f  %f  %f  %f  %f", &x, &y, &z, &u, &v))
							{
								frames[k]->addVertex(l, vertex(x, y, z, u, v));
							}
						}
						k++; //новый полигон
					}
				}
			}
		}

		if (k < total) 
			exit(k*10 + 1);

    file.close();
}

interior::~interior() {
//деструктор интерьера
	if (total)
		delete[] frames;
	if (tex)
		delete[] textures;
}

void interior::display(vertex* viewer) {
// отображение объекта интерьера
	if (viewer != NULL)
		organize(*viewer);
	else if (save_total != total)
	{
		vertex inception;
		organize(inception);
	}

	for (int i = 0; i < total; i++)
	{
		TexturesOn();
		frames[base[i].index]->glPolygon(tex);
		TexturesOff();
	}
		
}
	
void interior::TexturesOn() {
// включение текстуры
	if (tex >= 1)
	{
		glActiveTexture(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, gettex(0)); 
		glEnable(GL_TEXTURE_2D);
		if (tex > 1)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
		}

		for (int i = 1; i < tex; i++)
		{
			glActiveTexture(GLTexture[i]);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, gettex(i)); 
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
		}
	}
}

void interior::TexturesOff() {
//выключение текстурирования
	if (tex >= 1)
	{
		for (int i = 0; i < tex; i++)
		{
			glDisable(GL_TEXTURE_2D);
		}
	}
}

GLuint interior::gettex(int n) {
//получение номера текстуры
	int k = int(textures[n]);
	if (k > 0) {
		return texture[textures[n]];
	}
}

void interior::divide(int n) {
//деление на полигоны
	using namespace std;
	triangle** aux;
	polygon** frames_new;
	int count;
	for (int i = 0; i < n; i++)
	{
		aux = frames[0]->divide(count);
		frames_new = new polygon* [total + count - 1];
		for (int j = 1; j < total; j++)
		{
			frames_new[j-1] = frames[j];
		}
		for (int k = 0; k < count; k++)
		{
			frames_new[total - 1 + k] = aux[k];
		}
		delete[] frames;
		total += (count-1);
		frames = new polygon* [total + count - 1];
		frames = frames_new;
	}
}

void interior::organize(vertex viewer){
	if (save_total != total){
		if (base != NULL)
			delete[] base;
		base = new data[total];
		save_total = total;
	}
	//считаем расстояния
	for (int h = 0; h < total; h++) {
		base[h].index = h;
		base[h].key = *frames[h] - viewer;
	}
	//упорядочивание граней
	data aux;
	bool ordnung = true;
	for (int i = 0; i < total; i++) {
		for (int j = 0; j < total - 1; j++) {
			if (base[j].key < base[j+1].key) {
				aux = base[j];
				base[j] = base[j+1];
				base[j+1] = aux;	
				ordnung = false;
			}				
		}
		if (ordnung)
			break;
	}
}
