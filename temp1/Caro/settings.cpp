#include "Settings.h"
#include <string>

void RenderSettings(sf::RenderWindow& window, sf::Font& font, bool soundOn, float Width, float Height) {
    sf::Text settingsTitle(font);
    settingsTitle.setString("SETTINGS");
    settingsTitle.setCharacterSize(60);
    sf::FloatRect stBounds = settingsTitle.getLocalBounds();
    settingsTitle.setOrigin({ stBounds.size.x / 2, stBounds.size.y / 2 });
    settingsTitle.setPosition({ Width / 2, 150 });
    settingsTitle.setFillColor(sf::Color::Yellow);
    window.draw(settingsTitle);

    sf::Text soundText(font);
    soundText.setString(std::string("Sound: ") + (soundOn ? "ON" : "OFF"));
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