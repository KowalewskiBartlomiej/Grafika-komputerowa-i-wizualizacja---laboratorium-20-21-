/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdlib.h>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <map>
#include <ctime>
#include <stdio.h>

#include "constants.h"
#include "iostream"
#include "shaderprogram.h"
#include "model.h"

using namespace std;

GLuint tex;
// Wektory przesyłane do shadera
vector<glm::vec4> verts;
vector<glm::vec4> norms;
vector<glm::vec2> texCoords;
vector<unsigned int> indices;

//Zmienne pomocnicze
float speed_x = 0.01;//[radiany/s]
float speed_y = 0.01;//[radiany/s]
bool whites_move = true;
clock_t timer;
int moves_counter = 1;
int castles[4][4];
int castles_counter = 0;
int starting_up;
unsigned int WIDTH = 1080;
unsigned int HEIGTH = 1080;
float aspect_ratio = 1080 / 1080;
// // // // // // // // // // // //
glm::vec3 chessboard[8][8]; //SZACHOWNICA

//Wektory figur
vector<Model*> white;
vector<Model*> black;
Model* board;

ShaderProgram* shader;


//Funkcja wyszukująca indeks pionka na danym polu w wektorze figur
int findIndex(int row, int column, vector<Model*> pieces)
{
	for (int i = 0; i < 16; i++)
	{
		if (row == pieces[i]->row && column == pieces[i]->column && pieces[i]->visible)
		{
			return i;
		}
	}
	return -1;
}

//Poruszanie się figury i sprawdzanie bicia
void movePiece(int index1, int index2, vector<Model*> pieces1, vector<Model*> pieces2, int row, int column)
{
	if (index1 != -1)
	{
		pieces1[index1]->row = row;
		pieces1[index1]->column = column;
	}
	if (index2 != -1) // Warunek sprawdzający, czy występuje bicie
	{
		pieces2[index2]->visible = false;
	}
}

//Zliczanie ruchów w pliku
const int countMoves(string filename)
{
	string line;
	ifstream input(filename);
	int counter = 0;
	while (getline(input, line))
		++counter;

	input.close();
	return counter;
}

//Wypełnianie tablicy ruchów
void fillMovesArreay(int moves[][4], string filename)
{
	string line;
	ifstream input(filename);
	map<char, int> fields;

	fields.insert(pair<char, int>('a', 0));
	fields.insert(pair<char, int>('b', 1));
	fields.insert(pair<char, int>('c', 2));
	fields.insert(pair<char, int>('d', 3));
	fields.insert(pair<char, int>('e', 4));
	fields.insert(pair<char, int>('f', 5));
	fields.insert(pair<char, int>('g', 6));
	fields.insert(pair<char, int>('h', 7));

	int i = 0;
	int j = 0;

	for (line; getline(input, line);)
	{
		if (line[2] == '-')
		{
			moves[i][0] = fields[line[0]];
			moves[i][1] = (line[1] - '0') - 1;
			moves[i][2] = fields[line[3]];
			moves[i][3] = (line[4] - '0') - 1;
		}
		else
		{
			int king_col = fields[line[0]];
			int rook_col = fields[line[3]];
			moves[i][0] = 9;
			if (king_col > rook_col)
			{
				castles[j][0] = king_col;
				castles[j][1] = (line[1] - '0') - 1;
				castles[j][2] = king_col - 2;
				castles[j][3] = (line[4] - '0') - 1;

				castles[j+1][0] = rook_col;
				castles[j+1][1] = (line[1] - '0') - 1;
				castles[j+1][2] = rook_col + 3;
				castles[j+1][3] = (line[4] - '0') - 1;

			}
			else
			{
				castles[j][0] = king_col;
				castles[j][1] = (line[1] - '0') - 1;
				castles[j][2] = king_col + 2;
				castles[j][3] = (line[4] - '0') - 1;

				castles[j + 1][0] = rook_col;
				castles[j + 1][1] = (line[1] - '0') - 1;
				castles[j + 1][2] = rook_col - 2;
				castles[j + 1][3] = (line[4] - '0') - 1;

			}
			j += 2;
		}
		i++;
	}
}

