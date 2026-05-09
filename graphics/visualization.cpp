#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>

struct Point { double x, y; };

void runVisualization(const std::string& trajectoryFile)
{
    // 1. Загружаем данные из CSV
    std::map<int, std::vector<Point>> tracks;
    std::vector<double> times;
    
    std::ifstream file(trajectoryFile);
    if (!file.is_open())
    {
        std::cerr << "Ошибка: не удалось открыть " << trajectoryFile << std::endl;
        return;
    }
    
    std::string header;
    std::getline(file, header); // "time,id,x,y"
    
    double t, x, y;
    int id;
    char comma;
    while (file >> t >> comma >> id >> comma >> x >> comma >> y)
    {
        if (times.empty() || times.back() != t)
            times.push_back(t);
        tracks[id].push_back({x, y});
    }
    file.close();
    
    if (tracks.empty() || times.empty())
    {
        std::cerr << "Нет данных в файле" << std::endl;
        return;
    }
    
    std::cout << "Загружено " << times.size() << " кадров, " << tracks.size() << " объектов" << std::endl;
    
    // 2. Настройки окна
    unsigned int screen_w = sf::VideoMode::getDesktopMode().width;
    unsigned int screen_h = sf::VideoMode::getDesktopMode().height;
    sf::RenderWindow window(sf::VideoMode(screen_w, screen_h), "Астероиды около Земли");
    
    sf::Vector2f center(screen_w / 2.0f, screen_h / 2.0f);
    const double scale = 200.0 / 8e12; // 200 пикселей = 800 000 км
    
    size_t current_frame = 0;
    sf::Clock animation_clock;
    sf::Time delay = sf::milliseconds(50);
    
    // 3. Основной цикл
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        // Обновление кадра
        if (animation_clock.getElapsedTime() >= delay)
        {
            current_frame = (current_frame + 1) % times.size();
            animation_clock.restart();
        }
        
        // Позиция Земли (id = -1) в текущем кадре
        Point earth_pos = {0.0, 0.0};
        auto it = tracks.find(-1);
        if (it != tracks.end() && current_frame < it->second.size())
            earth_pos = it->second[current_frame];
        
        // Отрисовка
        window.clear(sf::Color::Black);
        
        // Солнце (жёлтый круг в центре)
        sf::CircleShape sun(10);
        sun.setOrigin(10, 10);
        sun.setPosition(center);
        sun.setFillColor(sf::Color::Yellow);
        window.draw(sun);
        
        // Земля (синий круг тоже в центре)
        sf::CircleShape earth(10);
        earth.setOrigin(10, 10);
        earth.setPosition(center);
        earth.setFillColor(sf::Color::Blue);
        window.draw(earth);
        
        // Астероиды (все, кроме Земли)
        for (const auto& pair : tracks)
        {
            int obj_id = pair.first;
            const auto& points = pair.second;
            if (obj_id == -1) continue; // пропускаем Землю
            if (current_frame >= points.size()) continue;
            
            Point ast = points[current_frame];
            double dx = ast.x - earth_pos.x;
            double dy = ast.y - earth_pos.y;
            
            float sx = center.x + float(dx * scale);
            float sy = center.y + float(dy * scale);
            
            sf::CircleShape asteroid(2);
            asteroid.setOrigin(2, 2);
            asteroid.setPosition(sx, sy);
            asteroid.setFillColor(sf::Color::White);
            window.draw(asteroid);
        }
        
        window.display();
    }
}