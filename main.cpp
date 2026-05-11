#include "config.h"
#include "engine/engine.h"
#include "graphics/visualization.cpp"  // один файл содержит обе функции

int main() {
    ConfigManager cfg("config.json");
    
    // Запускаем меню
    run_menu(cfg);
    
    // Запускаем симуляцию
    SimulationEngine engine(cfg);
    engine.setup();
    engine.set_asteroid_params(cfg.num_asteroids(),
                               cfg.asteroid_speed_min(),
                               cfg.asteroid_speed_max());
    engine.generate_asteroids();
    engine.run();
    engine.save_all_results();
    
    // Запускаем визуализацию
    run_visualization(cfg);
    
    return 0;
}