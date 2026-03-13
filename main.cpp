#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include "console_renderer.h"
#include <fstream>
#include "LevelId.h"
#include "highscore.h"

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

enum class Dir 
{ 
	Up, 
	Down, 
	Left, 
	Right 
};

enum class MenuChoice
{
	Level1,
	Level2,
	Level3,
	HighScores,
	Settings,
	Quit
};

struct GameState
{
	// stores the whole game, snake body, walls, dir, food, score, game over, level size, loaded level data

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
	// reads map from text file line by line
	// sets game.wdth and game.height
	// converts # tile int positions stored in gamel.walls

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
	// converts level enums into a text file

	if (level == LevelId::L1) return "level1.txt";
	if (level == LevelId::L2) return "level2.txt";
	if (level == LevelId::L3) return "level3.txt";

	return "level1.txt";
}


static bool IsOnSnake(const GameState& game, int x, int y)
{
	// used for self collision detection

	for (const Pos& p : game.snake)
	{
		if (p.x == x && p.y == y) return true;
	}
	return false;
}


static void RespawnFood(GameState& game)
{
	// picking random coordinates for the food until it finds a valid spot where there is no snake or wall
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

			// WASD buttons, arrow keys, ESC, R
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
	// resets snake to startinf size and position, resets score and game over state, loads the selected level file, spawns food.

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
	bool ateFood = (newHead.x == game.food.x && newHead.y == game.food.y); // if both match, then ateFood = true

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

	if (!game.gameOver) // continues when game is not over
	{
		// GROW! and gain score.
		if (ateFood)
		{
			game.score++;

			game.snake.insert(game.snake.begin(), newHead); // add new head at the front snake vector
			RespawnFood(game);
		}
		else
		{
			// add new head, remove tail, snake moves forward, remove head at the back. stays same length
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

	// walls, top and bottom walls
	for (int x = 0; x < game.width; x++)
	{
		drawTile(x, 0, '#');
		drawTile(x, game.height - 1, '#');
	}

	// walls, side walls
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
	drawString(1, 15, (std::string("Snake - Score: ") + std::to_string(game.score)).c_str());

	if (game.gameOver)
		drawString(10, 10, "GAME OVER");

	// send everything drawn to the screen.
	renderBuffer();
}

static void ShowGameOverScreen(const GameState& game, int highScore)
{
	// Positioned strings with drawString

	clearBuffer();
	drawString(10, 16, (std::string("Snake - Score: ") + std::to_string(game.score)).c_str());
	drawString(10, 17, (std::string("High score: ") + std::to_string(highScore)).c_str());

	// line pos
	drawString(10, 10, "GAME OVER");
	drawString(10, 12, "PRESS R to restart");
	drawString(10, 13, "PRESS M to Menu");
	drawString(10, 14, "PRESS ESC to exit");
	renderBuffer();
}

static MenuChoice ShowMainMenu(bool fastMode)
{
	// Positioned strings with drawString
	// Player presses a chosen key

	while (true)
	{
		clearBuffer();

		drawString(10, 2, "SNAKE GAME");
		drawString(6, 5, "1 - Play Level 1");
		drawString(6, 6, "2 - Play Level 2");
		drawString(6, 7, "3 - Play Level 3");
		drawString(6, 9, "H - Show HighScore");
		drawString(6, 10, "S - settings");
		drawString(6, 11, "ESC - Quit");

		drawString(6, 14, fastMode ? "Speed mode: FAST" : "Speed mode: NORMAL");

		renderBuffer();

		if (getIfBasicKeyIsCurrentlyDown('1')) return MenuChoice::Level1;
		if (getIfBasicKeyIsCurrentlyDown('2')) return MenuChoice::Level2;
		if (getIfBasicKeyIsCurrentlyDown('3')) return MenuChoice::Level3;
		if (getIfBasicKeyIsCurrentlyDown('H')) return MenuChoice::HighScores;
		if (getIfBasicKeyIsCurrentlyDown('S')) return MenuChoice::Settings;
		if (getIfEscKeyIsCurrentlyDown()) return MenuChoice::Quit;

		Sleep(10);
	}
}


static void ShowHighScoreScreen()
{
	// loads and displays the saved highscore for all three levels. it stays open until player presses B

	int hs1 = LoadHighScore(GetHighScoreFilename(LevelId::L1));
	int hs2 = LoadHighScore(GetHighScoreFilename(LevelId::L2));
	int hs3 = LoadHighScore(GetHighScoreFilename(LevelId::L3));

	while (true)
	{
		clearBuffer();

		drawString(10, 2, "HIGH SCORES");
		drawString(6, 5, (std::string("Level 1: ") + std::to_string(hs1)).c_str());
		drawString(6, 6, (std::string("Level 2: ") + std::to_string(hs2)).c_str());
		drawString(6, 7, (std::string("Level 3: ") + std::to_string(hs3)).c_str());

		drawString(6, 10, "Press B to go back");

		renderBuffer();

		if (getIfBasicKeyIsCurrentlyDown('B'))
			break;

		Sleep(10);
	}
}

static void ShowSettingsScreen(bool& fastMode)
{
	//Displays setting screen. toggle the speed with T. 

	while (true)
	{
		clearBuffer();

		drawString(10, 2, "SETTINGS");
		drawString(6, 5, fastMode ? "Speed mode: FAST" : "Speed mode: NORMAL");
		drawString(6, 7, "T - Toggle Speed");
		drawString(6, 8, "B - Back");

		drawString(6, 10, "Press B to go back");

		renderBuffer();

		if (getIfBasicKeyIsCurrentlyDown('T'))
		{
			fastMode = !fastMode;
			Sleep(150);
		}
		if (getIfBasicKeyIsCurrentlyDown('B'))
			break;

		Sleep(10);
	}
}




int main(int argc, char** argv)
{
	// controls the whole program

	setupCustomConsole();
	srand((unsigned int)time(NULL));

	bool fastMode = false;
	LevelId chosenLevel = ParseLevel(argc, argv);

	while (true) // outergame loop, keeps the whole program alive: menu, play game, return menu, play again, quit
	{
		MenuChoice menuChoice = ShowMainMenu(fastMode); // wait until player chooses

		if (menuChoice == MenuChoice::Quit)
		{
			deleteCustomConsole();
			return 0;
		}

		if (menuChoice == MenuChoice::HighScores)
		{
			ShowHighScoreScreen();
			continue;
		}
		
		if (menuChoice == MenuChoice::Settings)
		{
			ShowSettingsScreen(fastMode);
			continue;
		}

		if (menuChoice == MenuChoice::Level1) chosenLevel = LevelId::L1;
		if (menuChoice == MenuChoice::Level2) chosenLevel = LevelId::L2;
		if (menuChoice == MenuChoice::Level3) chosenLevel = LevelId::L3;


		while (true) // inner game loop, controls one selected level session. start lvl, die, rsrt lvl, go back to main menu
		{
			GameState* game = new GameState;
			ResetGame(*game, chosenLevel);

			while (!game->gameOver) // loop runs while snake is still alive. each pass through this loop is one game tick
			{
				TickInput input = ReadInput();

				if (input.quit)
				{
					delete game;
					deleteCustomConsole();
					return 0;
				}

				UpdateGame(input, *game);
				RenderGame(*game);

				// controls how fast each tick runs, game is faster when score increases  
				int basedSpeed = fastMode ? 100 : 150;
				int speed = 150 - game->score * 5;
				if (speed < 60) speed = 60;

				Sleep(speed);
			}

			// get correct hs file for chosenlevel, load hs, assume final hs is the old one first
			std::string highscoreFile = GetHighScoreFilename(chosenLevel);
			int oldHighScore = LoadHighScore(highscoreFile);
			int finalHighScore = oldHighScore;

			if (game->score > oldHighScore)
			{
				SaveHighScore(highscoreFile, game->score); // save new score, update final hs
				finalHighScore = game->score;

			}

			ShowGameOverScreen(*game, finalHighScore); // score, hs, restart option, menu option, exit option

			bool goToMenu = false;

			while (true) 
			{
				if (getIfEscKeyIsCurrentlyDown())
				{
					delete game;
					deleteCustomConsole();
					return 0;
				}

				if (getIfBasicKeyIsCurrentlyDown('M'))
				{
					delete game;
					goToMenu = true;
					break;
				}

				if (getIfBasicKeyIsCurrentlyDown('R'))
				{
					delete game;
					break;
				}

				Sleep(10);
			}

			if (goToMenu)
				break;
		}
	}
}