#include "AI.h"
#include "Gameplay.h"
#include <cstdlib>
#include <algorithm>
using namespace std;

// AI Move function - Simple AI with basic strategy
void AIMove(vector<vector<Cell>>& board, int& aiRow, int& aiCol) {
    int bestScore = -10000;
    aiRow = -1;
    aiCol = -1;

    // Find best move for AI (O = 1)
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].c == 0) {
                // Try this move
                int score = EvaluatePosition(board, i, j, 1);

                // Check if AI can win
                board[i][j].c = 1;
                if (CheckWin(board, i, j)) {
                    board[i][j].c = 0;
                    aiRow = i;
                    aiCol = j;
                    return; // Win immediately
                }
                board[i][j].c = 0;

                // Check if need to block player
                board[i][j].c = -1;
                if (CheckWin(board, i, j)) {
                    board[i][j].c = 0;
                    score += 5000; // High priority to block
                }
                board[i][j].c = 0;

                if (score > bestScore) {
                    bestScore = score;
                    aiRow = i;
                    aiCol = j;
                }
            }
        }
    }

    // If no good move found, pick random
    if (aiRow == -1) {
        vector<pair<int, int>> emptyCells;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j].c == 0) {
                    emptyCells.push_back({ i, j });
                }
            }
        }
        if (!emptyCells.empty()) {
            int idx = rand() % emptyCells.size();
            aiRow = emptyCells[idx].first;
            aiCol = emptyCells[idx].second;
        }
    }
}

int EvaluatePosition(const vector<vector<Cell>>& board, int row, int col, int player) {
    int score = 0;

    // Prefer center positions
    int centerDist = abs(row - BOARD_SIZE / 2) + abs(col - BOARD_SIZE / 2);
    score += (BOARD_SIZE - centerDist) * 10;

    // Check surrounding pieces
    int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

    for (int dir = 0; dir < 8; dir++) {
        int ni = row + dx[dir];
        int nj = col + dy[dir];
        if (ni >= 0 && nj >= 0 && ni < BOARD_SIZE && nj < BOARD_SIZE) {
            if (board[ni][nj].c == player) {
                score += 50; // Near friendly piece
            }
            else if (board[ni][nj].c == -player) {
                score += 30; // Near enemy piece (for blocking)
            }
        }
    }

    return score;
}