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
	Dlaczego nie warto�ci bezwzl�dne? Bo nie s� potrzebne, warto�ci X i Y mog� by� ujemne, co z punktu geometrii nie ma sensu (sujemne d�ugo�ci), ale nas obchodzi orientacja punkt�w
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


#define houseCounter  1
//[7] -> [P1.x, P1.y, P1.z, P2.x, P2.y, P2.z, wallH] 
static float houses[houseCounter][7] = {
	{ 0.0f, 0.0f, 0.0f, 5.0f, 10.0f, 0.0f, 5.0f }
};

unsigned int texture1;
unsigned int texture2;

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
PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORM1FVPROC glUniform1fv = NULL;
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

static GLfloat vdata[6][5] = { 
	{-0.5, Z, 0.0, 1.0, 1.0}, {-0.5, Z, 0.5, 1.0, 0.0 }, {0.5, Z, 0.5, 0.0, 0.0},
	{0.5, Z, 0.5, 1.0, 1.0}, {0.5, Z, 0.0, 1.0, 0.0}, {-0.5, Z, 0.0, 0.0}
};


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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vdata)	, vdata, GL_STATIC_DRAW);
	

	glGenTextures(2, terrainText);
	loadTerrain(terrainText[0], "soil.jpg");
	loadTerrain(terrainText[1], "grass.jpg");
}

void drawTerrain() {

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid*)0);
	glEnableVertexAttribArray(0);

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
	

	glDrawArrays(GL_PATCHES, 0, 6);


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	//======================================================================================
	//przekazywanie parametr�w przez uniforma

	GLint houseNumLocation = glGetUniformLocation(programHandle, "houseNum");
	GLint houseDataVecLocation = glGetUniformLocation(programHandle, "houseDataVec");

	//glUniform3fv

	//inicjalizajca wektora, kt�rym b�dziemy rpzekazywali dane
	std::vector<GLfloat> houseDataVec;

	//przekazujemy jako osobn� zmienn� liczb� domk�w. Skoro ka�dy domek to 7 float'�w, to w GPU b�dziemy mogli dzieki temu wiedzie� ile zmiennych mamy odczyta� z wektora
	glUseProgram(programHandle);
	glUniform1f(houseNumLocation, houseCounter);

	
	//przekazywanie zmiennych do vectora
	for (int i = 0; i < houseCounter; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			houseDataVec.push_back(houses[i][j]);
		}
	}

	//przekazujemy wektor z danymi do tworzenia domk�w
	glUseProgram(programHandle);
	glUniform1fv(houseDataVecLocation, houseDataVec.size(), reinterpret_cast<GLfloat*>(houseDataVec.data()));
	//=====================================================================================
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

	// Initialize.
	setup();
	extensionSetup();
	terrain();

	// Register display routine.
	glutDisplayFunc(drawScene);
	// Register reshape routine.
	glutReshapeFunc(resize);

	setShaders("subdiv.vs", "subdiv.fs", "subdiv_p.geom", "terrain.tcs", "terrain.es");

	// Begin processing.
	glutMainLoop();

	return 0;
}








