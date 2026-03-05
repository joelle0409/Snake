#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include "console_renderer.h"

using namespace std;


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
	std::vector<Pos> snake;
	std::vector<Pos> walls;

	Dir dir = Dir::Right;

	Pos food;

	int score = 0;
	bool gameOver = false;


	int width = 30;
	int height = 30;
};

static LevelId ParseLevel(int argc, char** argv)
{
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
	for (const Pos& w : game.walls)
		if (w.x == x && w.y == y) return true;
	return false;
}

static void BuildLevel(GameState& game, LevelId level)
{
	game.walls.clear();

	int cx = game.width / 2;
	int cy = game.height / 2;

	if (level == LevelId::L2)
	{
		for (int y = cy - 4; y <= cy + 4; y++)
			game.walls.push_back({ cx, y });
	}
	else if (level == LevelId::L3)
	{
		//forming the T
		int halfBar = 6;
		int topY = cy - 4;

		for (int x = cx - halfBar; x <= cx + halfBar; x++)
		{
			game.walls.push_back({ x, topY });
		}
		for (int y = topY; y <= cy + 6; y++)
		{
			game.walls.push_back({ cx, y });
		}
	}
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
	Pos head = game.snake[0];

	if (game.dir == Dir::Up) head.y--;
	else if (game.dir == Dir::Down) head.y++;
	else if (game.dir == Dir::Left) head.x--;
	else if (game.dir == Dir::Right) head.x++;

	return head;
}


int main(int argc, char** argv)
{
	setupCustomConsole();
	srand((unsigned int)time(NULL));
	LevelId chosenLevel = ParseLevel(argc, argv);

	if (chosenLevel == LevelId::L1)
		std::cout << "Level 1 loaded" << std::endl;
	else if (chosenLevel == LevelId::L2)
		std::cout << "Level 2 loaded" << std::endl;
	else if (chosenLevel == LevelId::L3)
		std::cout << "Level 3 loaded" << std::endl;

	while (true)
	{

		TickInput input;
		GameState game;

		game.snake.clear();
		game.snake.push_back({ 10,10 });
		game.snake.push_back({ 9,10 });
		game.snake.push_back({ 8,10 });

		game.dir = Dir::Right;
		game.score = 0;
		game.gameOver = false;

		BuildLevel(game, chosenLevel);
		RespawnFood(game);

		while (!game.gameOver)
		{
			input.up = getIfBasicKeyIsCurrentlyDown('W') || getIfUpKeyIsCurrentlyDown();
			input.down = getIfBasicKeyIsCurrentlyDown('S') || getIfDownKeyIsCurrentlyDown();
			input.left = getIfBasicKeyIsCurrentlyDown('A') || getIfLeftKeyIsCurrentlyDown();
			input.right = getIfBasicKeyIsCurrentlyDown('D') || getIfRightKeyIsCurrentlyDown();
			input.quit = getIfEscKeyIsCurrentlyDown();
			input.restart = getIfBasicKeyIsCurrentlyDown('R');

			if (input.quit)
				break;

			//DIRECTION:
			if (input.up && game.dir != Dir::Down) game.dir = Dir::Up;
			else if (input.down && game.dir != Dir::Up) game.dir = Dir::Down;
			else if (input.left && game.dir != Dir::Right) game.dir = Dir::Left;
			else if (input.right && game.dir != Dir::Left) game.dir = Dir::Right;

			Pos newHead = GetNextHead(game);

			if (newHead.x <= 0 || newHead.x >= game.width - 1 || newHead.y <= 0 || newHead.y >= game.height - 1)
			{
				game.gameOver = true;
			}

			if (IsWall(game, newHead.x, newHead.y))
			{
				game.gameOver = true;
			}

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
				if (ateFood)
				{
					game.score++;

					game.snake.insert(game.snake.begin(), newHead);
					RespawnFood(game);
				}
				else
				{
					game.snake.insert(game.snake.begin(), newHead);
					game.snake.pop_back();
				}
			}


			//RENDER AREA:
			clearBuffer();

			//walls
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

			for (int i = 0; i < (int)game.snake.size(); i++)
			{
				char c = (i == 0) ? '@' : 'o';
				drawTile(game.snake[i].x, game.snake[i].y, c);
			}

			for (const Pos& w : game.walls)
			{
				drawTile(w.x, w.y, '#');
			}

			drawTile(game.food.x, game.food.y, '*');
			drawString(1, 1, (std::string("Snake - Score: ") + std::to_string(game.score)).c_str());

			if (game.gameOver)
				drawString(10, 10, "GAME OVER");

			renderBuffer();

			int speed = 150 - game.score * 5;
			if (speed < 60) speed = 60;
			Sleep(speed);
		}

		clearBuffer();
		drawString(1, 1, (std::string("Snake - Score: ") + std::to_string(game.score)).c_str());
		drawString(10, 10, "GAME OVER");
		drawString(10, 12, "PRESS ESC to exit");
		drawString(10, 13, "PRESS R to restart");
		renderBuffer();

		while (true)
		{
			if (getIfEscKeyIsCurrentlyDown()) { deleteCustomConsole(); return 0; }
			if (getIfBasicKeyIsCurrentlyDown('R')) break;
			Sleep(10);
		}
	}

	deleteCustomConsole();
	return 0;
}