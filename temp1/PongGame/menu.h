#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

enum class GameState {
    MENU,
    PLAYING,
    SETTINGS,
    ABOUT,
    LOADGAME,
    INPUT_NAME
};

enum class GameMode {
    PVP,
    PVC
};

void InitMenu(std::vector<sf::Text>& texts, std::vector<sf::RectangleShape>& boxes,
    const std::vector<std::string>& menuItems, sf::Font& font,
    float Width, float Height, int selected);

void UpdateMenuSelection(std::vector<sf::Text>& texts, std::vector<sf::RectangleShape>& boxes,
    int selected);

void RenderMenu(sf::RenderWindow& window, const sf::Text& title,
    const std::vector<sf::Text>& texts,
    const std::vector<sf::RectangleShape>& boxes);

void RenderInputName(sf::RenderWindow& window, sf::Font& font,
    const std::string& playerName, float Width, float Height);

#endif