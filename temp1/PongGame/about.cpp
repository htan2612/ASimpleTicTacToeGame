#include "About.h"

void RenderAbout(sf::RenderWindow& window, sf::Font& font, const sf::Texture* avatarTexture) {
    sf::Vector2u size = window.getSize();
    float Width = size.x;
    float Height = size.y;

    // Title
    sf::Text title(font);
    title.setString("ABOUT US");
    title.setCharacterSize(Height / 12);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({ titleBounds.size.x / 2, titleBounds.size.y / 2 });
    title.setPosition({ Width / 2, Height / 8 });
    title.setFillColor(sf::Color::Yellow);
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(3);
    window.draw(title);

    // Team members info (để điền sau)
    std::vector<Member> members = {
        {"Nguyen Tran Kim Cuong", "25127023"},
        {"Nguyen Vu Duc Duy", "25127190"},
        {"Nguyen Minh Khoi", "25127389"},
        {"Huynh Minh Trung", "25127163"},
        {"Huynh Minh Trung", "25127178"}
    };

    float startY = Height / 3;
    float spacing = Height / 12;
    float avatarSize = 80.f;

    for (int i = 0; i < members.size(); i++) {
        float posY = startY + i * spacing;

        // Avatar placeholder (small circle or image)
        sf::CircleShape avatar(avatarSize / 2);
        avatar.setPosition({ Width / 3 - avatarSize, posY - avatarSize / 2 });
        avatar.setFillColor(sf::Color(100, 150, 200, 200));
        avatar.setOutlineThickness(2);
        avatar.setOutlineColor(sf::Color::White);

        // If avatar texture is provided, use it
        if (avatarTexture) {
            sf::Sprite avatarSprite(*avatarTexture);
            sf::Vector2u texSize = avatarTexture->getSize();
            float scale = avatarSize / static_cast<float>(texSize.x);
            avatarSprite.setScale({ scale, scale });
            avatarSprite.setPosition({ Width / 3 - avatarSize, posY - avatarSize / 2 });
            window.draw(avatarSprite);
        }
        else {
            window.draw(avatar);
        }

        // Name
        sf::Text nameText(font);
        nameText.setString(members[i].name);
        nameText.setCharacterSize(28);
        nameText.setFillColor(sf::Color::White);
        nameText.setPosition({ Width / 3 + 20, posY - 25 });
        window.draw(nameText);

        // Student ID
        sf::Text idText(font);
        idText.setString("ID " + members[i].studentId);
        idText.setCharacterSize(22);
        idText.setFillColor(sf::Color(200, 200, 200));
        idText.setPosition({ Width / 3 + 20, posY + 5 });
        window.draw(idText);
    }

    // Footer
    sf::Text footer(font);
    footer.setString("Press ESC to return");
    footer.setCharacterSize(20);
    sf::FloatRect footerBounds = footer.getLocalBounds();
    footer.setOrigin({ footerBounds.size.x / 2, footerBounds.size.y / 2 });
    footer.setPosition({ Width / 2, Height - 50 });
    footer.setFillColor(sf::Color(180, 180, 180));
    window.draw(footer);
}