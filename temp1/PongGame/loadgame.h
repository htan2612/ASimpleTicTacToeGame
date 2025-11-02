#ifndef LOADGAME_H
#define LOADGAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Menu.h"

struct SavedGame {
    std::string filename;
    std::string playerX;
    std::string playerO;
    std::string date;
    GameMode mode;
};

std::vector<SavedGame> GetSavedGames();

void RenderLoadGameList(sf::RenderWindow& window, sf::Font& font,
    const std::vector<SavedGame>& savedGames,
    int selected, float Width, float Height);

#endif