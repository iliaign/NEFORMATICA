// visualization.cpp
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <limits>
#include "visualization.h"


using std::vector;
using std::array;
using std::string;
using std::map;

unsigned int x_window = sf::VideoMode::getDesktopMode().width;
unsigned int y_window = sf::VideoMode::getDesktopMode().height;

struct Point {
    double x, y;
    Point(double x, double y) : x(x), y(y) {}
};

class Graphic_object {
protected:
    sf::Color color;
    Point Centre{ x_window / 2.0, y_window / 2.0 };
public:
    Graphic_object() {}
    virtual void Draw(sf::RenderWindow& window) {}
    virtual void Update() {}
};

class Sun : public Graphic_object {
public:
    Sun() { color = sf::Color::Yellow; }
    void Draw(sf::RenderWindow& window) {
        sf::CircleShape Sun(10);
        Sun.setOrigin(sf::Vector2f(10, 10));
        Sun.setPosition(sf::Vector2f(Centre.x, Centre.y));
        Sun.setFillColor(color);
        window.draw(Sun);
    }
};

class Planet : public Graphic_object {
protected:
    sf::Time pause = sf::milliseconds(10);
    vector<Point> trajectory;
    sf::Clock animation_clock;
    int current_index = 0;
public:
    Planet() {};
    Planet(std::ifstream& file, int x_col) {
        std::string header;
        std::getline(file, header);
        double t, x, y;
        char comma;
        while (file.good()) {
            for (int i = 1; i < x_col; i++) {
                double dummy;
                file >> dummy >> comma;
            }
            file >> x >> comma >> y;
            trajectory.push_back(Point(x/2.0, y/2.0));
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        file.clear();
        file.seekg(0);
        std::getline(file, header);
    }
    unsigned int Get_trajectory() { return trajectory.size(); }
    void Update() {
        if (animation_clock.getElapsedTime() >= pause) {
            current_index = (current_index + 1) % trajectory.size();
            animation_clock.restart();
        }
    }
    void DrawFullTrajectory(sf::RenderWindow& window) {
        for (int i = 0; i < current_index; i++) {
            sf::CircleShape p(1);
            p.setFillColor(color);
            p.setPosition(sf::Vector2f(trajectory[i].x + Centre.x, trajectory[i].y + Centre.y));
            p.setOrigin(1, 1);
            window.draw(p);
        }
    }
    void Draw(sf::RenderWindow& window) {
        DrawFullTrajectory(window);
        sf::CircleShape p(3);
        p.setFillColor(color);
        p.setPosition(sf::Vector2f(trajectory[current_index].x + Centre.x, trajectory[current_index].y + Centre.y));
        p.setOrigin(3, 3);
        window.draw(p);
    }
};

class Asteroids : public Graphic_object {
private:
    struct AsteroidData {
        int id;
        vector<Point> trajectory;
        sf::Color color;
        int current_index = 0;
        sf::Clock animation_clock;
    };
    vector<AsteroidData> asteroids;
    sf::Time pause = sf::milliseconds(10);
public:
    Asteroids(std::ifstream& file) {
        std::string header;
        std::getline(file, header);
        double time;
        int id;
        double x, y;
        char comma;
        std::map<int, vector<Point>> tempTrajectories;
        while (file >> time >> comma >> id >> comma >> x >> comma >> y) {
            tempTrajectories[id].push_back(Point(x * 1e-9, y * 1e-9));
        }
        for (auto& pair : tempTrajectories) {
            AsteroidData ast;
            ast.id = pair.first;
            ast.trajectory = pair.second;
            ast.color = sf::Color::White;
            asteroids.push_back(ast);
        }
        file.close();
        std::cout << "Загружено астероидов: " << asteroids.size() << std::endl;
    }
    void Update() override {
        for (auto& ast : asteroids) {
            if (ast.animation_clock.getElapsedTime() >= pause) {
                ast.current_index = (ast.current_index + 1) % ast.trajectory.size();
                ast.animation_clock.restart();
            }
        }
    }
    void Draw(sf::RenderWindow& window) override {
        for (auto& ast : asteroids) {
            if (ast.trajectory.empty()) continue;
            for (int i = 0; i < ast.current_index; i++) {
                sf::CircleShape p(1);
                p.setFillColor(ast.color);
                p.setPosition(sf::Vector2f(ast.trajectory[i].x + Centre.x, ast.trajectory[i].y + Centre.y));
                p.setOrigin(1, 1);
                window.draw(p);
            }
            sf::CircleShape p(3);
            p.setFillColor(ast.color);
            p.setPosition(sf::Vector2f(ast.trajectory[ast.current_index].x + Centre.x,
                ast.trajectory[ast.current_index].y + Centre.y));
            p.setOrigin(3, 3);
            window.draw(p);
        }
    }
};

class Earth : public Planet {
public:
    Earth(std::ifstream& file, int x_col) : Planet(file, x_col) { color = sf::Color::Blue; }
};

class Mars : public Planet {
public:
    Mars(std::ifstream& file, int x_col) : Planet(file, x_col) { color = sf::Color::Red; }
};

class Venus : public Planet {
public:
    Venus(std::ifstream& file, int x_col) : Planet(file, x_col) { color = sf::Color(255, 165, 0); }
};

class Jupiter : public Planet {
public:
    Jupiter(std::ifstream& file, int x_col) : Planet(file, x_col) { color = sf::Color(244, 164, 96); }
};

class General_window {
private:
    sf::RenderWindow window{ sf::VideoMode(x_window, y_window), "Визуализация полета астероидов" };
public:
    General_window() {}
    ~General_window() {}
    void event_processing(vector<Graphic_object*>& bodies) {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            window.clear();
            for (auto x : bodies) {
                x->Draw(window);
                x->Update();
            }
            window.display();
        }
    }
};

