#include "Menu.h"

void InitMenu(std::vector<sf::Text>& texts, std::vector<sf::RectangleShape>& boxes,
    const std::vector<std::string>& menuItems, sf::Font& font,
    float Width, float Height, int selected) {

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
}

void UpdateMenuSelection(std::vector<sf::Text>& texts, std::vector<sf::RectangleShape>& boxes,
    int selected) {
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

void RenderMenu(sf::RenderWindow& window, const sf::Text& title,
    const std::vector<sf::Text>& texts,
    const std::vector<sf::RectangleShape>& boxes) {
    window.draw(title);
    for (int i = 0; i < texts.size(); i++) {
        window.draw(boxes[i]);
        window.draw(texts[i]);
    }
}

void RenderInputName(sf::RenderWindow& window, sf::Font& font,
    const std::string& playerName, float Width, float Height) {

    sf::Text title(font);
    title.setString("ENTER YOUR NAME");
    title.setCharacterSize(50);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({ titleBounds.size.x / 2, titleBounds.size.y / 2 });
    title.setPosition({ Width / 2, Height / 3 });
    title.setFillColor(sf::Color::Yellow);
    window.draw(title);

    // Input box
    sf::RectangleShape inputBox({ 500, 60 });
    inputBox.setOrigin({ 250, 30 });
    inputBox.setPosition({ Width / 2, Height / 2 });
    inputBox.setFillColor(sf::Color(50, 50, 80, 200));
    inputBox.setOutlineThickness(3);
    inputBox.setOutlineColor(sf::Color::White);
    window.draw(inputBox);

    // Player name text
    sf::Text nameText(font);
    nameText.setString(playerName.empty() ? "_" : playerName);
    nameText.setCharacterSize(40);
    sf::FloatRect nameBounds = nameText.getLocalBounds();
    nameText.setOrigin({ nameBounds.size.x / 2, nameBounds.size.y / 2 });
    nameText.setPosition({ Width / 2, Height / 2 });
    nameText.setFillColor(sf::Color::White);
    window.draw(nameText);

    // Instruction
    sf::Text instruction(font);
    instruction.setString("Press ENTER when done | ESC to cancel");
    instruction.setCharacterSize(20);
    sf::FloatRect instrBounds = instruction.getLocalBounds();
    instruction.setOrigin({ instrBounds.size.x / 2, instrBounds.size.y / 2 });
    instruction.setPosition({ Width / 2, Height * 2 / 3 });
    instruction.setFillColor(sf::Color(200, 200, 200));
    window.draw(instruction);
}

void RenderPauseMenu(sf::RenderWindow& window, sf::Font& font,
    const std::vector<sf::Text>& texts,
    const std::vector<sf::RectangleShape>& boxes, float Width, float Height) {

    // Semi-transparent overlay
    sf::RectangleShape overlay({ static_cast<float>(Width), static_cast<float>(Height) });
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    // Pause title
    sf::Text title(font);
    title.setString("PAUSED");
    title.setCharacterSize(80);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({ titleBounds.size.x / 2, titleBounds.size.y / 2 });
    title.setPosition({ Width / 2, Height / 4 });
    title.setFillColor(sf::Color::Yellow);
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(4);
    window.draw(title);

    // Draw menu items
    for (size_t i = 0; i < boxes.size(); i++) {
        window.draw(boxes[i]);
        window.draw(texts[i]);
    }
}