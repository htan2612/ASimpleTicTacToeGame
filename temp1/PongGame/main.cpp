#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include "Menu.h"
#include "Graphic.h"
#include "Gameplay.h"
#include "Settings.h"
#include "About.h"
#include "AI.h"

using namespace std;

int main()
{
    // Tạo cửa sổ game
    sf::RenderWindow window(sf::VideoMode({ 1600,900 }), "Caro Game", sf::Style::Default, sf::State::Windowed);
    sf::Vector2u size = window.getSize();
    float Width = size.x;
    float Height = size.y;
    window.setFramerateLimit(60);

    // ===== Load Resources =====
    sf::Texture bgTexture;
    sf::Font font;
    if (!LoadGraphicResources(bgTexture, font)) {
        return EXIT_FAILURE;
    }

    // Load avatar texture (optional - for About screen)
    sf::Texture avatarTexture;
    bool hasAvatar = avatarTexture.loadFromFile("../assets/avatar.png");

    sf::Sprite background(bgTexture);
    sf::Vector2u texSize = bgTexture.getSize();
    float scaleX = static_cast<float>(Width) / texSize.x;
    float scaleY = static_cast<float>(Height) / texSize.y;
    background.setScale({ scaleX, scaleY });

    // ===== Âm thanh =====
    sf::SoundBuffer move;
    if (!move.loadFromFile("../assets/newmove.wav"))
    {
        cerr << "Không thể load file newmove.wav\n";
        return EXIT_FAILURE;
    }
    sf::Sound sfx(move);

    sf::Music music;
    if (!music.openFromFile("../assets/chillmusic.mp3"))
    {
        cerr << "Không thể mở chillmusic.mp3\n";
        return EXIT_FAILURE;
    }
    music.setLooping(true);
    music.play();

    // ===== Game State =====
    GameState state = GameState::MENU;
    GameMode gameMode = GameMode::PVP;
    bool soundOn = true;
    sf::Clock clock;
    sf::Clock aiClock; // For AI delay
    string playerName = "";
    bool waitingForPlayerName = false;

    // ===== Menu setup =====
    vector<string> menuItems = { "PVP", "PVE", "Load Game", "Settings","ABOUT", "Exit" };
    vector<sf::Text> texts;
    vector<sf::RectangleShape> boxes;
    int selected = 0;

    InitMenu(texts, boxes, menuItems, font, Width, Height, selected);

    sf::Text title(font);
    title.setString("CARO GAME");
    title.setCharacterSize(Height / 10);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({ titleBounds.size.x / 2, titleBounds.size.y / 2 });
    title.setPosition({ Width / 2.f, Height / 5.f });
    title.setFillColor(sf::Color::Yellow);
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(6);

    // ===== Caro Game Setup =====
    vector<vector<Cell>> board(BOARD_SIZE, vector<Cell>(BOARD_SIZE));
    bool turn = true; // true = X, false = O
    bool gameOver = false;
    string winner = "";
    string playerX = "Player X";
    string playerO = "Player O";

    // Tính toán vị trí bàn cờ ở giữa màn hình
    float CELL_SIZE = 50.f;
    float boardWidth = BOARD_SIZE * CELL_SIZE;
    float boardHeight = BOARD_SIZE * CELL_SIZE;
    float offsetX = (Width - boardWidth) / 2;
    float offsetY = (Height - boardHeight) / 2;

    // Cursor highlight
    int cursorRow = 0, cursorCol = 0;
    sf::RectangleShape cursor;
    cursor.setSize({ CELL_SIZE - 4.f, CELL_SIZE - 4.f });
    cursor.setFillColor(sf::Color::Transparent);
    cursor.setOutlineThickness(3);
    cursor.setOutlineColor(sf::Color::Yellow);

    ResetBoard(board, cursorRow, cursorCol, turn, gameOver, winner, offsetX, offsetY, CELL_SIZE);

    // ===== Game Loop =====
    while (window.isOpen())
    {
        float deltatime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    if (state == GameState::MENU) {
                        window.close();
                    }
                    else {
                        state = GameState::MENU;
                    }
                }

                if (state == GameState::MENU) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::W) {
                        selected = (selected - 1 + texts.size()) % texts.size();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
                        selected = (selected + 1) % texts.size();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                        if (soundOn) sfx.play();
                        string chosen = menuItems[selected];
                        if (chosen == "Player vs Player") {
                            gameMode = GameMode::PVP;
                            state = GameState::INPUT_NAME;
                            playerName = "";
                        }
                        else if (chosen == "Player vs Computer") {
                            gameMode = GameMode::PVC;
                            state = GameState::INPUT_NAME;
                            playerName = "";
                        }
                        else if (chosen == "Load Game") {
                            if (LoadGame(board, turn, "save.caro", gameOver, winner)) {
                                state = GameState::PLAYING;
                            }
                        }
                        else if (chosen == "Exit") {
                            window.close();
                        }
                        else if (chosen == "Settings") state = GameState::SETTINGS;
                        else if (chosen == "ABOUT") state = GameState::ABOUT;
                    }

                    UpdateMenuSelection(texts, boxes, selected);
                }
                else if (state == GameState::INPUT_NAME) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Enter && !playerName.empty()) {
                        state = GameState::PLAYING;
                        playerX = playerName;
                        if (gameMode == GameMode::PVP) {
                            playerO = "Player 2";
                        }
                        else {
                            playerO = "Computer";
                        }
                        ResetBoard(board, cursorRow, cursorCol, turn, gameOver, winner, offsetX, offsetY, CELL_SIZE);
                        aiClock.restart();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Backspace && !playerName.empty()) {
                        playerName.pop_back();
                    }
                }
                else if (state == GameState::PLAYING && !gameOver) {
                    // Di chuyển cursor
                    if (keyPressed->scancode == sf::Keyboard::Scancode::W) {
                        if (cursorRow > 0) cursorRow--;
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
                        if (cursorRow < BOARD_SIZE - 1) cursorRow++;
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::A) {
                        if (cursorCol > 0) cursorCol--;
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::D) {
                        if (cursorCol < BOARD_SIZE - 1) cursorCol++;
                    }
                    // Đánh cờ
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                        // Only allow player move in PVP or when it's player's turn in PVC
                        if (gameMode == GameMode::PVP || (gameMode == GameMode::PVC && turn)) {
                            if (board[cursorRow][cursorCol].c == 0) {
                                board[cursorRow][cursorCol].c = (turn ? -1 : 1);

                                if (soundOn) sfx.play();

                                // Kiểm tra thắng
                                if (CheckWin(board, cursorRow, cursorCol)) {
                                    gameOver = true;
                                    winner = (turn ? playerX + " wins!" : playerO + " wins!");
                                    // Save match result
                                    SaveMatchResult(playerName, winner, gameMode, playerO);
                                }

                                turn = !turn;
                                aiClock.restart(); // Reset AI timer
                            }
                        }
                    }
                    // Save game
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::L) {
                        SaveGame(board, turn, "save.caro");
                    }
                    // Load game
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::T) {
                        LoadGame(board, turn, "save.caro", gameOver, winner);
                    }
                }
                else if (state == GameState::SETTINGS) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                        soundOn = !soundOn;
                        if (soundOn) {
                            music.play();
                        }
                        else {
                            music.pause();
                        }
                    }
                }
            }
            else if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
            {
                if (state == GameState::INPUT_NAME) {
                    if (textEntered->unicode < 128 && textEntered->unicode != 8 && textEntered->unicode != 13) {
                        char c = static_cast<char>(textEntered->unicode);
                        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                            (c >= '0' && c <= '9') || c == ' ' || c == '_') {
                            if (playerName.length() < 20) {
                                playerName += c;
                            }
                        }
                    }
                }
            }
        }

        // AI Move logic (for PVC mode)
        if (state == GameState::PLAYING && !gameOver && gameMode == GameMode::PVC && !turn) {
            // Add delay for AI move (0.5 seconds)
            if (aiClock.getElapsedTime().asSeconds() > 0.5f) {
                int aiRow, aiCol;
                AIMove(board, aiRow, aiCol);

                if (aiRow >= 0 && aiCol >= 0 && board[aiRow][aiCol].c == 0) {
                    board[aiRow][aiCol].c = 1; // O for computer

                    if (soundOn) sfx.play();

                    // Check win
                    if (CheckWin(board, aiRow, aiCol)) {
                        gameOver = true;
                        winner = playerO + " wins!";
                        SaveMatchResult(playerName, winner, gameMode, playerO);
                    }

                    turn = !turn;
                    aiClock.restart();
                }
            }
        }

        // Cập nhật vị trí cursor
        cursor.setPosition({ offsetX + cursorCol * CELL_SIZE, offsetY + cursorRow * CELL_SIZE });

        // ===== Render =====
        window.clear();
        window.draw(background);

        if (state == GameState::MENU) {
            RenderMenu(window, title, texts, boxes);
        }
        else if (state == GameState::INPUT_NAME) {
            RenderInputName(window, font, playerName, Width, Height);
        }
        else if (state == GameState::PLAYING) {
            RenderGameplay(window, font, board, cursor, gameOver, winner, turn, playerX, playerO, offsetX, offsetY, CELL_SIZE, Width, gameMode);
        }
        else if (state == GameState::SETTINGS) {
            RenderSettings(window, font, soundOn, Width, Height);
        }
        else {
            RenderAbout(window, font, hasAvatar ? &avatarTexture : nullptr);
        }

        window.display();
    }

    return 0;
}