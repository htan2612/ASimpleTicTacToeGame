#include "LoadGame.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

std::vector<SavedGame> GetSavedGames() {
    std::vector<SavedGame> games;

    try {
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.path().extension() == ".caro") {
                SavedGame game;
                game.filename = entry.path().string();

                // Read metadata from file
                std::ifstream file(game.filename, std::ios::binary);
                if (file) {
                    // Skip board data
                    file.seekg(144 * sizeof(int), std::ios::beg); // 12x12 board

                    // Read turn
                    bool turn;
                    file.read((char*)&turn, sizeof(bool));

                    // Read metadata (256 bytes)
                    char metadata[256] = { 0 };
                    file.read(metadata, 256);

                    std::string metaStr(metadata);
                    size_t pos = metaStr.find('|');
                    if (pos != std::string::npos) {
                        game.playerX = metaStr.substr(0, pos);
                        std::string modeStr = metaStr.substr(pos + 1);
                        game.mode = (modeStr == "PVP") ? GameMode::PVP : GameMode::PVC;
                        game.playerO = (game.mode == GameMode::PVP) ? "Player 2" : "Computer";
                    }

                    file.close();

                    // Get file modification time
                    auto ftime = fs::last_write_time(entry);
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                    );
                    auto time = std::chrono::system_clock::to_time_t(sctp);

                    std::tm timeinfo;
                    localtime_s(&timeinfo, &time);
                    std::stringstream ss;
                    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M");
                    game.date = ss.str();

                    games.push_back(game);
                }
            }
        }
    }
    catch (...) {
        // Handle errors silently
    }

    return games;
}

void RenderLoadGameList(sf::RenderWindow& window, sf::Font& font,
    const std::vector<SavedGame>& savedGames,
    int selected, float Width, float Height) {

    // Title
    sf::Text title(font);
    title.setString("LOAD GAME");
    title.setCharacterSize(60);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({ titleBounds.size.x / 2, titleBounds.size.y / 2 });
    title.setPosition({ Width / 2, 80 });
    title.setFillColor(sf::Color::Yellow);
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(3);
    window.draw(title);

    if (savedGames.empty()) {
        sf::Text noGames(font);
        noGames.setString("No saved games found");
        noGames.setCharacterSize(30);
        sf::FloatRect ngBounds = noGames.getLocalBounds();
        noGames.setOrigin({ ngBounds.size.x / 2, ngBounds.size.y / 2 });
        noGames.setPosition({ Width / 2, Height / 2 });
        noGames.setFillColor(sf::Color::White);
        window.draw(noGames);
    }
    else {
        float startY = 180;
        float spacing = 100;

        for (size_t i = 0; i < savedGames.size(); i++) {
            float posY = startY + i * spacing;
            bool isSelected = (i == selected);

            // Background box
            sf::RectangleShape box({ Width - 200, 80 });
            box.setPosition({ 100, posY });
            box.setFillColor(isSelected ? sf::Color(100, 180, 255, 200) : sf::Color(50, 80, 120, 140));
            box.setOutlineThickness(2);
            box.setOutlineColor(sf::Color::White);
            window.draw(box);

            // Game info
            sf::Text gameInfo(font);
            std::string modeStr = (savedGames[i].mode == GameMode::PVP) ? "PVP" : "PVC";
            gameInfo.setString(savedGames[i].playerX + " vs " + savedGames[i].playerO +
                " (" + modeStr + ")");
            gameInfo.setCharacterSize(28);
            gameInfo.setPosition({ 120, posY + 10 });
            gameInfo.setFillColor(sf::Color::White);
            window.draw(gameInfo);

            // Date
            sf::Text dateText(font);
            dateText.setString(savedGames[i].date);
            dateText.setCharacterSize(20);
            dateText.setPosition({ 120, posY + 45 });
            dateText.setFillColor(sf::Color(200, 200, 200));
            window.draw(dateText);
        }
    }

    // Instructions
    sf::Text instruction(font);
    instruction.setString("W/S: Select | ENTER: Load | ESC: Back");
    instruction.setCharacterSize(20);
    sf::FloatRect instrBounds = instruction.getLocalBounds();
    instruction.setOrigin({ instrBounds.size.x / 2, instrBounds.size.y / 2 });
    instruction.setPosition({ Width / 2, Height - 50 });
    instruction.setFillColor(sf::Color(180, 180, 180));
    window.draw(instruction);
}