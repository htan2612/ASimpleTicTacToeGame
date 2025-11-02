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
#include "LoadGame.h"

using namespace std;

int main()
{
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

    sf::Texture avatarTexture;
    bool hasAvatar = avatarTexture.loadFromFile("../assets/avatar.png");

    sf::Sprite background(bgTexture);
    sf::Vector2u texSize = bgTexture.getSize();
    float scaleX = static_cast<float>(Width) / texSize.x;
    float scaleY = static_cast<float>(Height) / texSize.y;
    background.setScale({ scaleX, scaleY });

    // ===== Audio =====
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
    sf::Clock aiClock;
    string playerName = "";
    string player2Name = "";

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

    // ===== Pause Menu =====
    vector<string> pauseMenuItems = { "Resume", "Save & Exit", "Exit without Saving" };
    vector<sf::Text> pauseTexts;
    vector<sf::RectangleShape> pauseBoxes;
    int pauseSelected = 0;

    // ===== Load Game List =====
    vector<SavedGame> savedGames;
    int loadGameSelected = 0;

    // ===== Caro Game Setup =====
    vector<vector<Cell>> board(BOARD_SIZE, vector<Cell>(BOARD_SIZE));
    bool turn = true;
    bool gameOver = false;
    string winner = "";
    string playerX = "Player X";
    string playerO = "Player O";

    float CELL_SIZE = 50.f;
    float boardWidth = BOARD_SIZE * CELL_SIZE;
    float boardHeight = BOARD_SIZE * CELL_SIZE;
    float offsetX = (Width - boardWidth) / 2;
    float offsetY = (Height - boardHeight) / 2;

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
                    else if (state == GameState::PLAYING) {
                        state = GameState::PAUSE_MENU;
                        pauseSelected = 0;
                        pauseTexts.clear();
                        pauseBoxes.clear();
                        InitMenu(pauseTexts, pauseBoxes, pauseMenuItems, font, Width, Height, pauseSelected);
                    }
                    else if (state == GameState::PAUSE_MENU) {
                        state = GameState::PLAYING;
                    }
                    else if (state == GameState::LOAD_GAME_LIST) {
                        state = GameState::MENU;
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
                        if (chosen == "PVP") {
                            gameMode = GameMode::PVP;
                            state = GameState::INPUT_NAME;
                            playerName = "";
                        }
                        else if (chosen == "PVE") {
                            gameMode = GameMode::PVC;
                            state = GameState::INPUT_NAME;
                            playerName = "";
                        }
                        else if (chosen == "Load Game") {
                            savedGames = GetSavedGames();
                            loadGameSelected = 0;
                            state = GameState::LOAD_GAME_LIST;
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
                        if (gameMode == GameMode::PVP) {
                            state = GameState::INPUT_NAME_P2;
                            player2Name = "";
                        }
                        else {
                            playerX = playerName;
                            playerO = "Computer";
                            state = GameState::PLAYING;
                            ResetBoard(board, cursorRow, cursorCol, turn, gameOver, winner, offsetX, offsetY, CELL_SIZE);
                            aiClock.restart();
                        }
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Backspace && !playerName.empty()) {
                        playerName.pop_back();
                    }
                }
                else if (state == GameState::INPUT_NAME_P2) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Enter && !player2Name.empty()) {
                        playerX = playerName;
                        playerO = player2Name;
                        state = GameState::PLAYING;
                        ResetBoard(board, cursorRow, cursorCol, turn, gameOver, winner, offsetX, offsetY, CELL_SIZE);
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Backspace && !player2Name.empty()) {
                        player2Name.pop_back();
                    }
                }
                else if (state == GameState::INPUT_NAME_P2) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Enter && !player2Name.empty()) {
                        playerX = playerName;
                        playerO = player2Name;
                        state = GameState::PLAYING;
                        ResetBoard(board, cursorRow, cursorCol, turn, gameOver, winner, offsetX, offsetY, CELL_SIZE);
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Backspace && !player2Name.empty()) {
                        player2Name.pop_back();
                    }
                }
                else if (state == GameState::LOAD_GAME_LIST) {
                    if (!savedGames.empty()) {
                        if (keyPressed->scancode == sf::Keyboard::Scancode::W) {
                            loadGameSelected = (loadGameSelected - 1 + savedGames.size()) % savedGames.size();
                        }
                        else if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
                            loadGameSelected = (loadGameSelected + 1) % savedGames.size();
                        }
                        else if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                            if (LoadGame(board, turn, savedGames[loadGameSelected].filename.c_str(),
                                gameOver, winner, playerX, playerO, gameMode)) {
                                playerName = playerX;
                                // Update board shapes after loading
                                for (int i = 0; i < BOARD_SIZE; ++i) {
                                    for (int j = 0; j < BOARD_SIZE; ++j) {
                                        board[i][j].shape.setSize({ CELL_SIZE - 2.f, CELL_SIZE - 2.f });
                                        board[i][j].shape.setPosition({ offsetX + j * CELL_SIZE, offsetY + i * CELL_SIZE });
                                        board[i][j].shape.setFillColor(sf::Color(240, 240, 240, 50));
                                        board[i][j].shape.setOutlineThickness(1);
                                        board[i][j].shape.setOutlineColor(sf::Color(100, 100, 100));
                                    }
                                }
                                state = GameState::PLAYING;
                                aiClock.restart();
                            }
                        }
                        else if (keyPressed->scancode == sf::Keyboard::Scancode::Delete) {
                            // Delete selected save
                            if (DeleteSavedGame(savedGames[loadGameSelected].filename)) {
                                savedGames = GetSavedGames();
                                if (savedGames.empty()) {
                                    loadGameSelected = 0;
                                }
                                else if (loadGameSelected >= savedGames.size()) {
                                    loadGameSelected = savedGames.size() - 1;
                                }
                            }
                        }
                        else if (keyPressed->scancode == sf::Keyboard::Scancode::C) {
                            // Clear all saves
                            if (ClearAllSavedGames()) {
                                savedGames.clear();
                                loadGameSelected = 0;
                            }
                        }
                    }
                }
                else if (state == GameState::PAUSE_MENU) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::W) {
                        pauseSelected = (pauseSelected - 1 + pauseTexts.size()) % pauseTexts.size();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
                        pauseSelected = (pauseSelected + 1) % pauseTexts.size();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                        if (soundOn) sfx.play();
                        string chosen = pauseMenuItems[pauseSelected];
                        if (chosen == "Resume") {
                            state = GameState::PLAYING;
                        }
                        else if (chosen == "Save & Exit") {
                            SaveGame(board, turn, playerName, gameMode, "autosave.caro");
                            state = GameState::MENU;
                        }
                        else if (chosen == "Exit without Saving") {
                            state = GameState::MENU;
                        }
                    }

                    UpdateMenuSelection(pauseTexts, pauseBoxes, pauseSelected);
                }
                else if (state == GameState::PLAYING && !gameOver) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::P) {
                        state = GameState::PAUSE_MENU;
                        pauseSelected = 0;
                        pauseTexts.clear();
                        pauseBoxes.clear();
                        InitMenu(pauseTexts, pauseBoxes, pauseMenuItems, font, Width, Height, pauseSelected);
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::W) {
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
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                        if (gameMode == GameMode::PVP || (gameMode == GameMode::PVC && turn)) {
                            if (board[cursorRow][cursorCol].c == 0) {
                                board[cursorRow][cursorCol].c = (turn ? -1 : 1);

                                if (soundOn) sfx.play();

                                if (CheckWin(board, cursorRow, cursorCol)) {
                                    gameOver = true;
                                    winner = (turn ? playerX + " wins!" : playerO + " wins!");
                                    SaveMatchResult(playerName, winner, gameMode, playerO);
                                }

                                turn = !turn;
                                aiClock.restart();
                            }
                        }
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
                else if (state == GameState::INPUT_NAME_P2) {
                    if (textEntered->unicode < 128 && textEntered->unicode != 8 && textEntered->unicode != 13) {
                        char c = static_cast<char>(textEntered->unicode);
                        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                            (c >= '0' && c <= '9') || c == ' ' || c == '_') {
                            if (player2Name.length() < 20) {
                                player2Name += c;
                            }
                        }
                    }
                }
            }
        }

        // AI Move logic
        if (state == GameState::PLAYING && !gameOver && gameMode == GameMode::PVC && !turn) {
            if (aiClock.getElapsedTime().asSeconds() > 0.5f) {
                int aiRow, aiCol;
                AIMove(board, aiRow, aiCol);

                if (aiRow >= 0 && aiCol >= 0 && board[aiRow][aiCol].c == 0) {
                    board[aiRow][aiCol].c = 1;

                    if (soundOn) sfx.play();

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

        cursor.setPosition({ offsetX + cursorCol * CELL_SIZE, offsetY + cursorRow * CELL_SIZE });

        // ===== Render =====
        window.clear();
        window.draw(background);

        if (state == GameState::MENU) {
            RenderMenu(window, title, texts, boxes);
        }
        else if (state == GameState::INPUT_NAME) {
            // Player 1 input
            sf::Text title1(font);
            title1.setString("PLAYER 1 - ENTER YOUR NAME");
            title1.setCharacterSize(50);
            sf::FloatRect titleBounds1 = title1.getLocalBounds();
            title1.setOrigin({ titleBounds1.size.x / 2, titleBounds1.size.y / 2 });
            title1.setPosition({ Width / 2, Height / 3 });
            title1.setFillColor(sf::Color::Yellow);
            window.draw(title1);

            sf::RectangleShape inputBox({ 500, 60 });
            inputBox.setOrigin({ 250, 30 });
            inputBox.setPosition({ Width / 2, Height / 2 });
            inputBox.setFillColor(sf::Color(50, 50, 80, 200));
            inputBox.setOutlineThickness(3);
            inputBox.setOutlineColor(sf::Color::White);
            window.draw(inputBox);

            sf::Text nameText(font);
            nameText.setString(playerName.empty() ? "_" : playerName);
            nameText.setCharacterSize(40);
            sf::FloatRect nameBounds = nameText.getLocalBounds();
            nameText.setOrigin({ nameBounds.size.x / 2, nameBounds.size.y / 2 });
            nameText.setPosition({ Width / 2, Height / 2 });
            nameText.setFillColor(sf::Color::White);
            window.draw(nameText);

            sf::Text instruction(font);
            instruction.setString("Press ENTER when done | ESC to cancel");
            instruction.setCharacterSize(20);
            sf::FloatRect instrBounds = instruction.getLocalBounds();
            instruction.setOrigin({ instrBounds.size.x / 2, instrBounds.size.y / 2 });
            instruction.setPosition({ Width / 2, Height * 2 / 3 });
            instruction.setFillColor(sf::Color(200, 200, 200));
            window.draw(instruction);
        }
        else if (state == GameState::INPUT_NAME_P2) {
            // Player 2 input
            sf::Text title2(font);
            title2.setString("PLAYER 2 - ENTER YOUR NAME");
            title2.setCharacterSize(50);
            sf::FloatRect titleBounds2 = title2.getLocalBounds();
            title2.setOrigin({ titleBounds2.size.x / 2, titleBounds2.size.y / 2 });
            title2.setPosition({ Width / 2, Height / 3 });
            title2.setFillColor(sf::Color::Cyan);
            window.draw(title2);

            sf::RectangleShape inputBox({ 500, 60 });
            inputBox.setOrigin({ 250, 30 });
            inputBox.setPosition({ Width / 2, Height / 2 });
            inputBox.setFillColor(sf::Color(50, 50, 80, 200));
            inputBox.setOutlineThickness(3);
            inputBox.setOutlineColor(sf::Color::White);
            window.draw(inputBox);

            sf::Text nameText(font);
            nameText.setString(player2Name.empty() ? "_" : player2Name);
            nameText.setCharacterSize(40);
            sf::FloatRect nameBounds = nameText.getLocalBounds();
            nameText.setOrigin({ nameBounds.size.x / 2, nameBounds.size.y / 2 });
            nameText.setPosition({ Width / 2, Height / 2 });
            nameText.setFillColor(sf::Color::White);
            window.draw(nameText);

            sf::Text instruction(font);
            instruction.setString("Press ENTER when done | ESC to cancel");
            instruction.setCharacterSize(20);
            sf::FloatRect instrBounds = instruction.getLocalBounds();
            instruction.setOrigin({ instrBounds.size.x / 2, instrBounds.size.y / 2 });
            instruction.setPosition({ Width / 2, Height * 2 / 3 });
            instruction.setFillColor(sf::Color(200, 200, 200));
            window.draw(instruction);
        }
        else if (state == GameState::PLAYING) {
            RenderGameplay(window, font, board, cursor, gameOver, winner, turn, playerX, playerO,
                offsetX, offsetY, CELL_SIZE, Width, gameMode);
        }
        else if (state == GameState::PAUSE_MENU) {
            RenderGameplay(window, font, board, cursor, gameOver, winner, turn, playerX, playerO,
                offsetX, offsetY, CELL_SIZE, Width, gameMode);
            RenderPauseMenu(window, font, pauseTexts, pauseBoxes, Width, Height);
        }
        else if (state == GameState::LOAD_GAME_LIST) {
            RenderLoadGameList(window, font, savedGames, loadGameSelected, Width, Height);
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