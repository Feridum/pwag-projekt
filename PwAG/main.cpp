#define _CRT_SECURE_NO_DEPRECATE
#include <cstdlib>
#include <iostream>
#include <fstream>
#define GLUT_DISABLE_ATEXIT_HACK
#include "GL/glut.h"
#include "GL/glext.h"
#include "GL/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <math.h>  
#include "Heightmap.hpp"
#include <time.h>     
#include <algorithm>
#include <random>
#include <vector>

#define PI 3.14159265
#define MAX_HOUSE_COUNTER		10
#define HOUSE_QUANTITY			3
#define HOUSE_COUNTER			min(HOUSE_QUANTITY,MAX_HOUSE_COUNTER)
#define MIN_X					-0.9
#define MAX_X					0.9
#define MIN_Y					-0.9
#define MAX_Y					0.9
#define PROPORTION				0.2
#define WALL_LENGTH				(abs(MIN_X) + abs(MAX_X))*PROPORTION

using namespace std;

void setShaders(const char* vertexShaderFile, const char* fragmentShaderFile, const char* geom, const char* tcs, const char* es);

//static int programHandle; // obiekt programu
int houseProgramHandle; // obiekt programu
int terrainProgramHandle; 

glm::vec3 posATab[HOUSE_COUNTER] = { glm::vec3(0.0, 0.0, 0.0) };
float rand1Tab[HOUSE_COUNTER] = { 0 };
float rand2Tab[HOUSE_COUNTER] = { 0 };

GLuint* terrainText = new GLuint[5];
GLuint* houseText = new GLuint[2];

unsigned int texture1;
unsigned int texture2;
float wallHeight = 0;
bool firstShader = true;

GLuint vbo_id[2];
GLfloat xyz[20 * 3][3] = { 0 };

static GLint viewMatrixTerrain;
static GLint viewMatrixHouse;
static GLint wallLLocation;
static GLint randLocation1;
static GLint randLocation2;
static GLint wallHLocation;
static GLint ZLocation;
static GLint posAXLocation;
static GLint posAYLocation;

//wskaźniki funkcji
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC	glUseProgram = NULL;
PFNGLCOMPILESHADERPROC	glCompileShader = NULL;
PFNGLPATCHPARAMETERIPROC	glPatchParameteri = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB = NULL;
PFNGLGETUNIFORMLOCATIONPROC  glGetUniformLocation = NULL;
PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLUNIFORM2FVPROC glUniform2fv = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = NULL;
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;
PFNGLMAPBUFFERPROC glMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;

// funkcja do odczytu kodu shaderow
char* readShader(const char* aShaderFile)
{
	FILE* filePointer = fopen(aShaderFile, "rb");
	char* content = NULL;
	long numVal = 0;

	fseek(filePointer, 0L, SEEK_END);
	numVal = ftell(filePointer);
	fseek(filePointer, 0L, SEEK_SET);
	content = (char*)malloc((numVal + 1) * sizeof(char));
	fread(content, 1, numVal, filePointer);
	content[numVal] = '\0';
	fclose(filePointer);
	return content;
}

//To jest do glLookAt
GLdouble eyex = 0;
GLdouble eyey = 0;
GLdouble eyez = 0;

GLdouble centerx = 6;
GLdouble centery = 77;
GLdouble centerz = 100;

//change also in terrain.es file
int numHills = 4;
float hillRadiusMin = 0.3;
float hillRadiusMax = 0.4;
float hillMinHeight = 0.4;
float hillMaxHeight = 0.5;

static GLint centers;
static GLint height; 
static GLint radius;


vector<float> hillsCenters(numHills*2);
float hillRadius;
float hillHeight;






glm::vec3 cameraPos = glm::vec3(eyex, eyey, eyez);
glm::vec3 cameraTarget = glm::vec3(centerx, centery, centerz);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


GLfloat Z = 0.0;

static GLfloat vhouse[HOUSE_COUNTER * 42][5] = { 0 };

GLfloat singleHouse[4][5] = { 
	{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},
	{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0} };

static GLfloat vdata[6][5] = {
	{-1.0, Z, -1.0, 1.0, 1.0}, {-1.0, Z, 1.0, 1.0, 0.0 }, {1.0, Z, 1.0, 0.0, 0.0},
	{1.0, Z, 1.0, 1.0, 1.0}, {1.0, Z, -1.0, 1.0, 0.0}, {-1.0, Z, -1.0, 0.0}
};

