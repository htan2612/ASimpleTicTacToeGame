#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include "Menu.h"

const int BOARD_SIZE = 12;

struct Cell {
    int c; // 0 = trống, -1 = X, 1 = O
    sf::RectangleShape shape;
};

void ResetBoard(std::vector<std::vector<Cell>>& board, int& cursorRow, int& cursorCol,
    bool& turn, bool& gameOver, std::string& winner,
    float offsetX, float offsetY, float CELL_SIZE);

bool CheckWin(const std::vector<std::vector<Cell>>& board, int i, int j);

bool SaveGame(const std::vector<std::vector<Cell>>& board, bool turn, const char* filename);

bool LoadGame(std::vector<std::vector<Cell>>& board, bool& turn, const char* filename,
    bool& gameOver, std::string& winner);

void RenderGameplay(sf::RenderWindow& window, sf::Font& font,
    const std::vector<std::vector<Cell>>& board,
    const sf::RectangleShape& cursor,
    bool gameOver, const std::string& winner,
    bool turn, const std::string& playerX, const std::string& playerO,
    float offsetX, float offsetY, float CELL_SIZE, float Width,
    GameMode mode);

// Save match result
void SaveMatchResult(const std::string& playerName, const std::string& result,
    GameMode mode, const std::string& opponent);

#endif