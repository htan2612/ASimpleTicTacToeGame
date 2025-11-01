#include "Graphic.h"
#include <iostream>
using namespace std;

bool LoadGraphicResources(sf::Texture& bgTexture, sf::Font& font) {
    if (!bgTexture.loadFromFile("../assets/bg.png"))
    {
        cerr << "Không thể load ảnh background.jpg trong folder assets!\n";
        return false;
    }

    if (!font.openFromFile("../assets/ARCADECLASSIC.TTF")) {
        cerr << "Không thể load font ARCADECLASSIC.TTF\n";
        return false;
    }

    return true;
}