/*
	Funkcja, która przypisuje wartości punktu we wskazane miejsce w tablicy domków. Bez tego funkcja createHouses() byłaby BARDZO długa...
	Jako pola 4 i 5 ([][3], [][4]) wpisuje wartości współrzędnych tekstury (kombinacja 0 i 1)
*/
void copyPoint(int i, int j, int t, glm::vec3 Point)
{
	/* //PIERWOWZOR KROTSZY BOK
	vhouse[i * 42 + j][0] = Point.x;
	vhouse[i * 42 + j][1] = Point.y;
	vhouse[i * 42 + j][2] = Point.z;
	*/
	//DLUZSZY BOK
	vhouse[i * 42 + j][0] = Point.y;
	vhouse[i * 42 + j][1] = Point.x;
	vhouse[i * 42 + j][2] = Point.z;

	/* //OD GORY
	vhouse[i * 42 + j][0] = Point.x;
	vhouse[i * 42 + j][1] = Point.z;
	vhouse[i * 42 + j][2] = Point.y;
	*/

	if (t == 0)
	{
		vhouse[i * 42 + j][3] = 1.0;
		vhouse[i * 42 + j][4] = 1.0;
	}
	else if (t == 1)
	{
		vhouse[i * 42 + j][3] = 1.0;
		vhouse[i * 42 + j][4] = 0.0;
	}
	else if (t == 2)
	{
		vhouse[i * 42 + j][3] = 0.0;
		vhouse[i * 42 + j][4] = 0.0;
	}
	else
	{
		vhouse[i * 42 + j][3] = 0.0;
		vhouse[i * 42 + j][4] = 1.0;
	}
}

void renderHouse(int i)
{
	wallLLocation = glGetUniformLocation(houseProgramHandle, "wallLength");
	randLocation1 = glGetUniformLocation(houseProgramHandle, "randoms1");
	randLocation2 = glGetUniformLocation(houseProgramHandle, "randoms2");
	wallHLocation = glGetUniformLocation(houseProgramHandle, "wallH");
	ZLocation = glGetUniformLocation(houseProgramHandle, "Zvalue");
	posAXLocation = glGetUniformLocation(houseProgramHandle, "vertexAX");
	posAYLocation = glGetUniformLocation(houseProgramHandle, "vertexAY");

	srand(time(NULL));
	glUseProgram(houseProgramHandle);
	glUniform1f(wallHLocation, wallHeight);
	glUniform1f(ZLocation, Z);
	glUniform1f(wallLLocation, WALL_LENGTH);
	float rand1 = 0;
	float rand2 = 0;
	//rand1 = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 2.0));
	//rand2 = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 3.0));
	rand1 = rand1Tab[i];
	rand2 = rand2Tab[i];
	glUniform1f(randLocation1, rand1);
	glUniform1f(randLocation2, rand2);


	glm::vec3 posA = glm::vec3(0.0, 0.0, 0.0);
	posA.x = posATab[i].x;
	posA.y = posATab[i].y;
	//posA.z = posATab[i].z;
	glUniform1f(posAXLocation, posA.x);
	glUniform1f(posAYLocation, posA.y);

	//singleHouse: mamy 4 wierzchołki, pierwszy z nich jest oddalony od punktu A o -1, -1 (x-1, y-1)

	singleHouse[0][0] = posA.x - 1.0;
	singleHouse[0][1] = posA.y - 1.0;
	singleHouse[0][2] = posA.z;
	singleHouse[0][3] = 0.0;
	singleHouse[0][4] = 0.0;

	singleHouse[1][0] = posA.x - 1.0;
	singleHouse[1][1] = posA.y + 1.0;
	singleHouse[1][2] = posA.z;
	singleHouse[1][3] = 0.0;
	singleHouse[1][4] = 1.0;

	singleHouse[2][0] = posA.x + 1.0;
	singleHouse[2][1] = posA.y - 1.0;
	singleHouse[2][2] = posA.z;
	singleHouse[2][3] = 1.0;
	singleHouse[2][4] = 0.0;

	singleHouse[3][0] = posA.x + 1.0;
	singleHouse[3][1] = posA.y + 1.0;
	singleHouse[3][2] = posA.z;
	singleHouse[3][3] = 1.0;
	singleHouse[3][4] = 1.0;

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(singleHouse), singleHouse, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_PATCHES, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void makeHouse()
{
	srand(time(NULL));
	glm::vec3 posA;
	bool colide = false;
	int failCounter = 0;
	float rand1, rand2;

	for (int i = 0; i < HOUSE_COUNTER; i++)
	{
		posATab[i].x = 0;
		posATab[i].y = 0;
		posATab[i].z = 0;

		rand1 = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 0.5));
		rand1Tab[i] = rand1;
		rand2 = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 0.4));
		rand2Tab[i] = rand2;
	}

	wallHeight = max((float)(WALL_LENGTH / 3.0), (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (WALL_LENGTH / 1.0))));

	for (int i = 0; i < HOUSE_COUNTER; i++)
	{
		posA.x = MIN_X + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (abs(MAX_X) + abs(MIN_X))));
		posA.y = MIN_Y + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (abs(MAX_Y) + abs(MIN_Y))));
		posA.z = Z;

		//jeżeli mamy już jakieś punkty A, to musimy sprawdzić, czy nie leżą za blisko siebie - czy zbudowane na nich domki nie będą na siebie nachodzić
		if (posATab[0].x != 0 || posATab[0].y != 0 || posATab[0].z != 0)		//jeśli tak, to znaczy, że mamy już jakieś domki
		{
			colide = true;
			failCounter = 0;
			while (colide && failCounter < 50)
			{
				failCounter++;
				colide = false;
				for (int j = 0; j < HOUSE_COUNTER; j++)
				{
					if (posATab[j].x != 0 || posATab[j].y != 0 || posATab[j].z != 0)
					{
						if (pow(posATab[j].x - posA.x, 2.0) + pow(posATab[j].y - posA.y, 2.0) <= pow(WALL_LENGTH * 2, 2.0))
						{
							colide = true;
							break;
						}
					}
				}
				if (colide == true)
				{
					posA.x = MIN_X + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (abs(MAX_X) + abs(MIN_X))));
					posA.y = MIN_Y + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (abs(MAX_Y) + abs(MIN_Y))));
					posA.z = Z;
				}
			}
		}
		if (colide == true)
		{
			std::cout << "\nUnable to create new house. Please change house number to smaller value or make villige plain larger\n";
		}
		else
		{
			posATab[i].x = posA.x;
			posATab[i].y = posA.y;
			posATab[i].z = posA.z;
		}
	}
}