//Tworzenie szachownicy
void createChessboard()
{
	glm::vec3 A1 = glm::vec3(16.8f, 0.0f, 16.75f); //Współrzędne A1
	// 4.8f - szerokość pola szachownicy

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			chessboard[i][j] = A1;
			chessboard[i][j].x -= 4.8f * i;
			chessboard[i][j].z -= 4.8f * j;
		}
	}
}

//Funkcja tworząca figury na odpowiednich pozycjach
void createModels()
{
	for (int i = 0; i < 8; i++)
	{
		white.push_back(new Model("my_models/whitepieces/pawn/Pawn.obj", i, 1));
		black.push_back(new Model("my_models/darkpieces/pawn/Pawn.obj", i, 6));
	}
	white.push_back(new Model("my_models/whitepieces/bishop/Bishop.obj", 2, 0));
	white.push_back(new Model("my_models/whitepieces/bishop/Bishop.obj", 5, 0));
	white.push_back(new Model("my_models/whitepieces/knight/Knight.obj", 1, 0));
	white.push_back(new Model("my_models/whitepieces/knight/Knight.obj", 6, 0));
	white.push_back(new Model("my_models/whitepieces/rook/Rook.obj", 0, 0));
	white.push_back(new Model("my_models/whitepieces/rook/Rook.obj", 7, 0));
	white.push_back(new Model("my_models/whitepieces/queen/Queen.obj", 3, 0));
	white.push_back(new Model("my_models/whitepieces/king/King.obj", 4, 0));

	black.push_back(new Model("my_models/darkpieces/bishop/Bishop.obj", 2, 7));
	black.push_back(new Model("my_models/darkpieces/bishop/Bishop.obj", 5, 7));
	black.push_back(new Model("my_models/darkpieces/knight/Knight.obj", 1, 7));
	black.push_back(new Model("my_models/darkpieces/knight/Knight.obj", 6, 7));
	black.push_back(new Model("my_models/darkpieces/rook/Rook.obj", 0, 7));
	black.push_back(new Model("my_models/darkpieces/rook/Rook.obj", 7, 7));
	black.push_back(new Model("my_models/darkpieces/queen/Queen.obj", 3, 7));
	black.push_back(new Model("my_models/darkpieces/king/King.obj", 4, 7));
}

