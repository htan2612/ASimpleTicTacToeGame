#ifndef ABOUT_H
#define ABOUT_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct Member {
    std::string name;
    std::string studentId;
};

void RenderAbout(sf::RenderWindow& window, sf::Font& font, const sf::Texture* avatarTexture = nullptr);

#endif