/*
	Aby wygenerować domki należy przesłać pewną ilość wierzchołków do potoku graficznego
	Domek posiada 10 wierzchołków, które budują jego szkielet oraz jedynasty - punkt środkowy, potrzebny do określenia położenia pozostałych wierzchołków
	Jednakże ściany będą zbudowane z trójkątów, a trójkąty będziemy wykorzystywać przy renderowaniu. Z tego też powodu wysyłamy:
	(3+3)*6 + 3 + 3 = 42 wierzchołki na domek.

	Potrzebne będzie wylosowanie 5 liczb (przy założeniu, że domki będą leżały na jednej wysokości): wartości x,y dla punktu centalnego - A, wartości x,y dla jednego wierzchołka podstawy - B oraz wartości wysokości ściany domku.
*/
void createHouses()
{
	//(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) -> liczba losowa typu float z zakresu 0-1
	srand(time(NULL));
	glm::vec3 posA, posB, posBp, posC, posCp, posD, posDp, posE, posEp, posF, posFp;
	float houseX, houseY;

	glm::vec3 posATab[HOUSE_COUNTER];
	bool colide = false;
	int failCounter = 0;

	for (int i = 0; i < HOUSE_COUNTER; i++)
	{
		posATab[i].x = 0;
		posATab[i].y = 0;
		posATab[i].z = 0;
	}

	wallHeight = max((float)(WALL_LENGTH / 2.0), (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (WALL_LENGTH / 1.0))));



	for (int i = 0; i < HOUSE_COUNTER; i++)
	{
		posA.x = MIN_X + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (abs(MAX_X) + abs(MIN_X))));
		posA.y = MIN_Y + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (abs(MAX_Y) + abs(MIN_Y))));
		posA.z = Z;

		//jeżeli mamy już jakieś punkty A, to musimy sprawdzić, czy nie leżą za blisko siebie - czy zbudowane na nich domki nie będą na siebie nachodzić
		if (posATab[0].x != 0 || posATab[0].y != 0 || posATab[0].z != 0)		//jeśli tak, to znaczy, że mamy już jakieś domki
		{
			colide = true;
			failCounter = 0;
			while (colide && failCounter < 30)
			{
				failCounter++;
				colide = false;
				for (int j = 0; j < HOUSE_COUNTER; j++)
				{
					if (posATab[j].x != 0 || posATab[j].y != 0 || posATab[j].z != 0)
					{
						if (pow(posATab[j].x - posA.x, 2.0) + pow(posATab[j].y - posA.y, 2.0) <= pow(WALL_LENGTH * 2, 2.0))
						{
							colide = true;
							break;
						}
					}
				}
				if (colide == true)
				{
					posA.x = MIN_X + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (abs(MAX_X) + abs(MIN_X))));
					posA.y = MIN_Y + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (abs(MAX_Y) + abs(MIN_Y))));
					posA.z = Z;
				}
			}
		}
		if (colide == true)
		{
			std::cout << "\nUnable to create new house. Please change house number to smaller value or make villige plain larger\n";
		}
		else
		{
			//teraz mamy już pewność, że zbudowane domki nie będą na siebie nachodzić
			posATab[i].x = posA.x;
			posATab[i].y = posA.y;
			posATab[i].z = posA.z;

			//houseX = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 2.0));
			houseX = (max((float)0.4, (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 1.5))))* (float)WALL_LENGTH;
			posB.x = posA.x + houseX;	//Punkt B w OSI X będzie oddalony od punktu A o wartość <0.2-0.5> bazowej długości ściany
			houseY = (max((float)0.2, (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 3.0))))* (float)WALL_LENGTH;
			posB.y = posA.y + houseY;	//Punkt B w OSI Y będzie oddalony od punktu A o wartość <0.1-0.25> bazowej długości ściany
			posB.z = posA.z;

			posBp.x = posA.x - houseX;
			posBp.y = posA.y - houseY;
			posBp.z = posA.z;

			posC.x = posA.x - houseX;
			posC.y = posB.y;
			posC.z = posA.z;

			posCp.x = posB.x;
			posCp.y = posA.y - houseY;
			posCp.z = posA.z;

			posD.x = posB.x;
			posD.y = posB.y;
			posD.z = posB.z + wallHeight;

			posDp.x = posBp.x;
			posDp.y = posBp.y;
			posDp.z = posBp.z + wallHeight;

			posE.x = posC.x;
			posE.y = posC.y;
			posE.z = posC.z + wallHeight;

			posEp.x = posCp.x;
			posEp.y = posCp.y;
			posEp.z = posCp.z + wallHeight;	//

			posF.x = posA.x;
			posF.y = posA.y + (0.8 * houseY);//posB.y;
			posF.z = posA.z + 1.5 * wallHeight;

			posFp.x = posA.x;
			posFp.y = posA.y - (0.8 * houseY);// posBp.y;
			posFp.z = posA.z + 1.5 * wallHeight;

			//teraz mamy już wszystkie wierzchołki domku. Pozostało połączyć je w trójkąty i odpowiednio wpakować do tabeli
			/*
				Trójkąty:

			#0  BCB`		132		->jeślibyśmy chcieli robić też podłogę
			#1  B`C`B		241		->jeślibyśmy chcieli robić też podłogę

			2  B`EC		273
			3  D`EB`	672
			4  CEB		371
			5  EDB		751
			6  B`D`C`	264
			7  C`D`E`	468
			8  E`DC`	854
			9  BC`D		145
			10 EFD		795
			11 B`F`E`	2(10)8
			12 D`F`E	6(10)8
			13 F`FE		(10)97
			14 FF`E`	9(10)8
			15 E`DF		859

			*/

			copyPoint(i, 0, 0, posBp);
			copyPoint(i, 1, 1, posE);
			copyPoint(i, 2, 2, posC);

			copyPoint(i, 3, 0, posDp);
			copyPoint(i, 4, 1, posE);
			copyPoint(i, 5, 2, posBp);

			copyPoint(i, 6, 0, posC);
			copyPoint(i, 7, 1, posE);
			copyPoint(i, 8, 2, posB);

			copyPoint(i, 9, 0, posE);
			copyPoint(i, 10, 1, posD);
			copyPoint(i, 11, 2, posB);

			copyPoint(i, 12, 0, posBp);
			copyPoint(i, 13, 1, posDp);
			copyPoint(i, 14, 2, posCp);

			copyPoint(i, 15, 0, posCp);
			copyPoint(i, 16, 1, posDp);
			copyPoint(i, 17, 2, posEp);

			copyPoint(i, 18, 0, posEp);
			copyPoint(i, 19, 1, posD);
			copyPoint(i, 20, 2, posCp);

			copyPoint(i, 21, 0, posB);
			copyPoint(i, 22, 1, posCp);
			copyPoint(i, 23, 2, posD);

			copyPoint(i, 24, 0, posE);
			copyPoint(i, 25, 1, posF);
			copyPoint(i, 26, 2, posD);

			copyPoint(i, 27, 0, posBp);
			copyPoint(i, 28, 1, posFp);
			copyPoint(i, 29, 2, posEp);

			copyPoint(i, 30, 0, posDp);
			copyPoint(i, 31, 1, posFp);
			copyPoint(i, 32, 2, posE);

			copyPoint(i, 33, 0, posFp);
			copyPoint(i, 34, 1, posF);
			copyPoint(i, 35, 2, posE);

			copyPoint(i, 36, 0, posF);
			copyPoint(i, 37, 1, posFp);
			copyPoint(i, 38, 2, posEp);

			copyPoint(i, 39, 0, posEp);
			copyPoint(i, 40, 1, posD);
			copyPoint(i, 41, 2, posF);
		}
	}
	//skończono generować domki
}

