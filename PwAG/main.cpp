#define _CRT_SECURE_NO_DEPRECATE
#include <cstdlib>
#include <iostream>
#include <fstream>
#define GLUT_DISABLE_ATEXIT_HACK
#include "GL/glut.h"
#include "GL/glext.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/type_ptr.hpp"
#include <time.h>     
#include <algorithm>


using namespace std;

static int programHandle; // obiekt programu
static int vertexShaderHandle; // obiekt shadera wierzcho�k�w
static int fragmentShaderHandle; // obiekt shadera fragment�w
static int geomShaderHandle; // obiekt shadera geometrii
static int tcsShaderHandle; // obiekt shadera geometrii
static int esShaderHandle; // obiekt shadera geometrii

GLuint* terrainText = new GLuint[2];

//===================================================================================================================================================================================
//Domek
/*
	Do generacji domku potrzebne jest par� parametr�w: rozmieszczenie wierzcho�k�w podstawy, wysoko�� �cian, wysoko�� i k�t nachylenia dachu. Przekazywanie tego
	wszystkiego mo�e by� niewydajne (te dane trzeba by by�o gdzie� przechowywa�). Zamiast tego mo�na rpzekaza� tylko cz�� informacji i wyliczy� reszt� na ich
	podstawie. Po analizie problemu stwierdzam, �� wymagane b�d� tak naprawd� trzy rzeczy:
	a)punkt �rodkowy domku (punkt przeci�cia przek�tnych prostok�ta b�d�cego podstaw�) - punkt A (jego wsp�rz�dne x, y, z)		//uwaga: dla uproszczenia: z odpowiada za wysoko��
	b)dowolny wierzcho�ek, kt�ry nale�y do postawy (z takie same co punktu A), a warto�ci x i y r�ni� si� na pewo o jak�� warto�� (np. 5<|x-x`|<10 i 10<|y-y`|<20 -> punkt B
	c)wysoko�� �ciany (sta�a warto��, np. 6, czy 10) -> oznaczone jako zmienna H

	Dzi�ki temu jeste�my w stanie wyliczy� pozosta�e wierzcho�ki domku, kt�ry b�dzie sk��da� si� z prostopad�o�cianu i po�o�emu na nim spadzistym dachu (granistos�up o podstawie
	tr�jk�ta r�wnoramiennego, kt�rego �ciana le��ca na boku nie b�d�cym ramieniem le�y na prospopad�o�cianie i si� z nim pokrywa).

	Pos�u�my si� ozanaczeniami literowymi dla kolejnych punkt�w charakterystycznych:
	0  A - punkt b�d�cy centrum domku
	1  B - jeden z wierzcho�k�w le��cych na podstawie domku
	2  B`- wierzcho�ek le��cy na podstawie domku, nale��cy do tej samej przek�tnej co punkt B (le�y na prostej przechodz�cej przez punktu AB)
	3  C - wierzho�ek le�acy na podstawie, s�siadujacy kr�tsz� �cian� z punktem B i d�u�sz� z punktem B`
	4  C`- analogicznie, wierzcho�ek le��cy na przek�tnej z punktem C, kr�tsz� �cian� s�siaduj�cy z B`, a d�u�sz� z punktem B
	5  D - punkt le��cy nad punktem B
	6  D`- punkt le��cy nad punktem B`
	7  E - punkt le��cy nad punktem C
	8  E`- punkt le��cy nad punktem C`
	9  F - punkt b�d�cy szczytem dachu - punktem przy ramionach tr�jk�ca b�d�cego podstaw� graniastos�upa -> le��cy na tej samej p�aszczy�nie co punkty B, C, D, E
	10 F`- analogiczny punkt do F, z tym, z� le��cy na p�aszczy�nie z punktami B`, C`, D`, E`

	Wida� tu, �e przechowuj�c po�o�enie tych punkt�w wymagane by by�o przechowywanie po 3 float'�w dla 11 punkt�w (33 float'y na domek). Zamiast tego da si� wszystko zapami�ta�
	dzi�ki jedynie 2 punktom (A i B) oraz warto�ci wysoko�ci �ciany (7 float'�w (tak naparwd� 6 i 1 int).

	Znane jest po�o�enie punkt�w A i B (A.x, A.y, A.z, B.x, B.y. B.z). Obliczmy warto�� X i Y jako odleg�o�ci odpowiadaj�cych wsp�rz�dnych punkt�w A i B:
	float X = A.x-B.x;
	float Y = A.y-B.y;
	Dlaczego nie warto�ci bezwzl�dne? Bo nie s� potrzebne, warto�ci X i Y mog� by� ujemne, co z punktu geometrii nie ma sensu (ujemne d�ugo�ci), ale nas obchodzi orientacja punkt�w
	wzgl�dem siebie. Dzi�ki temu zawsze mamy: B.x = A.x + X; B.y = A.y + Y;
	Na tej podstawie mo�emy wyznaczy� po�o�enie punktu B` -> B`.x = A.x - X; B`.y = A.y - Y;
	Teraz aby otrzyma� prostok�t powinno si� dobra� wierzcho�ki C i C`, aby odcinki |BC| i |B`C| by�y do siebie prostopad�e. Prawd� m�wi�c pocz�tkowo my�la�em, czy nie wykorzysta�
	w�a�ciwo�ci przek�tnych, tego, �e da si� opisa� okr�g na prostok�cie i bawi� si� z k�tami mi�dzy przek�tnymi... Ale w zasadzie da si� uzysta� dobry rezultat (na 99%), je�eli
	za�o�y si�:
	C.x = A.x-X; C.y = B.y; (, tudzie� C.y = A.y + Y;) {oczywi�cie C.z = A.z;}
	C`.x = B.x; (, tudzie� C.x = A.x + X;) C`.y = A.y - Y; {oczywi�cie C`.z te� jest r�wne A.z}

	Mamy teraz ca�� podstaw�, w kolejnym kroku nale�y wyznaczy� "sufit". W tym celu przyda si� ostatni podany parametr - wysoko�� �ciany. Korzystaj�c z rozpisanych wcze�niej punkt�w
	mo�emy w prosty spos�b wyznaczy� wierzcho�ki drugiej cz�ci prostopad�o�cianu:
	D.x = B.x; D.y = B.y; D.z = B.z + H;
	D`.x = B`.x; D`.y = B`.y; D`.z = B`.z + H;
	E.x = C.x; E.y = C.y; E.z = C.z + H;
	E`.x = C`.x; E`.y = C`.y; E`.z = C`.z + H;

	Pozosta�y nam ju� tylko dwa wierzcho�ki tworz�ce g�rn� belk� dachu. W tym celu zak��damy, �e wysoko�� dachu jest r�wna po�owie wysoko�ci �ciany, a punkty owe le�� po�rodku
	kr�tszych �cian:
	F.x = A.x; F.y = B.y; F.z = A.z + 1.5 * H;
	F`.x = A.x; F`.y = B`.y; F`.z = A.z + 1.5 * H;

	Teraz posiadaj�c wszystkie wierzcho�ki mo�na zbudowa� tr�jk�ty. Tutaj mo�e si� okaza�, �� kolejno�� jest odwrotna, przez co niekt�re �ciany mog� by� zwr�cone w przeciwn� stron�,
	ale to nie b�dzie problemem przy za��czeniu opcji, aby rendorowa�y si� obie strony �cian. Tak samo niepotrzebne jest renderowanie spodniej cz�cie budynku, jednak mo�na to zrobi�,
	dzi�ki czemu otrzyma si� zamkni�t� bry��.
	Tr�jk�ty:
	0  BCB`		132
	1  B`C`B	241
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
	Mamy wi�c 16 tr�jk�t�w oraz 11 (w tym A) wierzcho�k�w. Potrzebne by wi�c by�y nast�puj�ce struktury (zak��daj�c dost�pne punkty A, B oraz wysoko�� �ciany H)

	float X = A.x-B.x;
	float Y = A.y-B.y;

	static GLfloat vdata[11][3] = {
		{A.x, A.y, A.z}, {B.x, B.y, B.z}, {A.x-X, A.y-Y, A.z}, {A.x-X, B.y, A.z}, {B.x, A.y-Y, A.z},
		{B.x, B.y, A.z+H}, {A.x-X, A.y-Y, A.z+H}, {A.x-X, B.y, A.z+H}, {B.x, A.y-Y, A.z+H},
		{A.x, B.y, A.z+1.5*H}, {A.x, A.y-Y, A.z+1.5*H}
	};
	static GLuint tindices[16][3] = {
		{1,3,2}, {2,4,1}, {2,7,3}, {6,7,2},
		{3,7,1}, {7,5,1}, {2,6,4}, {4,6,8},
		{8,5,4}, {1,4,5}, {7,9,5}, {2,10,8}
		{6,10,8}, {10,9,7}, {9,10,8}, {8,5,9}
	};


	W zasadzie problemem jest to, �e nie jestem pewien jak to zaimplementowa� po stronie karty graficznej, przepraszam �� jeszcze tego nie zrobi�em
	Tabela do przechowywania danych o po�o�eniu i rozmiarze budynk�w:
	float domki[liczba_domkow][7] = {
	{A.x.1, A.y.1, A.z.1, B.x.1, B.y.1, B.z.1, H.1},
	{A.x.2, A.y.2, A.z.2, B.x.2, B.y.2, B.z.2, H.2},
	...
	{A.x.n, A.y.n, A.z.n, B.x.n, B.y.n, B.z.n, H.n}
	};

*/
//===================================================================================================================================================================================



