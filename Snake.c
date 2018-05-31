#include <windows.h>
#include <stdio.h>

#define ROWS 20
#define COLUMNS 30
#define MAX_WIDTH ROWS*COLUMNS

#define INITIAL_POSITION_X 10
#define INITIAL_POSITION_Y 3

typedef enum { BORDER_VERTICAL = 'x', BORDER_HORIZONTAL = '+', BLANK = ' ', SNAKE = '#', SNAKE_FULL = '@', FOOD = '*', GAME_OVER = 'X' } Caracter;
enum Direction { TO_TOP, TO_RIGHT, TO_BOTTOM, TO_LEFT };

typedef enum { false, true } bool;

typedef struct {
	int y;
	int x;
}Coordenate;

typedef struct {
	Coordenate coordenates[MAX_WIDTH];
	int width;
}Snake;

struct {
	Coordenate coordenates[MAX_WIDTH];
	int width;
} foods;

HHOOK hHock;
DWORD cCharsWritten = 0;

bool hasFood = false;
int oldDirection;
int currentDirection;
int currentPositionX;
int currentPositionY;
char gameField[ROWS][COLUMNS];
int speed = 1000;
Snake snake;

void updateGameField(int y, int x, Caracter c) 
{
	gameField[y][x] = c;
	COORD here = (COORD) { x, y };
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleOutputCharacter(hStdOut, &c, 1, here, &cCharsWritten);
}

void printGameField() 
{
	int i, j;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLUMNS; j++)
		{
			printf("%c", gameField[i][j]);
		}
		printf("\n");
	}
}

void gameOver() 
{
	updateGameField(currentPositionY, currentPositionX, GAME_OVER);
	printf("\nGAME OVER!!!\n\nYour score was %d\n\n", snake.width - 1);
	exit(0);
}

void generateFood()
{
	int randomRow;
	int randomColumn;
	do {
		randomRow = rand() % ROWS;
		randomColumn = rand() % COLUMNS;
	} while (gameField[randomRow][randomColumn] != BLANK);
	updateGameField(randomRow, randomColumn, FOOD);
	hasFood = true;
}

void fillGameField() 
{
	int i, j;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLUMNS; j++)
		{
			if (j < 2 || j > COLUMNS - 3) {
				gameField[i][j] = BORDER_VERTICAL;
				continue;
			}
			if (i < 2 || i > ROWS - 3) {
				gameField[i][j] = BORDER_HORIZONTAL;
				continue;
			}
			gameField[i][j] = BLANK;
		}
	}
}

void moveSnake()
{

	switch (currentDirection)
	{
	case TO_TOP:
		currentPositionY--;
		break;
	case TO_RIGHT:
		currentPositionX++;
		break;
	case TO_BOTTOM:
		currentPositionY++;
		break;
	case TO_LEFT:
		currentPositionX--;
		break;
	default:
		break;
	}

	switch (gameField[currentPositionY][currentPositionX])
	{
	case BORDER_VERTICAL:
		currentPositionX = (currentPositionX <= 1) ? COLUMNS - 3 : 2;
		break;
	case BORDER_HORIZONTAL:
		currentPositionY = (currentPositionY <= 1) ? ROWS - 3 : 2;
		break;
	default:
		break;
	}

	updateGameField(snake.coordenates[snake.width - 1].y, snake.coordenates[snake.width - 1].x, BLANK);
	memmove(&snake.coordenates[1], snake.coordenates, sizeof(Coordenate)*(snake.width - 1));
	snake.coordenates[0] = (Coordenate) { currentPositionY, currentPositionX };

	if (gameField[foods.coordenates[foods.width - 1].y][foods.coordenates[foods.width - 1].x] == BLANK) {
		updateGameField(foods.coordenates[foods.width - 1].y, foods.coordenates[foods.width - 1].x, SNAKE);
		snake.coordenates[snake.width++] = (Coordenate) { foods.coordenates[foods.width - 1].y, foods.coordenates[foods.width - 1].x };
		foods.width--;
	}

	switch (gameField[currentPositionY][currentPositionX])
	{
	case FOOD:
		hasFood = false;
		memmove(&foods.coordenates[1], foods.coordenates, sizeof(Coordenate)*foods.width);
		foods.coordenates[0] = (Coordenate) { currentPositionY, currentPositionX };
		foods.width++;
		updateGameField(currentPositionY, currentPositionX, SNAKE_FULL);
		generateFood();
		if (speed != 5)speed -= 5;
		break;
	case SNAKE:
	case SNAKE_FULL:
		gameOver();
		break;
	case BLANK:
		updateGameField(currentPositionY, currentPositionX, SNAKE);
		break;
	default:
		break;
	}
}

LRESULT CALLBACK MyLowLevelHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= HC_ACTION)
	{
		KBDLLHOOKSTRUCT* pkbhs = (KBDLLHOOKSTRUCT*)lParam;
		if (pkbhs->vkCode == 'W') {
			if (oldDirection != TO_BOTTOM) {
				currentDirection = TO_TOP;
			}
		}
		if (pkbhs->vkCode == 'D') {
			if (oldDirection != TO_LEFT) {
				currentDirection = TO_RIGHT;
			}
		}
		if (pkbhs->vkCode == 'S') {
			if (oldDirection != TO_TOP) {
				currentDirection = TO_BOTTOM;
			}
		}
		if (pkbhs->vkCode == 'A') {
			if (oldDirection != TO_RIGHT) {
				currentDirection = TO_LEFT;
			}
		}
	}
	return CallNextHookEx(hHock, nCode, wParam, lParam);
}

DWORD WINAPI runKeyboardListener(void* data)
{
	MSG msg;
	hHock = SetWindowsHookEx(WH_KEYBOARD_LL, MyLowLevelHook, NULL, NULL);

	while (!GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(hHock);
	return 0;
}

DWORD WINAPI runPromptViewer(void* data)
{
	currentPositionX = INITIAL_POSITION_X;
	currentPositionY = INITIAL_POSITION_Y;
	currentDirection = TO_RIGHT;

	fillGameField();
	printGameField();
	generateFood();
	snake.width = 1;
	snake.coordenates[0] = (Coordenate) { currentPositionY, currentPositionX };
	updateGameField(currentPositionY, currentPositionX, SNAKE);
	while (true)
	{
		moveSnake();
		oldDirection = currentDirection;
		Sleep(speed);
	}
	return 0;
}

int main()
{
	HANDLE promptViewer = CreateThread(NULL, 0, runPromptViewer, NULL, 0, NULL);
	HANDLE keyboardListener = CreateThread(NULL, 0, runKeyboardListener, NULL, 0, NULL);

	while (true);

	return 0;
}