void loadTerrain(GLuint texture, const char* textureFile) {
	
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(textureFile, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}


void terrain() {
	glGenBuffers(2, vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vdata), vdata, GL_STATIC_DRAW);
	glGenTextures(3, terrainText);
	loadTerrain(terrainText[0], "soil.jpg");
	loadTerrain(terrainText[1], "grass.jpg");
	loadTerrain(terrainText[2], "snow.jpg");


	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(singleHouse), singleHouse, GL_STATIC_DRAW);
	glGenTextures(2, houseText);
	loadTerrain(houseText[0], "roof.jpg");
	loadTerrain(houseText[1], "wallW.jpg");
}


void drawTerrain() {

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(terrainProgramHandle, "soil"), 0);
	glBindTexture(GL_TEXTURE_2D, terrainText[0]);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(terrainProgramHandle, "grass"), 1);
	glBindTexture(GL_TEXTURE_2D, terrainText[1]);


	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(terrainProgramHandle, "snow"), 2);
	glBindTexture(GL_TEXTURE_2D, terrainText[2]);

	glDrawArrays(GL_PATCHES, 0, 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void drawHouses() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[1]);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(houseProgramHandle, "texture1"), 0);
	glBindTexture(GL_TEXTURE_2D, houseText[0]);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(houseProgramHandle, "texture2"), 1);
	glBindTexture(GL_TEXTURE_2D, houseText[1]);

	glCullFace(GL_FRONT_AND_BACK);

	for (int i = 0; i < HOUSE_COUNTER; i++)
	{
		renderHouse(i);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// incjalizacja shaderów
void setShaders(int program, const char* vertexShaderFile, const char* fragmentShaderFile, const char* tcs, const char* es)
{
	GLint status = 0;

	char* vertexShader = readShader(vertexShaderFile);
	char* fragmentShader = readShader(fragmentShaderFile);
	char* tcsShader = readShader(tcs);
	char* esShader = readShader(es);

	 // tworzenie obiektu programu
	static int vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER); // shader wierzchołków
	static int fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER); // shader fragmentów
	static int tcsShaderHandle = glCreateShader(GL_TESS_CONTROL_SHADER); // shader tcs
	static int esShaderHandle = glCreateShader(GL_TESS_EVALUATION_SHADER); // shader es

	glShaderSource(vertexShaderHandle, 1, (const char**)&vertexShader, NULL); // ustawianie źródła shadera wierzchołków
	glShaderSource(fragmentShaderHandle, 1, (const char**)&fragmentShader, NULL); // ustawianie źródła shadera fragmentów
	glShaderSource(tcsShaderHandle, 1, (const char**)&tcsShader, NULL); // ustawianie źródła shadera fragmentów
	glShaderSource(esShaderHandle, 1, (const char**)&esShader, NULL); // ustawianie źródła shadera fragmentów

	// kompilacja shaderów
	glCompileShader(vertexShaderHandle);
	glCompileShader(fragmentShaderHandle);
	glCompileShader(tcsShaderHandle);
	glCompileShader(esShaderHandle);


	/* error check */

	glGetObjectParameterivARB(vertexShaderHandle, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if (!status) {
		const int maxInfoLogSize = 2048;
		GLchar infoLog[maxInfoLogSize];
		glGetInfoLogARB(vertexShaderHandle, maxInfoLogSize, NULL, infoLog);
		std::cout << infoLog;
	}

	glGetObjectParameterivARB(fragmentShaderHandle, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if (!status) {
		const int maxInfoLogSize = 2048;
		GLchar infoLog[maxInfoLogSize];
		glGetInfoLogARB(fragmentShaderHandle, maxInfoLogSize, NULL, infoLog);
		std::cout << infoLog;
	}


	//dodanie shaderów do programu
	glAttachShader(program, vertexShaderHandle);
	glAttachShader(program, tcsShaderHandle);
	glAttachShader(program, esShaderHandle);
	glAttachShader(program, fragmentShaderHandle);

	/* link */
	//uruchomienie
	glLinkProgram(program);

	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &status);
	if (!status) {
		const int maxInfoLogSize = 2048;
		GLchar infoLog[maxInfoLogSize];
		glGetInfoLogARB(program, maxInfoLogSize, NULL, infoLog);
		std::cout << infoLog;
	}

	
}

