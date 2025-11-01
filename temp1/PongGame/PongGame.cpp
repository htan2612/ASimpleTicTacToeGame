#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

enum class GameState {
    MENU,
    PLAYING,
    SETTINGS,
    ABOUT,
    LOADGAME
};

const int BOARD_SIZE = 12;

struct Cell {
    int c; // 0 = trống, -1 = X, 1 = O
    sf::RectangleShape shape;
};

int main()
{
    // Tạo cửa sổ game
    sf::RenderWindow window(sf::VideoMode({ 1600,900 }), "Caro Game", sf::Style::Default, sf::State::Windowed);
    sf::Vector2u size = window.getSize();
    float Width = size.x;
    float Height = size.y;
    window.setFramerateLimit(60);

    // ===== Background =====
    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("../assets/bg.png"))
    {
        cerr << "Không thể load ảnh background.jpg trong folder assets!\n";
        return EXIT_FAILURE;
    }
    sf::Sprite background(bgTexture);

    // Scale ảnh để khớp với kích thước cửa sổ
    sf::Vector2u texSize = bgTexture.getSize();
    float scaleX = static_cast<float>(Width) / texSize.x;
    float scaleY = static_cast<float>(Height) / texSize.y;
    background.setScale({ scaleX, scaleY });

    // ===== Font và Text =====
    sf::Font font;
    if (!font.openFromFile("../assets/ARCADECLASSIC.TTF")) return EXIT_FAILURE;

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
    bool soundOn = true;
    sf::Clock clock;

    //Menu setup
    vector<string> menuItems = { "New Game", "Load Game", "Settings","ABOUT", "Exit" };
    vector<sf::Text> texts;
    vector<sf::RectangleShape> boxes;
    int selected = 0;

    float startY = Height / 2.f - (menuItems.size() * (Height / 10.f)) / 2.f;
    float spacing = Height / 9;

    for (int i = 0; i < menuItems.size(); ++i)
    {
        float posY = startY + i * spacing + 150;
        sf::Text text(font);
        text.setString(menuItems[i]);
        text.setCharacterSize(Height / 20);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.size.x / 2, bounds.size.y / 2 });
        text.setFillColor(i == selected ? sf::Color::Cyan : sf::Color::White);
        text.setPosition({ Width / 2,posY });
        texts.push_back(text);

        float boxWidth = Width / 3;
        float boxHeight = Height / 12;
        sf::RectangleShape box({ boxWidth,boxHeight });
        box.setOrigin({ boxWidth / 2,boxHeight / 2 });
        box.setPosition({ Width / 2,posY });

        if (i == selected) {
            box.setFillColor(sf::Color({ 100,100,255,180 }));
        }
        else {
            box.setFillColor(sf::Color({ 50,80,120,140 }));
        }
        box.setOutlineThickness(3);
        box.setOutlineColor(sf::Color::White);

        boxes.push_back(box);
    }

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

    // Hàm reset board
    auto ResetBoard = [&]() {
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
        };

    ResetBoard();

    // Text hiển thị thông tin game
    sf::Text turnText(font);
    turnText.setCharacterSize(30);
    turnText.setFillColor(sf::Color::White);
    turnText.setPosition({ 50, 50 });

    sf::Text resultText(font);
    resultText.setCharacterSize(50);
    resultText.setFillColor(sf::Color::Green);
    sf::FloatRect resBounds = resultText.getLocalBounds();
    resultText.setOrigin({ resBounds.size.x / 2, resBounds.size.y / 2 });
    resultText.setPosition({ Width / 2, 50 });

    sf::Text instructionText(font);
    instructionText.setString("WASD: Move | Enter: Place | L: Save | T: Load | ESC: Menu");
    instructionText.setCharacterSize(20);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition({ 50, Height - 40 });

    // Hàm kiểm tra thắng (giống code console)
    auto CheckWin = [&](int i, int j) -> bool {
        int c = board[i][j].c;
        if (c == 0) return false;

        int dx[] = { 0, 1, 1, 1 };
        int dy[] = { 1, 0, 1, -1 };

        for (int dir = 0; dir < 4; ++dir) {
            int count = 1;

            // Đếm về phía trước
            for (int k = 1; k < 5; ++k) {
                int ni = i + dx[dir] * k;
                int nj = j + dy[dir] * k;
                if (ni < 0 || nj < 0 || ni >= BOARD_SIZE || nj >= BOARD_SIZE) break;
                if (board[ni][nj].c == c) count++;
                else break;
            }

            // Đếm về phía sau
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
        };

    // Hàm save game
    auto SaveGame = [&](const char* filename) {
        ofstream f(filename, ios::binary);
        if (!f) return false;
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                f.write((char*)&board[i][j].c, sizeof(int));
        f.write((char*)&turn, sizeof(bool));
        f.close();
        return true;
        };

    // Hàm load game
    auto LoadGame = [&](const char* filename) {
        ifstream f(filename, ios::binary);
        if (!f) return false;
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                f.read((char*)&board[i][j].c, sizeof(int));
        f.read((char*)&turn, sizeof(bool));
        f.close();
        gameOver = false;
        winner = "";
        return true;
        };

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
                        if (chosen == "New Game") {
                            state = GameState::PLAYING;
                            ResetBoard();
                        }
                        else if (chosen == "Load Game") {
                            if (LoadGame("save.caro")) {
                                state = GameState::PLAYING;
                            }
                        }
                        else if (chosen == "Exit") {
                            window.close();
                        }
                        else if (chosen == "Settings") state = GameState::SETTINGS;
                        else if (chosen == "ABOUT") state = GameState::ABOUT;
                    }

                    //Update highlight
                    for (int i = 0; i < texts.size(); i++) {
                        bool isSelected = (i == selected);
                        texts[i].setFillColor(isSelected ? sf::Color::Cyan : sf::Color::White);

                        if (isSelected) {
                            boxes[i].setFillColor(sf::Color(100, 180, 255, 200));
                            boxes[i].setScale({ 1.1f,1.1f });
                        }
                        else {
                            boxes[i].setFillColor(sf::Color(50, 80, 120, 140));
                            boxes[i].setScale({ 1,1 });
                        }
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
                        if (board[cursorRow][cursorCol].c == 0) {
                            board[cursorRow][cursorCol].c = (turn ? -1 : 1);

                            if (soundOn) sfx.play();

                            // Kiểm tra thắng
                            if (CheckWin(cursorRow, cursorCol)) {
                                gameOver = true;
                                winner = (turn ? playerX + " wins!" : playerO + " wins!");
                            }

                            turn = !turn;
                        }
                    }
                    // Save game
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::L) {
                        SaveGame("save.caro");
                    }
                    // Load game
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::T) {
                        LoadGame("save.caro");
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
        }

        // Cập nhật vị trí cursor
        cursor.setPosition({ offsetX + cursorCol * CELL_SIZE, offsetY + cursorRow * CELL_SIZE });

        // ===== Render =====
        window.clear();
        window.draw(background);

        if (state == GameState::MENU) {
            window.draw(title);
            for (int i = 0; i < texts.size(); i++) {
                window.draw(boxes[i]);
                window.draw(texts[i]);
            }
        }
        else if (state == GameState::PLAYING) {
            // Vẽ bàn cờ
            for (int i = 0; i < BOARD_SIZE; ++i) {
                for (int j = 0; j < BOARD_SIZE; ++j) {
                    window.draw(board[i][j].shape);

                    // Vẽ X hoặc O
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

            // Vẽ cursor
            if (!gameOver) {
                window.draw(cursor);
            }

            // Hiển thị lượt chơi hoặc kết quả
            if (gameOver) {
                resultText.setString(winner);
                sf::FloatRect resBounds = resultText.getLocalBounds();
                resultText.setOrigin({ resBounds.size.x / 2, resBounds.size.y / 2 });
                resultText.setPosition({ Width / 2, 50 });
                window.draw(resultText);
            }
            else {
                turnText.setString(turn ? (playerX + "'s turn (X)") : (playerO + "'s turn (O)"));
                window.draw(turnText);
            }

            window.draw(instructionText);
        }
        else if (state == GameState::SETTINGS) {
            sf::Text settingsTitle(font);
            settingsTitle.setString("SETTINGS");
            settingsTitle.setCharacterSize(60);
            sf::FloatRect stBounds = settingsTitle.getLocalBounds();
            settingsTitle.setOrigin({ stBounds.size.x / 2, stBounds.size.y / 2 });
            settingsTitle.setPosition({ Width / 2, 150 });
            settingsTitle.setFillColor(sf::Color::Yellow);
            window.draw(settingsTitle);

            sf::Text soundText(font);
            soundText.setString(string("Sound: ") + (soundOn ? "ON" : "OFF"));
            soundText.setCharacterSize(40);
            sf::FloatRect sdBounds = soundText.getLocalBounds();
            soundText.setOrigin({ sdBounds.size.x / 2, sdBounds.size.y / 2 });
            soundText.setPosition({ Width / 2, Height / 2 });
            soundText.setFillColor(sf::Color::White);
            window.draw(soundText);

            sf::Text backText(font);
            backText.setString("Press ENTER to toggle | ESC to return");
            backText.setCharacterSize(25);
            sf::FloatRect bkBounds = backText.getLocalBounds();
            backText.setOrigin({ bkBounds.size.x / 2, bkBounds.size.y / 2 });
            backText.setPosition({ Width / 2, Height - 100 });
            backText.setFillColor(sf::Color(200, 200, 200));
            window.draw(backText);
        }
        else {
            sf::Text Annouce(font);
            Annouce.setString("COMING SOON");
            sf::FloatRect anBounds = Annouce.getLocalBounds();
            Annouce.setOrigin({ anBounds.size.x / 2, anBounds.size.y / 2 });
            Annouce.setPosition({ Width / 2, Height / 2 });
            Annouce.setCharacterSize(Height / 10);
            Annouce.setFillColor(sf::Color::Red);
            window.draw(Annouce);
        }

        window.display();
    }
}