// ============= МЕНЮ =============
void run_menu(ConfigManager& cfg) {
    class Menu {
    private:
        struct Area {
            int x0, y0, x, y;
            int num;
        };
        sf::RenderWindow window{ sf::VideoMode(x_window, y_window), "Начальное меню" };
        sf::Texture texture;
        sf::Sprite sprite;
        vector<Area> transition{5, Area{0,0,0,0,0}};
        vector<Area> transition2{3, Area{0,0,0,0,0}};
        Area transition_ok;
        sf::Text text, text2, text_ok;
        float x_head = x_window / 15.0;
        float y_head = y_window / 15.0;
        sf::RectangleShape rect, rect2, rect_ok;
        vector<sf::Text> button, button2;
        vector<bool> planet_selected;
        int selected_speed = -1;
        sf::Font font;
        ConfigManager& cfg_;
        
        void create_header(const string& s, sf::Text& t, sf::RectangleShape& rct, float x, float y) {
            t = sf::Text(s, font, 80);
            t.setFillColor(sf::Color::Black);
            t.setPosition(x, y);
            sf::FloatRect bounds = t.getGlobalBounds();
            rct = sf::RectangleShape(sf::Vector2f(bounds.width + x_head / 6.0, bounds.height + y_head / 6.0));
            rct.setPosition(sf::Vector2f(bounds.left - x_head / 18.0, bounds.top - x_head / 18.0));
            rct.setFillColor(sf::Color(255, 255, 255));
        }
        
        void create_buttons(vector<sf::Text>& btns, vector<Area>& trans,
                            const vector<string>& names, float y_pos) {
            float currentX = x_head;
            for (size_t i = 0; i < names.size(); i++) {
                btns.push_back(sf::Text(names[i], font, 70));
                btns[i].setFillColor(sf::Color::White);
                btns[i].setPosition(currentX, y_pos);
                sf::FloatRect boundsP = btns[i].getGlobalBounds();
                trans[i].num = i + 1;
                trans[i].x0 = boundsP.left;
                trans[i].y0 = boundsP.top;
                trans[i].x = boundsP.left + boundsP.width;
                trans[i].y = boundsP.top + boundsP.height;
                currentX += boundsP.width + x_window / 10.0;
            }
        }
        
    public:
        Menu(ConfigManager& cfg) : cfg_(cfg) {
            if (!font.loadFromFile("G.ttf"))
                std::cout << "Ошибка при открытии файла шрифта" << std::endl;
            
            create_header("Select the asteroid velocity range (km/s):", text, rect, x_head, y_head);
            create_header("Select the planets in the system:", text2, rect2, x_head, y_head * 6);
            create_header("OK", text_ok, rect_ok, x_head * 13, y_head * 12);
            
            sf::FloatRect boundsOK = text_ok.getGlobalBounds();
            transition_ok.num = 1;
            transition_ok.x0 = boundsOK.left;
            transition_ok.y0 = boundsOK.top;
            transition_ok.x = boundsOK.left + boundsOK.width;
            transition_ok.y = boundsOK.top + boundsOK.height;
            
            create_buttons(button, transition, {"20 - 40", "40 - 90", "90 - 150"}, 3 * y_head);
            create_buttons(button2, transition2, {"Venus", "Mars", "Jupiter"}, 8 * y_head);
            
            planet_selected.resize(button2.size(), false);
            
            if (!texture.loadFromFile("F.jpg")) 
                std::cout << "Ошибка при открытии файла F.jpg" << std::endl;
            
            sprite.setTexture(texture);
            sf::Vector2u tSize = texture.getSize();
            sf::Vector2u wSize = window.getSize();
            sprite.setScale(static_cast<float>(wSize.x) / tSize.x, static_cast<float>(wSize.y) / tSize.y);
        }
        
        void show() {
            while (window.isOpen()) {
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) 
                        window.close();
                    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                        for (size_t i = 0; i < transition.size(); i++) {
                            auto& area = transition[i];
                            if (area.x0 <= event.mouseButton.x && event.mouseButton.x <= area.x &&
                                area.y0 <= event.mouseButton.y && event.mouseButton.y <= area.y) {
                                if (selected_speed != -1)
                                    button[selected_speed].setFillColor(sf::Color::White);
                                selected_speed = i;
                                button[selected_speed].setFillColor(sf::Color::Green);
                                
                                switch (i) {
                                    case 0: cfg_.set_asteroid_speed_range(20000.0, 40000.0); break;
                                    case 1: cfg_.set_asteroid_speed_range(40000.0, 90000.0); break;
                                    case 2: cfg_.set_asteroid_speed_range(90000.0, 150000.0); break;
                                }
                            }
                        }
                        for (size_t i = 0; i < transition2.size(); i++) {
                            auto& area = transition2[i];
                            if (area.x0 <= event.mouseButton.x && event.mouseButton.x <= area.x &&
                                area.y0 <= event.mouseButton.y && event.mouseButton.y <= area.y) {
                                planet_selected[i] = !planet_selected[i];
                                button2[i].setFillColor(planet_selected[i] ? sf::Color::Green : sf::Color::White);
                                
                                switch (i) {
                                        case 0: cfg_.set_remove_venus(planet_selected[i]); break;
                                        case 1: cfg_.set_remove_mars(planet_selected[i]); break;
                                        case 2: cfg_.set_remove_jupiter(planet_selected[i]); break;
                                }
                            }
                        }
                        if (transition_ok.x0 <= event.mouseButton.x && event.mouseButton.x <= transition_ok.x &&
                            transition_ok.y0 <= event.mouseButton.y && event.mouseButton.y <= transition_ok.y) {
                            cfg_.save("config.json");
                            window.close();
                        }
                    }
                }
                window.clear();
                window.draw(sprite);
                window.draw(rect);
                window.draw(text);
                window.draw(rect2);
                window.draw(text2);
                for (auto& btn : button) window.draw(btn);
                for (auto& btn : button2) window.draw(btn);
                window.draw(rect_ok);
                window.draw(text_ok);
                window.display();
            }
        }
    };
    
    Menu menu(cfg);
    menu.show();
}

