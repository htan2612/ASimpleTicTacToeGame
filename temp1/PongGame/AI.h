#ifndef AI_H
#define AI_H

#include <vector>

// Forward declaration
struct Cell;

// AI Move function
void AIMove(std::vector<std::vector<Cell>>& board, int& aiRow, int& aiCol);

// Evaluate position for AI
int EvaluatePosition(const std::vector<std::vector<Cell>>& board, int row, int col, int player);

// Check win function (from Gameplay)
bool CheckWin(const std::vector<std::vector<Cell>>& board, int i, int j);

// Board size constant
extern const int BOARD_SIZE;

#endif