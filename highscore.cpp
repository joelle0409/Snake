#include "highscore.h"
#include <fstream>
#include <string>

int LoadHighScore(const std::string& filename)
{
	// reads highscore from file

	std::ifstream file(filename); // reading from file

	int score = 0;

	if (file.is_open()) // checks if the file was opened
		file >> score;  // reads the number from file into variable. if the file says 17 then: score = 17.

	return score;
}

void SaveHighScore(const std::string& filename, int score)
{
	// writes highscore to file

	std::ofstream file(filename); // writes to file

	if (file.is_open())
		file << score; // if the score = 17, then the file becomes 17.
}


std::string GetHighScoreFilename(LevelId level)
{
	if (level == LevelId::L1) return "highscore_level1.txt";
	if (level == LevelId::L2) return "highscore_level2.txt";
	if (level == LevelId::L3) return "highscore_level3.txt";

	return "highscore_level1.txt";
}