#define MAX_HOUSE_COUNTER		10
#define HOUSE_QUANTITY			3
#define HOUSE_COUNTER			min(HOUSE_QUANTITY,MAX_HOUSE_COUNTER)
#define MIN_X					-1
#define MAX_X					1
#define MIN_Y					-1
#define MAX_Y					1
#define PROPORTION				0.2
#define WALL_LENGTH				(abs(MIN_X) + abs(MAX_X))*PROPORTION*0.5

//[7] -> [P1.x, P1.y, P1.z, P2.x, P2.y, P2.z, wallH] 
float houses[HOUSE_COUNTER][7] = {
	{ 0.0f, 0.0f, 0.0f, 5.0f, 10.0f, 0.0f, 5.0f }
};

unsigned int texture1;
unsigned int texture2;

static GLint houseNumLocation;
//static GLint houseFirstPartcLocation;
//static GLint houseSecoundPartcLocation;
//static GLint houseThirdPartcLocation;
//static GLint houseFourthPartcLocation;
//static GLint houseFifthPartcLocation;
//static GLfloat test[7] = { 1,2,3,4,5,6,7 };
float wallHeight = 0;

GLuint vbo_id[2];
GLfloat xyz[20 * 3][3] = { 0 };

//wska�niki funkcji
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
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

PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f = NULL;

PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORM1FVPROC glUniform1fv = NULL;
PFNGLUNIFORM3FPROC glUniform3f = NULL;
PFNGLUNIFORM4FPROC glUniform4f = NULL;
PFNGLUNIFORM3FVPROC glUniform3fv = NULL;
PFNGLUNIFORM4FVPROC glUniform4fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = NULL;
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
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

GLfloat Z = 0;

static GLfloat vhouse[HOUSE_COUNTER * 42][5] = { 0 };

static GLfloat vdata[6][5] = { 
	{-0.5, Z, 0.0, 1.0, 1.0}, {-0.5, Z, 0.5, 1.0, 0.0 }, {0.5, Z, 0.5, 0.0, 0.0},
	{0.5, Z, 0.5, 1.0, 1.0}, {0.5, Z, 0.0, 1.0, 0.0}, {-0.5, Z, 0.0, 0.0}
};

/*
	Funkcja, kt�ra przypisuje warto�ci punktu we wskazane miejsce w tablicy domk�w. Bez tego funkcja createHouses() by�aby BARDZO d�uga...
	Jako pola 4 i 5 ([][3], [][4]) wpisuje warto�ci losowe 0, albo 1 (dzieki temu tekstura si� jako� wczyta - to do test�w)
*/
void copyPoint(int i, int j, int t, glm::vec3 Point)
{
	srand(time(NULL));
	/* PIERWOWZ�R
	vhouse[i * 42 + j][0] = Point.x;
	vhouse[i * 42 + j][1] = Point.y;
	vhouse[i * 42 + j][2] = Point.z;
	*/
	vhouse[i * 42 + j][0] = Point.x;
	vhouse[i * 42 + j][1] = Point.y;
	vhouse[i * 42 + j][2] = Point.z;
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
	else
	{
		vhouse[i * 42 + j][3] = 0.0;
		vhouse[i * 42 + j][4] = 0.0;
	}
}

