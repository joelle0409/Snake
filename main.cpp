#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include "console_renderer.h"
#include <fstream>

using namespace std;

// bool: true or false. Currently false until the keys are pressed, then it will be true.
struct TickInput
{
	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;
	bool quit = false;
	bool restart = false;
};

struct Pos { int x = 0; int y = 0; };

enum class Dir { Up, Down, Left, Right };

enum class LevelId { L1, L2, L3};

struct GameState
{
	// vector as a dynamic list, a list of Pos values for both Snake and walls, store many positions.
	// the snake is a multiple body parts, with positions.
	// same goes for the walls.

	std::vector<Pos> snake;
	std::vector<Pos> walls;

	Dir dir = Dir::Right;

	Pos food;

	int score = 0;
	bool gameOver = false;


	int width = 30;
	int height = 30;

	std::vector<std::string> level;
};

static LevelId ParseLevel(int argc, char** argv)
{
	// Parse means to read some text and interpret what it means.
	// argc is how many arguments there are, argv is the actual argument text.
	// (char& c : arg) loops through every character string of "LEVEL2", turning it into "level2". tolower.

	if (argc < 2) return LevelId::L1;

	std::string arg = argv[1];
	for (char& c : arg) c = (char)tolower((unsigned char)c);

	if (arg == "level1") return LevelId::L1;
	if (arg == "level2") return LevelId::L2;
	if (arg == "level3") return LevelId::L3;

	return LevelId::L1;
}


static bool IsWall(const GameState& game, int x, int y)
{
	// check if there is a wall on x, y
	// for every wall position inside game.walls, call it w.
	
	for (const Pos& w : game.walls)
		if (w.x == x && w.y == y) return true; // if all are equal to one another then they are the same position.
	return false;
}

static void LoadLevel(GameState& game, const std::string& filename)
{
	game.level.clear();
	game.walls.clear();

	std::ifstream file(filename);
	std::string line;

	while (std::getline(file, line))
	{
		game.level.push_back(line);
	}

	game.height = (int)game.level.size();
	game.width = (int)game.level[0].size();

	for (int y = 0; y < game.height; y++)
	{
		for (int x = 0; x < game.width; x++)
		{
			char tile = game.level[y][x];

			if (tile == '#')
				game.walls.push_back({ x, y });
		}
	}

}


static std::string GetLevelFilename(LevelId level)
{
	if (level == LevelId::L1) return "level1.txt";
	if (level == LevelId::L2) return "level2.txt";
	if (level == LevelId::L3) return "level3.txt";

	return "level1.txt";
}


static bool IsOnSnake(const GameState& game, int x, int y)
{
	for (const Pos& p : game.snake)
	{
		if (p.x == x && p.y == y) return true;
	}
	return false;
}


static void RespawnFood(GameState& game)
{
	while (true)
	{
		game.food.x = rand() % game.width;
		game.food.y = rand() % game.height;

		if (!IsOnSnake(game, game.food.x, game.food.y) && !IsWall(game, game.food.x, game.food.y))
			break;
	}
}


static Pos GetNextHead(const GameState& game)
{
	// tAKE THe current head position. the head is stored at index 0
	Pos head = game.snake[0];

	// if the snake is moving up, subtract 1 from y
	// same goes for other directions with -- or ++
	if (game.dir == Dir::Up) head.y--;
	else if (game.dir == Dir::Down) head.y++;
	else if (game.dir == Dir::Left) head.x--;
	else if (game.dir == Dir::Right) head.x++;

	// return the new calculated pos
	return head;
}



static TickInput ReadInput()
{  
	//INPUT: 

			// WASD buttons
			// Information gets stored in TickInput input; allowing the game to know what the player wants to do.
			// Tick: one cycle of the game loop, one game step.
			// The snake will move once every loop, then moves one tile per tick

	TickInput input;

	input.up = getIfBasicKeyIsCurrentlyDown('W') || getIfUpKeyIsCurrentlyDown();
	input.down = getIfBasicKeyIsCurrentlyDown('S') || getIfDownKeyIsCurrentlyDown();
	input.left = getIfBasicKeyIsCurrentlyDown('A') || getIfLeftKeyIsCurrentlyDown();
	input.right = getIfBasicKeyIsCurrentlyDown('D') || getIfRightKeyIsCurrentlyDown();
	input.quit = getIfEscKeyIsCurrentlyDown();
	input.restart = getIfBasicKeyIsCurrentlyDown('R');

	return input;

}



