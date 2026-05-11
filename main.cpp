#include "config.h"
#include "engine/engine.h"
#include "graphics/visualization.h"

// Импортируем физику (после того как добавили namespace Physics в physics.h)
using namespace Physics;

int main() {
    ConfigManager cfg("config.json");
    
    // 1. Меню выбора параметров
    run_menu(cfg);
    
    // 2. Симуляция
    SimulationEngine engine(cfg);
    engine.setup();
    engine.set_asteroid_params(cfg.num_asteroids(),
                               cfg.asteroid_speed_min(),
                               cfg.asteroid_speed_max());
    engine.generate_asteroids();
    engine.run();
    engine.save_all_results();
    
    // 3. Визуализация
    run_visualization(cfg);
    
    return 0;
}