void setUniformTerrain() {
	centers = glGetUniformLocation(terrainProgramHandle, "hill_centers");
	height = glGetUniformLocation(terrainProgramHandle, "hill_height");
	radius = glGetUniformLocation(terrainProgramHandle, "hill_radius");
	viewMatrixTerrain = glGetUniformLocation(terrainProgramHandle, "view_matrix");

	glUseProgram(terrainProgramHandle);
	glUniform1f(wallHLocation, wallHeight);
	glUniform2fv(centers, hillsCenters.size(), hillsCenters.data());
	glUniform1f(height, hillHeight);
	glUniform1f(radius, hillRadius);
}

void setUniformHouse() {
	viewMatrixHouse = glGetUniformLocation(houseProgramHandle, "view_matrix");
	GLint wallHLocation = glGetUniformLocation(houseProgramHandle, "wallH");
	GLint ZLocation = glGetUniformLocation(houseProgramHandle, "Zvalue");

	glUseProgram(houseProgramHandle);
	glUniform1f(wallHLocation, wallHeight);
	glUniform2fv(centers, hillsCenters.size(), hillsCenters.data());
}

// Drawing (display) routine.
void drawScene(void)
{
	// Clear screen to background color.
	glClear(GL_COLOR_BUFFER_BIT);

	// Set foreground (or drawing) color.
	glColor3f(0.0, 0.0, 0.0);
	//glFrontFace(GL_CW);
	// Set polygon mode.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	cameraPos = glm::vec3(eyex, eyey, eyez);
	cameraTarget = glm::vec3(centerx, centery, centerz);
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	


	glUseProgram(terrainProgramHandle);
	glUniformMatrix4fv(viewMatrixTerrain, 1, GL_FALSE, glm::value_ptr(view));
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	drawTerrain();
	
	glUseProgram(houseProgramHandle);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glUniformMatrix4fv(viewMatrixHouse, 1, GL_FALSE, glm::value_ptr(view));
	drawHouses();

	// Flush created objects to the screen, i.e., force rendering.
	glutSwapBuffers();
	glutPostRedisplay();


}

