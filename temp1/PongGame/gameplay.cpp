#include "Gameplay.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstring>
using namespace std;

void ResetBoard(vector<vector<Cell>>& board, int& cursorRow, int& cursorCol,
    bool& turn, bool& gameOver, string& winner,
    float offsetX, float offsetY, float CELL_SIZE) {
    board.clear();
    board.resize(BOARD_SIZE, vector<Cell>(BOARD_SIZE));

    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j].c = 0;
            board[i][j].shape.setSize({ CELL_SIZE - 2.f, CELL_SIZE - 2.f });
            board[i][j].shape.setPosition({ offsetX + j * CELL_SIZE, offsetY + i * CELL_SIZE });
            board[i][j].shape.setFillColor(sf::Color(240, 240, 240, 50));
            board[i][j].shape.setOutlineThickness(1);
            board[i][j].shape.setOutlineColor(sf::Color(100, 100, 100));
        }
    }
    cursorRow = 0;
    cursorCol = 0;
    turn = true;
    gameOver = false;
    winner = "";
}

bool CheckWin(const vector<vector<Cell>>& board, int i, int j) {
    int c = board[i][j].c;
    if (c == 0) return false;

    int dx[] = { 0, 1, 1, 1 };
    int dy[] = { 1, 0, 1, -1 };

    for (int dir = 0; dir < 4; ++dir) {
        int count = 1;

        for (int k = 1; k < 5; ++k) {
            int ni = i + dx[dir] * k;
            int nj = j + dy[dir] * k;
            if (ni < 0 || nj < 0 || ni >= BOARD_SIZE || nj >= BOARD_SIZE) break;
            if (board[ni][nj].c == c) count++;
            else break;
        }

        for (int k = 1; k < 5; ++k) {
            int ni = i - dx[dir] * k;
            int nj = j - dy[dir] * k;
            if (ni < 0 || nj < 0 || ni >= BOARD_SIZE || nj >= BOARD_SIZE) break;
            if (board[ni][nj].c == c) count++;
            else break;
        }

        if (count >= 5) return true;
    }
    return false;
}

bool SaveGame(const vector<vector<Cell>>& board, bool turn, const string& playerName,
    GameMode mode, const char* filename) {
    ofstream f(filename, ios::binary);
    if (!f) return false;

    // Save board
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            f.write((char*)&board[i][j].c, sizeof(int));

    // Save turn
    f.write((char*)&turn, sizeof(bool));

    // Save metadata
    char metadata[256] = { 0 };
    string modeStr = (mode == GameMode::PVP) ? "PVP" : "PVC";
    string metaStr = playerName + "|" + modeStr;
    strncpy_s(metadata, 256, metaStr.c_str(), _TRUNCATE);
    f.write(metadata, 256);

    f.close();
    return true;
}

bool LoadGame(vector<vector<Cell>>& board, bool& turn, const char* filename,
    bool& gameOver, string& winner, string& playerX, string& playerO, GameMode& mode) {
    ifstream f(filename, ios::binary);
    if (!f) return false;

    board.clear();
    board.resize(BOARD_SIZE, vector<Cell>(BOARD_SIZE));

    // Load board
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            f.read((char*)&board[i][j].c, sizeof(int));

    // Load turn
    f.read((char*)&turn, sizeof(bool));

    // Load metadata
    char metadata[256] = { 0 };
    f.read(metadata, 256);

    string metaStr(metadata);
    size_t pos = metaStr.find('|');
    if (pos != string::npos) {
        playerX = metaStr.substr(0, pos);
        string modeStr = metaStr.substr(pos + 1);
        mode = (modeStr == "PVP") ? GameMode::PVP : GameMode::PVC;
        playerO = (mode == GameMode::PVP) ? "Player 2" : "Computer";
    }

    f.close();
    gameOver = false;
    winner = "";
    return true;
}

void RenderGameplay(sf::RenderWindow& window, sf::Font& font,
    const vector<vector<Cell>>& board,
    const sf::RectangleShape& cursor,
    bool gameOver, const string& winner,
    bool turn, const string& playerX, const string& playerO,
    float offsetX, float offsetY, float CELL_SIZE, float Width,
    GameMode mode) {

    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            window.draw(board[i][j].shape);

            if (board[i][j].c == -1) {
                sf::Text mark(font);
                mark.setString("X");
                mark.setCharacterSize(CELL_SIZE - 15);
                mark.setFillColor(sf::Color::Red);
                sf::FloatRect bounds = mark.getLocalBounds();
                mark.setOrigin({ bounds.size.x / 2, bounds.size.y / 2 });
                mark.setPosition({ offsetX + j * CELL_SIZE + CELL_SIZE / 2,
                                 offsetY + i * CELL_SIZE + CELL_SIZE / 2 });
                window.draw(mark);
            }
            else if (board[i][j].c == 1) {
                sf::Text mark(font);
                mark.setString("O");
                mark.setCharacterSize(CELL_SIZE - 15);
                mark.setFillColor(sf::Color::Blue);
                sf::FloatRect bounds = mark.getLocalBounds();
                mark.setOrigin({ bounds.size.x / 2, bounds.size.y / 2 });
                mark.setPosition({ offsetX + j * CELL_SIZE + CELL_SIZE / 2,
                                 offsetY + i * CELL_SIZE + CELL_SIZE / 2 });
                window.draw(mark);
            }
        }
    }

    if (!gameOver) {
        window.draw(cursor);
    }

    if (gameOver) {
        sf::Text resultText(font);
        resultText.setString(winner);
        resultText.setCharacterSize(50);
        resultText.setFillColor(sf::Color::Green);
        sf::FloatRect resBounds = resultText.getLocalBounds();
        resultText.setOrigin({ resBounds.size.x / 2, resBounds.size.y / 2 });
        resultText.setPosition({ Width / 2, 50 });
        window.draw(resultText);
    }
    else {
        sf::Text turnText(font);
        string turnStr = turn ? (playerX + "'s turn (X)") : (playerO + "'s turn (O)");
        if (mode == GameMode::PVC && !turn) {
            turnStr = "Computer's turn (O)";
        }
        turnText.setString(turnStr);
        turnText.setCharacterSize(30);
        turnText.setFillColor(sf::Color::White);
        turnText.setPosition({ 50, 50 });
        window.draw(turnText);
    }

    sf::Text instructionText(font);
    instructionText.setString("WASD: Move | Enter: Place | P: Pause | ESC: Menu");
    instructionText.setCharacterSize(20);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    sf::Vector2u size = window.getSize();
    instructionText.setPosition({ 50, (float)size.y - 40 });
    window.draw(instructionText);
}

void SaveMatchResult(const string& playerName, const string& result,
    GameMode mode, const string& opponent) {
    ofstream file("match_history.txt", ios::app);
    if (!file) return;

    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);

    stringstream ss;
    tm timeinfo;
    localtime_s(&timeinfo, &in_time_t);
    ss << put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");

    file << "====================================\n";
    file << "Date: " << ss.str() << "\n";
    file << "Player: " << playerName << "\n";
    file << "Mode: " << (mode == GameMode::PVP ? "Player vs Player" : "Player vs Computer") << "\n";
    file << "Opponent: " << opponent << "\n";
    file << "Result: " << result << "\n";
    file << "====================================\n\n";

    file.close();
}