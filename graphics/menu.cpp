//Вика, это первая половина твоего фалйа.
//Т.к в товоем есть маин, он конфликутет с маином програмы
// и чтобы все это интегрировать в один файл сливаю код так


#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <iostream>
#include "config.h"   // чтобы видеть класс ConfigManager

using std::vector;
using std::array;

class Menu {
private:
    struct Area {
        int x0, y0, x, y;
        int num;
    };
    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite;
    array<Area, 3> transition;
    sf::Text text;
    sf::FloatRect bounds;
    float x_head, y_head;
    sf::RectangleShape rect;
    vector<sf::Text> button;
    sf::Font font;
    ConfigManager& cfg_;   // ссылка на конфиг

public:
    // Конструктор теперь принимает ConfigManager&
    Menu(ConfigManager& cfg) : window(sf::VideoMode::getDesktopMode(), "Выбор скорости"), cfg_(cfg) {
        unsigned int x_window = window.getSize().x;
        unsigned int y_window = window.getSize().y;
        x_head = x_window / 15.0;
        y_head = y_window / 15.0;

        if (!font.loadFromFile("G.ttf"))
            std::cout << "Font G.ttf not loaded\n";

        text = sf::Text("pls set speed (km/s):", font, 60);
        text.setFillColor(sf::Color::Black);
        text.setPosition(x_head, y_head);
        bounds = text.getGlobalBounds();
        rect = sf::RectangleShape(sf::Vector2f(bounds.width + x_head/6.0,
                                               bounds.height + y_head/6.0));
        rect.setPosition(sf::Vector2f(bounds.left - x_head/12.0,
                                      bounds.top - x_head/12.0));
        rect.setFillColor(sf::Color(255,255,255));

        button = {
            sf::Text("20 - 40", font, 70),
            sf::Text("40 - 90", font, 70),
            sf::Text("90 - 150", font, 70)
        };
        float currentX = x_head;
        for (size_t i = 0; i < button.size(); ++i) {
            button[i].setFillColor(sf::Color::White);
            button[i].setPosition(currentX, 3 * y_head);
            sf::FloatRect boundsP = button[i].getGlobalBounds();
            transition[i] = { (int)boundsP.left, (int)boundsP.top,
                              (int)(boundsP.left + boundsP.width),
                              (int)(boundsP.top + boundsP.height),
                              (int)i+1 };
            currentX += boundsP.width + 130.0f;
        }

        if (!texture.loadFromFile("F.jpg"))
            std::cout << "Background F.jpg not loaded\n";
        sprite.setTexture(texture);
        sf::Vector2u tSize = texture.getSize();
        sf::Vector2u wSize = window.getSize();
        sprite.setScale(static_cast<float>(wSize.x)/tSize.x,
                        static_cast<float>(wSize.y)/tSize.y);
    }

    // Метод show() уже не возвращает int, а прямо записывает в cfg_
    void show() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == sf::Mouse::Left) {
                    for (auto& area : transition) {
                        if (area.x0 <= event.mouseButton.x &&
                            event.mouseButton.x <= area.x &&
                            area.y0 <= event.mouseButton.y &&
                            event.mouseButton.y <= area.y) {
                            // В зависимости от выбранной кнопки, устанавливаем диапазон
                            switch (area.num) {
                                case 1:
                                    cfg_.set_asteroid_speed_range(20000.0, 40000.0);
                                    break;
                                case 2:
                                    cfg_.set_asteroid_speed_range(40000.0, 90000.0);
                                    break;
                                case 3:
                                    cfg_.set_asteroid_speed_range(90000.0, 150000.0);
                                    break;
                            }
                            window.close();
                        }
                    }
                }
            }
            window.clear();
            window.draw(sprite);
            window.draw(rect);
            window.draw(text);
            for (auto& btn : button) window.draw(btn);
            window.display();
        }
    }
};