// Initialization routine.
void setup(void)
{
	// Set background (or clearing) color.
	glClearColor(1.0, 1.0, 1.0, 0.0);


}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	// Set viewport size to be entire OpenGL window.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	// Set matrix mode to projection.
	glMatrixMode(GL_PROJECTION);

	// Clear current projection matrix to identity.
	glLoadIdentity();

	// Specify the orthographic (or perpendicular) projection, 
	// i.e., define the viewing box.
	gluPerspective(-60, w / h, -100, 1000);
	//glFrustum(-10.0, 10.0, -10.0, 10.0, 0.0, 200.0);
	// Set matrix mode to modelview.
	glMatrixMode(GL_MODELVIEW);

	// Clear current modelview matrix to identity.
	glLoadIdentity();
	drawScene();
}

// sprawdzenie i przygotowanie obslugi wybranych rozszerzen
void extensionSetup()
{
	// pobranie numeru wersji biblioteki OpenGL
	const char* version = (char*)glGetString(GL_VERSION);
	printf("%s", version);
	if ((version[0] < '1') || ((version[0] == '1') && (version[2] < '5')) || (version[1] != '.')) {
		printf("Bledny format wersji OpenGL\n");
		exit(0);
	}
	else {
		glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
		glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
		glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
		glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
		glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
		glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
		glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
		glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)wglGetProcAddress("glPatchParameteri");
		glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
		glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
		glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
		glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
		glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
		glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
		glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
		glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
		glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
		glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
		glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
		glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");

		glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
		glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
		glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
		glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
		glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	}

}

