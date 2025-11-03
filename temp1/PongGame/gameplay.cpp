#include "Gameplay.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cmath>
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

bool CheckWin(const vector<vector<Cell>>& board, int i, int j, WinningLine& winLine) {
    int c = board[i][j].c;
    if (c == 0) return false;

    int dx[] = { 0, 1, 1, 1 };
    int dy[] = { 1, 0, 1, -1 };

    winLine.exists = false;
    winLine.cells.clear();

    for (int dir = 0; dir < 4; ++dir) {
        vector<pair<int, int>> line;
        line.push_back({ i, j });

        // Tìm về phía trước
        for (int k = 1; k < 5; ++k) {
            int ni = i + dx[dir] * k;
            int nj = j + dy[dir] * k;
            if (ni < 0 || nj < 0 || ni >= BOARD_SIZE || nj >= BOARD_SIZE) break;
            if (board[ni][nj].c == c) line.push_back({ ni, nj });
            else break;
        }

        // Tìm về phía sau
        for (int k = 1; k < 5; ++k) {
            int ni = i - dx[dir] * k;
            int nj = j - dy[dir] * k;
            if (ni < 0 || nj < 0 || ni >= BOARD_SIZE || nj >= BOARD_SIZE) break;
            if (board[ni][nj].c == c) line.push_back({ ni, nj });
            else break;
        }

        if (line.size() >= 5) {
            winLine.exists = true;
            winLine.cells = line;
            return true;
        }
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
    GameMode mode, const WinningLine& winLine, float animTime) {

    // Vẽ bàn cờ
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            window.draw(board[i][j].shape);

            // Kiểm tra xem ô này có trong dòng thắng không
            bool isWinCell = false;
            if (winLine.exists) {
                for (const auto& cell : winLine.cells) {
                    if (cell.first == i && cell.second == j) {
                        isWinCell = true;
                        break;
                    }
                }
            }

            if (board[i][j].c == -1) {
                sf::Text mark(font);
                mark.setString("X");
                mark.setCharacterSize(CELL_SIZE - 15);

                // Hiệu ứng cho dòng thắng
                if (isWinCell && gameOver) {
                    float pulse = (sin(animTime * 4.0f) + 1.0f) / 2.0f;
                    float scale = 1.0f + pulse * 0.3f;
                    mark.setScale({ scale, scale });

                    // Màu sáng lên theo pulse
                    std::uint8_t brightness = 200 + static_cast<std::uint8_t>(pulse * 55);
                    mark.setFillColor(sf::Color(255, brightness / 2, brightness / 2));
                    mark.setOutlineThickness(2);
                    mark.setOutlineColor(sf::Color::Yellow);
                }
                else {
                    mark.setFillColor(sf::Color::Red);
                }

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

                // Hiệu ứng cho dòng thắng
                if (isWinCell && gameOver) {
                    float pulse = (sin(animTime * 4.0f) + 1.0f) / 2.0f;
                    float scale = 1.0f + pulse * 0.3f;
                    mark.setScale({ scale, scale });

                    // Màu sáng lên theo pulse
                    std::uint8_t brightness = 200 + static_cast<std::uint8_t>(pulse * 55);
                    mark.setFillColor(sf::Color(brightness / 2, brightness / 2, 255));
                    mark.setOutlineThickness(2);
                    mark.setOutlineColor(sf::Color::Yellow);
                }
                else {
                    mark.setFillColor(sf::Color::Blue);
                }

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

    // Hiển thị tên người chơi ở 2 bên
    float boardWidth = BOARD_SIZE * CELL_SIZE;
    float leftPanelX = offsetX - 200;
    float rightPanelX = offsetX + boardWidth + 50;
    float panelY = offsetY + 50;

    // Panel bên trái - Player X
    sf::RectangleShape leftPanel({ 180, 250 });
    leftPanel.setPosition({ leftPanelX, panelY });
    leftPanel.setFillColor(sf::Color(30, 30, 50, 200));
    leftPanel.setOutlineThickness(3);

    if (gameOver && winner.find(playerX) != string::npos) {
        leftPanel.setOutlineColor(sf::Color::Green);
    }
    else if (gameOver) {
        leftPanel.setOutlineColor(sf::Color(150, 150, 150));
    }
    else if (turn) {
        leftPanel.setOutlineColor(sf::Color::Yellow);
    }
    else {
        leftPanel.setOutlineColor(sf::Color::White);
    }
    window.draw(leftPanel);

    // Avatar X
    sf::CircleShape avatarX(40);
    avatarX.setPosition({ leftPanelX + 50, panelY + 20 });
    avatarX.setFillColor(sf::Color(200, 50, 50));
    avatarX.setOutlineThickness(3);
    avatarX.setOutlineColor(sf::Color::Red);
    window.draw(avatarX);

    sf::Text xMark(font);
    xMark.setString("X");
    xMark.setCharacterSize(50);
    xMark.setFillColor(sf::Color::White);
    sf::FloatRect xBounds = xMark.getLocalBounds();
    xMark.setOrigin({ xBounds.size.x / 2, xBounds.size.y / 2 });
    xMark.setPosition({ leftPanelX + 90, panelY + 60 });
    window.draw(xMark);

    // Tên Player X
    sf::Text nameX(font);
    nameX.setString(playerX);
    nameX.setCharacterSize(20);
    nameX.setFillColor(sf::Color::White);
    nameX.setPosition({ leftPanelX + 10, panelY + 120 });
    window.draw(nameX);

    // Status Player X
    sf::Text statusX(font);
    if (gameOver) {
        if (winner.find(playerX) != string::npos) {
            statusX.setString("WINNER!");
            statusX.setFillColor(sf::Color::Green);
        }
        else {
            statusX.setString("LOSER");
            statusX.setFillColor(sf::Color(150, 150, 150));
        }
    }
    else if (turn) {
        statusX.setString("Your Turn");
        statusX.setFillColor(sf::Color::Yellow);
    }
    else {
        statusX.setString("Waiting...");
        statusX.setFillColor(sf::Color(200, 200, 200));
    }
    statusX.setCharacterSize(18);
    statusX.setPosition({ leftPanelX + 10, panelY + 150 });
    window.draw(statusX);

    // Panel bên phải - Player O
    sf::RectangleShape rightPanel({ 180, 250 });
    rightPanel.setPosition({ rightPanelX, panelY });
    rightPanel.setFillColor(sf::Color(30, 30, 50, 200));
    rightPanel.setOutlineThickness(3);

    if (gameOver && winner.find(playerO) != string::npos) {
        rightPanel.setOutlineColor(sf::Color::Green);
    }
    else if (gameOver) {
        rightPanel.setOutlineColor(sf::Color(150, 150, 150));
    }
    else if (!turn) {
        rightPanel.setOutlineColor(sf::Color::Yellow);
    }
    else {
        rightPanel.setOutlineColor(sf::Color::White);
    }
    window.draw(rightPanel);

    // Avatar O
    sf::CircleShape avatarO(40);
    avatarO.setPosition({ rightPanelX + 50, panelY + 20 });
    avatarO.setFillColor(sf::Color(50, 50, 200));
    avatarO.setOutlineThickness(3);
    avatarO.setOutlineColor(sf::Color::Blue);
    window.draw(avatarO);

    sf::Text oMark(font);
    oMark.setString("O");
    oMark.setCharacterSize(50);
    oMark.setFillColor(sf::Color::White);
    sf::FloatRect oBounds = oMark.getLocalBounds();
    oMark.setOrigin({ oBounds.size.x / 2, oBounds.size.y / 2 });
    oMark.setPosition({ rightPanelX + 90, panelY + 60 });
    window.draw(oMark);

    // Tên Player O
    sf::Text nameO(font);
    nameO.setString(playerO);
    nameO.setCharacterSize(20);
    nameO.setFillColor(sf::Color::White);
    nameO.setPosition({ rightPanelX + 10, panelY + 120 });
    window.draw(nameO);

    // Status Player O
    sf::Text statusO(font);
    if (gameOver) {
        if (winner.find(playerO) != string::npos) {
            statusO.setString("WINNER!");
            statusO.setFillColor(sf::Color::Green);
        }
        else {
            statusO.setString("LOSER");
            statusO.setFillColor(sf::Color(150, 150, 150));
        }
    }
    else if (!turn) {
        statusO.setString("Your Turn");
        statusO.setFillColor(sf::Color::Yellow);
    }
    else {
        statusO.setString("Waiting...");
        statusO.setFillColor(sf::Color(200, 200, 200));
    }
    statusO.setCharacterSize(18);
    statusO.setPosition({ rightPanelX + 10, panelY + 150 });
    window.draw(statusO);

    // Instructions
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

void RenderGameOverMenu(sf::RenderWindow& window, sf::Font& font,
    const vector<sf::Text>& texts,
    const vector<sf::RectangleShape>& boxes,
    const string& winner, float Width, float Height) {

    // Semi-transparent overlay
    sf::RectangleShape overlay({ Width, Height });
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    // Winner text with glow effect
    sf::Text winnerText(font);
    winnerText.setString(winner);
    winnerText.setCharacterSize(70);
    sf::FloatRect winBounds = winnerText.getLocalBounds();
    winnerText.setOrigin({ winBounds.size.x / 2, winBounds.size.y / 2 });
    winnerText.setPosition({ Width / 2, Height / 4 });
    winnerText.setFillColor(sf::Color::Yellow);
    winnerText.setOutlineThickness(5);
    winnerText.setOutlineColor(sf::Color(255, 165, 0)); // Orange glow
    window.draw(winnerText);

    // Trophy/celebration emoji
    sf::Text trophy(font);
    trophy.setString("* * *");
    trophy.setCharacterSize(50);
    sf::FloatRect trophyBounds = trophy.getLocalBounds();
    trophy.setOrigin({ trophyBounds.size.x / 2, trophyBounds.size.y / 2 });
    trophy.setPosition({ Width / 2, Height / 4 - 80 });
    trophy.setFillColor(sf::Color(255, 215, 0)); // Gold
    window.draw(trophy);

    // Draw menu items
    for (size_t i = 0; i < boxes.size(); i++) {
        window.draw(boxes[i]);
        window.draw(texts[i]);
    }
}