//Вероник, это твой минимально измененный файл

#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <cmath>

void show_trajectories(const std::string& filename = "trajectories.csv") {
    std::map<int, std::vector<std::pair<double, double>>> tracks;
    std::vector<double> times;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: " << filename << " not found.\n";
        return;
    }
    std::string line;
    std::getline(file, line); // header
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        double t, x, y;
        int id;
        char comma;
        ss >> t >> comma >> id >> comma >> x >> comma >> y;
        if (times.empty() || times.back() != t) times.push_back(t);
        tracks[id].push_back({x, y});
    }
    file.close();

    if (times.empty()) {
        std::cerr << "No data in " << filename << "\n";
        return;
    }

    size_t max_frames = times.size();

    sf::RenderWindow window(sf::VideoMode(1200, 900), "Asteroids near Earth");
    sf::CircleShape earth_shape(12);
    earth_shape.setFillColor(sf::Color::Blue);
    earth_shape.setOrigin(12, 12);

    sf::CircleShape asteroid_shape(2);
    asteroid_shape.setFillColor(sf::Color::White);
    asteroid_shape.setOrigin(2, 2);

    const double scale = 200.0 / 8e8;   // 200 px on 800 000 km
    const sf::Vector2f center(600, 450);

    sf::Clock clock;
    float frame_delay = 0.05f;
    size_t frame = 0;

    auto& earth_track = tracks[-1];
    if (earth_track.empty()) {
        std::cerr << "No Earth trajectory (id=-1)\n";
        return;
    }

    while (window.isOpen() && frame < max_frames) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (clock.getElapsedTime().asSeconds() >= frame_delay) {
            frame++;
            if (frame >= max_frames) frame = max_frames - 1;
            clock.restart();
        }

        window.clear(sf::Color::Black);
        earth_shape.setPosition(center);
        window.draw(earth_shape);

        double earth_x = earth_track[frame].first;
        double earth_y = earth_track[frame].second;

        for (auto& [id, points] : tracks) {
            if (id < 0) continue;
            if (frame >= points.size()) continue;
            double ax = points[frame].first;
            double ay = points[frame].second;
            double rx = ax - earth_x;
            double ry = ay - earth_y;
            sf::Vector2f screen(center.x + rx * scale, center.y + ry * scale);
            asteroid_shape.setPosition(screen);
            window.draw(asteroid_shape);
        }

        window.display();
    }
}