void Keyboard(unsigned char key, int x, int y)
{
	if (key == 'w') {
		eyez -= 0.05;
	}
	else if (key == 's') {
		eyez += 0.05;
	}

	// odrysowanie okna
	resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

void SpecialKeys(int key, int x, int y) //funkcja obsługi klawiatury
{
	GLfloat cameraSpeed = 1;
	switch (key)
	{
		// kursor w lewo
	case GLUT_KEY_LEFT:
		/*LGx -= 0.1;
		LDx -= 0.1;
		PGx -= 0.1;
		PDx -= 0.1;*/
		//eyex -= cameraSpeed;
		centerx -= cameraSpeed;
		break;

		// kursor w górę
	case GLUT_KEY_UP:
		/*LGy -= 0.1;
		LDy -= 0.1;
		PGy -= 0.1;
		PDy -= 0.1;*/
		//eyey += cameraSpeed;
		centery += cameraSpeed;
		break;

		// kursor w prawo
	case GLUT_KEY_RIGHT:
		/*LGx += 0.1;
		LDx += 0.1;
		PGx += 0.1;
		PDx += 0.1;*/
		//eyex += cameraSpeed;
		centerx += cameraSpeed;
		break;

		// kursor w dół
	case GLUT_KEY_DOWN:
		/*LGy += 0.1;
		LDy += 0.1;
		PGy += 0.1;
		PDy += 0.1;*/
		//eyey -= cameraSpeed;
		centery -= cameraSpeed;
		break;
	}

	resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

// Main routine: defines window properties, creates window,
// registers callback routines and begins processing.
int main(int argc, char** argv)
{
	//display(heightmap);
	std::random_device rd;
	std::mt19937 generator(rd());
	uniform_real_distribution<float> hillRadiusDistribution(hillRadiusMin, hillRadiusMax);
	uniform_real_distribution<float> hillHeightDistribution(hillMinHeight, hillMaxHeight);
	//the same as coordinates of terrain
	uniform_real_distribution<float> hillCenterRowIntDistribution(-0.2, 0.6);
	uniform_real_distribution<float> hillCenterColIntDistribution(-0.0, 0.8);
	float x;
	float y;
	for (int i = 0; i < numHills*2; i=i+2) {
		do {
			x = hillCenterRowIntDistribution(generator);
			y = hillCenterColIntDistribution(generator);
		} while (x * x + y * y < 0.5);

		hillsCenters[i] = x;
		hillsCenters[i+1] = y;
	}

	hillHeight = hillHeightDistribution(generator);
	hillRadius = hillRadiusDistribution(generator);

	// Initialize GLUT.
	glutInit(&argc, argv);

	// Set display mode as single-buffered and RGB color.
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

	// Set OpenGL window size.
	glutInitWindowSize(500, 500);

	// Set position of OpenGL window upper-left corner.
	glutInitWindowPosition(100, 100);

	// Create OpenGL window with title.
	glutCreateWindow("cw6 - Ekspozja");

	// Initialize.
	//createHouses();
	makeHouse();
	setup();
	extensionSetup();
	terrain();

	// Register display routine.
	glutDisplayFunc(drawScene);
	// Register reshape routine.
	glutReshapeFunc(resize);
	glutSpecialFunc(SpecialKeys);
	glutKeyboardFunc(Keyboard);

	houseProgramHandle = glCreateProgram();
	terrainProgramHandle = glCreateProgram();
	setShaders(terrainProgramHandle, "subdiv.vs", "subdiv.fs", "terrain.tcs", "terrain.es");								
	setShaders(houseProgramHandle, "house.vs", "house.fs", "house.tcs", "house.es");

	setUniformHouse();
	setUniformTerrain();

	// Begin processing.
	glutMainLoop();


	return 0;
}