void window_callback(GLFWwindow* window, int width, int heigth)
{
	if (heigth == 0)
	{
		return;
	}
	WIDTH = width;
	HEIGTH = heigth;
	aspect_ratio = (float)width / (float)heigth;
	glViewport(0, 0, width, heigth);
}

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void key_callback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mod
) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) {
			speed_y = PI;
		}
		if (key == GLFW_KEY_RIGHT) {
			speed_y = -PI;
		}
		if (key == GLFW_KEY_UP) {
			speed_x = -PI;
		}
		if (key == GLFW_KEY_DOWN) {
			speed_x = PI;
		}
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
			speed_y = 0;
		}
		if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) {
			speed_x = 0;
		}
	}
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    initShaders();
	glClearColor(0, 0, 0, 1); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glEnable(GL_BLEND);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_callback);

	shader = new ShaderProgram("v_phong.glsl", NULL, "f_phong.glsl");
	board = new Model("my_models/board/Board.obj");
	createModels();
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
	glDeleteTextures(1, &tex);
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y, int moves[][4], int number_of_moves) {
	//Zmienne pomocnicze
	float x = 20.0f;
	float z = 20.0f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości

	//Tablice wektorów z pozycjami świateł oraz ich barwami
	
	glm::vec3 lights_positions[5] = { glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(x, 15.0f, z), glm::vec3(-x, 15.0f, -z), glm::vec3(x, 15.0f, -z), glm::vec3(-x, 15.0f, z) };
	glm::vec3 colors[5] = { glm::vec3(1.0f, 0.95f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f) };
	glm::vec3 view = glm::vec3(40.0f * cos(angle_y), 30.0f, -40.0f * sin(angle_y));

	glm::mat4 M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
	glm::mat4 V = glm::lookAt(view, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(glm::radians(75.0f), aspect_ratio, 1.0f, 75.0f); //Wylicz macierz rzutowania
	shader->use();
	glUniform3fv(shader->u("light_pos"), 5, &lights_positions[0][0]);
	glUniform3fv(shader->u("camera_pos"), 1, &view[0]);
	glUniform3fv(shader->u("light_color"), 5, &colors[0][0]);
	glUniformMatrix4fv(shader->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(shader->u("V"), 1, false, glm::value_ptr(V));

	//Rysowanie figur na szachownicy

	for (int i = 0; i < white.size(); i++)
	{
		if (white[i]->visible)
		{
			M = glm::mat4(1.0f);
			M = glm::translate(M, chessboard[white[i]->row][white[i]->column]);

			glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(M));

			white[i]->Draw(*shader);
		}
	}

	for (int i = 0; i < black.size(); i++)
	{
		if (black[i]->visible)
		{
			M = glm::mat4(1.0f);
			M = glm::translate(M, chessboard[black[i]->row][black[i]->column]);
			glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(M));

			black[i]->Draw(*shader);
		}
	}

	// Rysowanie planszy

	M = glm::mat4(1.0f);
	M = glm::scale(M, glm::vec3(80.0f, 80.0f, 80.0f));

	glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(M));

	board->Draw(*shader);

	// Poruszanie się figur

	int index_white, index_black;
	
	if ((timer - starting_up)/1000 >= 2 * moves_counter && moves_counter <= number_of_moves)
	{
;		if (whites_move)
		{
			if (moves[moves_counter-1][0] != 9)
			{
				index_white = findIndex(moves[moves_counter - 1][0], moves[moves_counter - 1][1], white);
				index_black = findIndex(moves[moves_counter - 1][2], moves[moves_counter - 1][3], black);
				movePiece(index_white, index_black, white, black, moves[moves_counter - 1][2], moves[moves_counter - 1][3]);
			}
			else
			{
				int index_king = findIndex(castles[castles_counter][0], castles[castles_counter][1], white);
				int index_rook = findIndex(castles[castles_counter + 1][0], castles[castles_counter + 1][1], white);
				movePiece(index_king, -1, white, white, castles[castles_counter][2], castles[castles_counter][3]);
				movePiece(index_rook, -1, white, white, castles[castles_counter + 1][2], castles[castles_counter + 1][3]);
				castles_counter += 2;
			}
		} 
		else
		{
			if (moves[moves_counter - 1][0] != 9)
			{
				index_white = findIndex(moves[moves_counter - 1][2], moves[moves_counter - 1][3], white);
				index_black = findIndex(moves[moves_counter - 1][0], moves[moves_counter - 1][1], black);
				movePiece(index_black, index_white, black, white, moves[moves_counter - 1][2], moves[moves_counter - 1][3]);
			}
			else
			{
				int index_king = findIndex(castles[castles_counter][0], castles[castles_counter][1], black);
				int index_rook = findIndex(castles[castles_counter + 1][0], castles[castles_counter + 1][1], black);
				movePiece(index_king, -1, black, black, castles[castles_counter][2], castles[castles_counter][3]);
				movePiece(index_rook, -1, black, black, castles[castles_counter + 1][2], castles[castles_counter + 1][3]);
				castles_counter += 2;
			}
			
		}
		whites_move ^= true;
		moves_counter++;
	}

	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
	glDisableVertexAttribArray(shader->a("vertex"));
	glDisableVertexAttribArray(shader->a("normal"));
	glDisableVertexAttribArray(shader->a("texCoord"));
}

int main(void)
{
	int starttime = clock();
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1080, 1080, "Szachy", NULL, NULL);

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące
	createChessboard();
	//Główna pętla
	float angle_x = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu
	float angle_y = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu
	float delta_time = 0.0f;
	int number_of_moves = countMoves("games/game.txt");
	auto moves = new int[number_of_moves][4];
	fillMovesArreay(moves, "games/game.txt");

	glfwSetTime(0); //Wyzeruj licznik czasu
	starting_up = clock() - starttime;
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		delta_time = glfwGetTime();
		angle_x += speed_x * delta_time; //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		angle_y += speed_y * delta_time; //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		timer = clock();
		glfwSetTime(0);
		drawScene(window,angle_x,angle_y, moves, number_of_moves); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}
	clock_t endtime = clock();
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
