#ifndef ENGINE_H
#define ENGINE_H

#include "../physics/physics.h"
#include "../config.h"
#include <vector>
#include <string>
#include <fstream>

using namespace Physics;  // <--- ДОБАВИТЬ ЭТУ СТРОКУ

class SimulationEngine {
public:
    SimulationEngine();
    explicit SimulationEngine(const ConfigManager& cfg);
    ~SimulationEngine();
    
    void setup();
    void set_asteroid_params(int count, double vmin, double vmax);
    void generate_asteroids();
    void run();
    void save_csv(const std::string& filename = "");
    void save_all_results();

private:
    struct Asteroid {
        double mass, x, y, vx, vy;
        bool alive;
        int id;
    };
    
    std::vector<Cosmic_bodies*> bodies_;
    Gravity_field field_;
    double dt_, total_seconds_, save_interval_seconds_, current_time_;
    std::string output_dir_;
    std::string output_file_;
    std::string trajectories_file_;
    std::string impact_stats_file_;
    
    std::vector<Asteroid> asteroids_;
    int num_asteroids_;
    double ast_vmin_, ast_vmax_;
    
    struct ImpactStats { int hits_earth=0, hits_mars=0, hits_jupiter=0, hits_venus=0; } impact_stats_;
    std::ofstream traj_file_;
    double last_save_time_;
    bool traj_open_;
    
    std::vector<double> times_;
    std::vector<std::vector<double>> planets_x_, planets_y_;
    
    void init_planets();
    void update_asteroids();
    void check_collisions();
    void save_trajectory(double t_days);
    void save_impact_stats();
};

#endif