/*
	Aby wygenerowa� domki nale�y przes�a� pewn� ilo�� wierzcho�k�w do potoku graficznego
	Domek posiada 10 wierzcho�k�w, kt�re buduj� jego szkielet oraz jedynasty - punkt �rodkowy, potrzebny do okre�lenia po�o�enia pozosta�ych wierzcho�k�w
	Jednak�e �ciany b�d� zbudowane z tr�jk�t�w, a tr�jk�ty b�dziemy wykorzystywa� przy renderowaniu. Z tego te� powodu wysy�amy:
	(3+3)*6 + 3 + 3 = 42 wierzcho�ki na domek. 

	Potrzebne b�dzie wylosowanie 5 liczb (przy za�o�eniu, �e domki b�d� le�a�y na jednej wysoko�ci): warto�ci x,y dla punktu centalnego - A, warto�ci x,y dla jednego wierzcho�ka podstawy - B oraz warto�ci wysoko�ci �ciany domku. 
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

		//wallHeight = max((float)(WALL_LENGTH/4.0) ,(static_cast <float> (rand()) / static_cast <float> (RAND_MAX/(WALL_LENGTH/2.0))));

		//je�eli mamy ju� jakie� punkty A, to musimy sprawdzi�, czy nie le�� za blisko siebie - czy zbudowane na nich domki nie b�d� na siebie nachodzi�
		if (posATab[0].x != 0 || posATab[0].y != 0 || posATab[0].z != 0)		//je�li tak, to znaczy, �e mamy ju� jakie� domki
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
						if (pow(posATab[j].x - posA.x, 2.0) + pow(posATab[j].y - posA.y, 2.0) <= pow(WALL_LENGTH * 2, 2.0) )
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
			//teraz mamy ju� pewno��, �e zbudowane domki nie b�d� na siebie nachodzi�
			posATab[i].x = posA.x;
			posATab[i].y = posA.y;
			posATab[i].z = posA.z;

			//houseX = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 2.0));
			houseX = (max((float)0.4, (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 1.5))))* (float)WALL_LENGTH;
			posB.x = posA.x + houseX;	//Punkt B w OSI X b�dzie oddalony od punktu A o warto�� <0.2-0.5> bazowej d�ugo�ci �ciany
			houseY = (max((float)0.2, (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 3.0))))* (float)WALL_LENGTH;
			posB.y = posA.y + houseY;	//Punkt B w OSI Y b�dzie oddalony od punktu A o warto�� <0.1-0.25> bazowej d�ugo�ci �ciany
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
			posF.y = posB.y;
			posF.z = posA.z + 1.5 * wallHeight;

			posFp.x = posA.x;
			posFp.y = posBp.y;
			posFp.z = posA.z + 1.5 * wallHeight;

			//teraz mamy ju� wszystkie wierzcho�ki domku. Pozosta�o po��czy� je w tr�jk�ty i odpowiednio wpakowa� do tabeli
			/*
				Tr�jk�ty:

			#0  BCB`		132		->je�liby�my chcieli robi� te� pod�og�
			#1  B`C`B		241		->je�liby�my chcieli robi� te� pod�og�

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

			copyPoint(i, 30, 0 ,posDp);
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
	//sko�czono generowa� domki
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

	
	glGenBuffers(1, vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vdata)	, vdata, GL_STATIC_DRAW);		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	glBufferData(GL_ARRAY_BUFFER, sizeof(vhouse), vhouse, GL_STATIC_DRAW);
	

	glGenTextures(2, terrainText);
	//loadTerrain(terrainText[0], "grass.jpg");
	//loadTerrain(terrainText[1], "soil.jpg");
	loadTerrain(terrainText[0], "roof.jpg");
	loadTerrain(terrainText[1], "wallW.jpg");
}

void drawTerrain() {		//tutaj dzieje si� magia

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid*)0);	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,  5 * sizeof(GLfloat), (const GLvoid*)0);
	glEnableVertexAttribArray(0);

	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(programHandle, "texture1"), 0);
	glBindTexture(GL_TEXTURE_2D, terrainText[0]);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(programHandle, "texture2"), 1);
	glBindTexture(GL_TEXTURE_2D, terrainText[1]);
	

	//glDrawArrays(GL_PATCHES, 0, 6);
	glDrawArrays(GL_PATCHES, 0, 42 * HOUSE_COUNTER);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);


}

// incjalizacja shader�w
void setShaders(const char* vertexShaderFile, const char* fragmentShaderFile, const char* geom, const char* tcs, const char* es)
{
	GLint status = 0;

	char* vertexShader = readShader(vertexShaderFile);
	char* fragmentShader = readShader(fragmentShaderFile);
	char* geomShader = readShader(geom);
	char* tcsShader = readShader(tcs);
	char* esShader = readShader(es);

	programHandle = glCreateProgram(); // tworzenie obiektu programu
	vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER); // shader wierzcho�k�w
	fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER); // shader fragment�w
	geomShaderHandle = glCreateShader(GL_GEOMETRY_SHADER); // shader fragment�w
	tcsShaderHandle = glCreateShader(GL_TESS_CONTROL_SHADER); // shader tcs
	esShaderHandle = glCreateShader(GL_TESS_EVALUATION_SHADER); // shader es

	glShaderSource(vertexShaderHandle, 1, (const char**)&vertexShader, NULL); // ustawianie �r�d�a shadera wierzcho�k�w
	glShaderSource(fragmentShaderHandle, 1, (const char**)&fragmentShader, NULL); // ustawianie �r�d�a shadera fragment�w
	glShaderSource(tcsShaderHandle, 1, (const char**)&tcsShader, NULL); // ustawianie �r�d�a shadera fragment�w
	glShaderSource(esShaderHandle, 1, (const char**)&esShader, NULL); // ustawianie �r�d�a shadera fragment�w

	// kompilacja shader�w
	glCompileShader(vertexShaderHandle);
	glCompileShader(fragmentShaderHandle);
	glCompileShader(geomShaderHandle);
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


	//dodanie shader�w do programu
	glAttachShader(programHandle, vertexShaderHandle);
	glAttachShader(programHandle, tcsShaderHandle);
	glAttachShader(programHandle, esShaderHandle);
	glAttachShader(programHandle, fragmentShaderHandle);
	//glAttachShader(programHandle, geomShaderHandle);

	/* link */
	//uruchomienie
	glLinkProgram(programHandle);

	glGetObjectParameterivARB(programHandle, GL_OBJECT_LINK_STATUS_ARB, &status);
	if (!status) {
		const int maxInfoLogSize = 2048;
		GLchar infoLog[maxInfoLogSize];
		glGetInfoLogARB(programHandle, maxInfoLogSize, NULL, infoLog);
		std::cout << infoLog;
	}

	//======================================================================================
	//przekazywanie parametr�w przez uniforma

	houseNumLocation = glGetUniformLocation(programHandle, "houseNum");
	//houseFirstPartcLocation = glGetUniformLocation(programHandle, "houseFirstPart");
	//houseSecoundPartcLocation = glGetUniformLocation(programHandle, "houseSecoundPart");

	//glUniform3fv

	//inicjalizajca wektora, kt�rym b�dziemy przekazywali dane
	std::vector<GLfloat> houseDataVec;
	//glm::vec3 firstPart;// [houseCounter] ;
	//glm::vec4 secoundPart;// [houseCounter] ;
	
	//GLfloat* test = new GLfloat [7];
	//test[0] = 10;
	//test[6] = 5;


	//przekazujemy jako osobn� zmienn� liczb� domk�w. Skoro ka�dy domek to 7 float'�w, to w GPU b�dziemy mogli dzieki temu wiedzie� ile zmiennych mamy odczyta� z wektora
	glUseProgram(programHandle);
	glUniform1i(houseNumLocation, HOUSE_COUNTER);
	GLint wallHLocation = glGetUniformLocation(programHandle, "wallH");
	GLint ZLocation = glGetUniformLocation(programHandle, "Zvalue");
	glUseProgram(programHandle);
	glUniform1f(wallHLocation, wallHeight);
	glUniform1f(ZLocation, Z);

	//przekazywanie zmiennych do vectora
	/*
	for (int i = 0; i < houseCounter; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			//houseDataVec.push_back(houses[i][j]);
			//test[j + 7 * 1] = houses[i][j];
			if (j < 3)
			{
				firstPart[j] = houses[i][j];
			}
			else
			{
				secoundPart[j-3] = houses[i][j];
			}
		}
	}
	*/
	//glUseProgram(programHandle);
	//houseNumLocation = glGetUniformLocation(programHandle, "houseNum");
	//glUniform1f(houseNumLocation, houseCounter);
	/*
	GLfloat params[3];
	params[0] = 2.0f;
	params[1] = 1.0f;
	params[2] = 1.0f;

	GLint colorRampUniformLocation = glGetUniformLocation(programHandle, "params");
	glUniform1fv(colorRampUniformLocation, 3, params);
	*/

	//float threshold[4] = { 0.5f, 0.25f };
	//float threshold[2][4] = { {0.5f}, {0.25f} };

	//glUniform4fv(glGetUniformLocation(programHandle, "t"), 2, threshold);

	/*
	GLfloat test[6];
	for (int i = 0; i < 6; i++)
	{
		test[i] = i;
	}

	GLfloat test2[8];
	for (int i = 0; i < 8; i++)
	{
		test2[i] = i;
	}
	glUniform3fv(houseFirstPartcLocation, 2, test);
	glUniform4fv(houseSecoundPartcLocation, 2, test2);
	*/
	//houseFirstPartcLocation = glGetUniformLocation(programHandle, "houseFirstPart");
	//glUniform3f(houseFirstPartcLocation, 7.0f, 5.0f, 4.0f);
	//glUniform1fv(houseFirstPartcLocation,  7, test);
	//glUniform4f(houseFirstPartcLocation, glm::value_ptr(secoundPart));

	//przekazujemy wektor z danymi do tworzenia domk�w
	//glUseProgram(programHandle);
	//int i = houseDataVec[4];
	//GLfloat *tmp = reinterpret_cast<GLfloat*>(&houseDataVec[0]);
	//test[0] = 10;
	//glUniform1fv(houseDataVecLocation, houseDataVec.size(), tmp);
	//glUniform1fv(houseDataVecLocation, 7*houseCounter, test);
	//=====================================================================================
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


	glPatchParameteri(GL_PATCH_VERTICES, 3);

	glUseProgram(programHandle);

	//funkcje, kt�re b�d� cos wy�wietla�y
	drawTerrain();	

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
	gluPerspective(-80, w / h, 0.1, 1000);

	//
	// glFrustum(-10.0, 10.0, -10.0, 10.0, 0.0, 200.0);
	// Set matrix mode to modelview.
	glMatrixMode(GL_MODELVIEW);

	// Clear current modelview matrix to identity.
	glLoadIdentity();
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
		glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
		glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
		glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
		glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
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

// Main routine: defines window properties, creates window,
// registers callback routines and begins processing.
int main(int argc, char** argv)
{
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

	// Initialize.		//tutaj przygotowujemy wszystko co si� p�niej rpzyda: rpzekazujemy zmienne dalej, etc.
	createHouses();	
	setup();
	extensionSetup();
	terrain();
	//createHouses();		//cholernie wa�na rzecz: powinno si� wype�nia� tablic� z punktami ZANIM si� j� wy�le do shader�w. A nie po tym -.-

	// Register display routine.
	glutDisplayFunc(drawScene);		//tutaj faktycznie co� rysujemy
	// Register reshape routine.
	glutReshapeFunc(resize);

	setShaders("subdiv.vs", "subdiv.fs", "subdiv_p.geom", "terrain.tcs", "terrain.es");

	// Begin processing.
	glutMainLoop();

	return 0;
}








