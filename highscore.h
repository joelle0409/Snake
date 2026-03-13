#pragma once
#include <string>
#include "LevelId.h"

int LoadHighScore(const std::string& filename); // reads saved integer score from a file and returns it. if file does not open, return 0
void SaveHighScore(const std::string& filename, int score); // writes score into the chosen hs file, replacing old contents
std::string GetHighScoreFilename(LevelId level); // converts enum level into the matching hs filename such as the highscore_level1.txt
