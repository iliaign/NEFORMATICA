#include "config.h"
#include "engine/engine.h"
#include "graphics/menu.cpp"

// Объявляем функцию визуализации (определена в visualization.cpp)
void runVisualization(const std::string& trajectoryFile);

int main() {
    // 1. Загружаем конфиг
    ConfigManager cfg("config.json");

    // 2. Меню выбора скорости (изменяет cfg)
    Menu menu(cfg);
    menu.show();

    // 3. Симуляция
    SimulationEngine engine(cfg);
    engine.setup();
    engine.set_asteroid_params(cfg.num_asteroids(),
                               cfg.asteroid_speed_min(),
                               cfg.asteroid_speed_max());
    engine.generate_asteroids();
    engine.run();
    engine.save_all_results();

    // 4. Визуализация после симуляции
    std::string trajFile =  cfg.output_dir() +"/" + cfg.trajectories_file();
    runVisualization(trajFile);   

    return 0;
}