static void ResetGame(GameState& game, LevelId chosenLevel)
{
	game.snake.clear();
	game.snake.push_back({ 10,10 });
	game.snake.push_back({ 9,10 });
	game.snake.push_back({ 8,10 });

	game.dir = Dir::Right;
	game.score = 0;
	game.gameOver = false;

	LoadLevel(game, GetLevelFilename(chosenLevel));
	RespawnFood(game);
}



static void UpdateGame(const TickInput& input, GameState& game)
{
	// LOGIC: 

	// changes the snake direction, but preventing turining backwards.

	if (input.up && game.dir != Dir::Down) game.dir = Dir::Up;
	else if (input.down && game.dir != Dir::Up) game.dir = Dir::Down;
	else if (input.left && game.dir != Dir::Right) game.dir = Dir::Left;
	else if (input.right && game.dir != Dir::Left) game.dir = Dir::Right;

	// determines where the snake will move next. 
	Pos newHead = GetNextHead(game);

	// snake hits edge, game over. 
	if (newHead.x <= 0 || newHead.x >= game.width - 1 || newHead.y <= 0 || newHead.y >= game.height - 1)
	{
		game.gameOver = true;
	}

	// same here, game over. 
	if (IsWall(game, newHead.x, newHead.y))
	{
		game.gameOver = true;
	}

	// check if the snake had taken the food 
	bool ateFood = (newHead.x == game.food.x && newHead.y == game.food.y);

	if (!game.gameOver)
	{
		if (IsOnSnake(game, newHead.x, newHead.y))
		{
			Pos tail = game.snake.back();
			bool hitTail = (!ateFood && newHead.x == tail.x && newHead.y == tail.y);

			if (!hitTail)
				game.gameOver = true;
		}
	}

	if (!game.gameOver)
	{
		// GROW! and gain score.
		if (ateFood)
		{
			game.score++;

			game.snake.insert(game.snake.begin(), newHead);
			RespawnFood(game);
		}
		else
		{
			// add new head, remove tail, snake moves forward.
			game.snake.insert(game.snake.begin(), newHead);
			game.snake.pop_back();
		}
	}
}



static void RenderGame(const GameState& game)
{
	//RENDER AREA:

			// erase previous frame 
	clearBuffer();

	// walls
	for (int x = 0; x < game.width; x++)
	{
		drawTile(x, 0, '#');
		drawTile(x, game.height - 1, '#');
	}

	for (int y = 0; y < game.height; y++)
	{
		drawTile(0, y, '#');
		drawTile(game.width - 1, y, '#');
	}

	// snake, @ as the head, o as the body
	for (int i = 0; i < (int)game.snake.size(); i++)
	{
		char c = (i == 0) ? '@' : 'o';
		drawTile(game.snake[i].x, game.snake[i].y, c);
	}

	// draw level walls with BuildLevel
	for (const Pos& w : game.walls)
	{
		drawTile(w.x, w.y, '#');
	}

	// Food is going to be * in the game.
	// get food and score is displayed. 
	drawTile(game.food.x, game.food.y, '*');
	drawString(1, 1, (std::string("Snake - Score: ") + std::to_string(game.score)).c_str());

	if (game.gameOver)
		drawString(10, 10, "GAME OVER");

	// send everything drawn to the screen.
	renderBuffer();
}

static void ShowGameOverScreen(const GameState& game)
{
	clearBuffer();
	drawString(1, 1, (std::string("Snake - Score: ") + std::to_string(game.score)).c_str());

	// line pos
	drawString(10, 10, "GAME OVER");
	drawString(10, 12, "PRESS ESC to exit");
	drawString(10, 13, "PRESS R to restart");
	renderBuffer();
}

int main(int argc, char** argv)
{
	setupCustomConsole();
	srand((unsigned int)time(NULL));
	LevelId chosenLevel = ParseLevel(argc, argv);

	while (true)
	{
		GameState game;
		ResetGame(game, chosenLevel);

		while (!game.gameOver)
		{
			TickInput input = ReadInput();

			if (input.quit)
			{
				deleteCustomConsole();
				return 0;
			}

			UpdateGame(input, game);
			RenderGame(game);

			// controls how fast each tick runs
			int speed = 150 - game.score * 5;
			if (speed < 60) speed = 60;
			Sleep(speed);
		}

		ShowGameOverScreen(game);

		while (true)
		{
			if (getIfEscKeyIsCurrentlyDown()) 
			{ 
				deleteCustomConsole(); return 0; 
			}
			if (getIfBasicKeyIsCurrentlyDown('R')) 
				break;
			
			Sleep(10);
		}
	};
}