// ============= ВИЗУАЛИЗАЦИЯ =============
void run_visualization(const ConfigManager& cfg) {
    std::string simFile = cfg.output_dir() + "/" + cfg.output_file();
    std::string trajFile = cfg.output_dir() + "/" + cfg.trajectories_file();
    
    std::ifstream earth_file(simFile);
    std::ifstream venus_file(simFile);
    std::ifstream mars_file(simFile);
    std::ifstream jupiter_file(simFile);
    
    vector<Graphic_object*> bodies;
    
    Earth earth(earth_file, 6);
    bodies.push_back(&earth);
    Mars mars(mars_file, 8);
    bodies.push_back(&mars);
    Venus venus(venus_file, 4);
    bodies.push_back(&venus);
    Jupiter jupiter(jupiter_file, 10);
    bodies.push_back(&jupiter);
    Sun sun;
    bodies.push_back(&sun);
    
    std::ifstream asteroid_file(trajFile);
    if (asteroid_file.is_open()) {
        Asteroids* asteroids = new Asteroids(asteroid_file);
        bodies.push_back(asteroids);
        std::cout << "Загрузка астероидов завершена" << std::endl;
    }
    
    General_window g_window;
    g_window.event_processing(bodies);
    
    for (auto body : bodies) {
        if (dynamic_cast<Asteroids*>(body)) {
            delete body;
        }
    }
}