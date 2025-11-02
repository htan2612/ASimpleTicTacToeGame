#ifndef CONSOLE_H
#define CONSOLE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Console {
public:
    Console(sf::RenderWindow& window, sf::Font& font);
    void addMessage(const std::string& msg);
    void render();
    void clear();

private:
    sf::RenderWindow& window;
    sf::Font& font;
    std::vector<std::string> messages;
    static const int MAX_MESSAGES = 5;
};

#endif