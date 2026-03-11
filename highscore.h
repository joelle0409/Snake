#pragma once
#include <string>
#include "LevelId.h"

int LoadHighScore(const std::string& filename);
void SaveHighScore(const std::string& filename, int score);
std::string GetHighScoreFilename(LevelId level);
