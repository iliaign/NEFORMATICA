#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigManager {
private:
    // Параметры симуляции
    double dt_ = 120.0;
    int total_days_ = 1460;
    int save_interval_hours_ = 24;
    std::string output_file_ = "simulation.csv";
    std::string trajectories_file_ = "trajectories.csv";
    std::string impact_stats_file_ = "impact_stats.csv";
    std::string output_dir_ = ".";
    
    // Флаги удаления планет (исправлено: remove_* вместо enable_*)
    bool remove_venus_ = false;    // <--- ИСПРАВЛЕНО
    bool remove_mars_ = false;     // <--- ИСПРАВЛЕНО
    bool remove_jupiter_ = false;  // <--- ИСПРАВЛЕНО
        
    // Параметры астероидов
    int num_asteroids_ = 100;
    double asteroid_speed_min_ = 20000.0;   // м/с
    double asteroid_speed_max_ = 40000.0;   // м/с

public:
    ConfigManager() = default;
    
    explicit ConfigManager(const std::string& filename) {
        load(filename);
    }
    
    void load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Warning: cannot open " << filename << ", using defaults.\n";
            return;
        }
        json data;
        try {
            file >> data;
        } catch (json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << "\n";
            return;
        }
        if (data.contains("dt")) dt_ = data["dt"];
        if (data.contains("total_days")) total_days_ = data["total_days"];
        if (data.contains("save_interval_hours")) save_interval_hours_ = data["save_interval_hours"];
        if (data.contains("output_file")) output_file_ = data["output_file"];
        if (data.contains("output_dir")) output_dir_ = data["output_dir"];
        if (data.contains("trajectories_file")) trajectories_file_ = data["trajectories_file"];
        if (data.contains("impact_stats_file")) impact_stats_file_ = data["impact_stats_file"];
        if (data.contains("remove_venus")) remove_venus_ = data["remove_venus"];
        if (data.contains("remove_mars")) remove_mars_ = data["remove_mars"];
        if (data.contains("remove_jupiter")) remove_jupiter_ = data["remove_jupiter"];
        if (data.contains("num_asteroids")) num_asteroids_ = data["num_asteroids"];
        if (data.contains("asteroid_speed_min")) asteroid_speed_min_ = data["asteroid_speed_min"];
        if (data.contains("asteroid_speed_max")) asteroid_speed_max_ = data["asteroid_speed_max"];
    }
    
    void save(const std::string& filename) const {
        json data;
        data["dt"] = dt_;
        data["total_days"] = total_days_;
        data["save_interval_hours"] = save_interval_hours_;
        data["output_file"] = output_file_;
        data["output_dir"] = output_dir_;
        data["trajectories_file"] = trajectories_file_;
        data["impact_stats_file"] = impact_stats_file_;
        data["remove_venus"] = remove_venus_;
        data["remove_mars"] = remove_mars_;
        data["remove_jupiter"] = remove_jupiter_;
        data["num_asteroids"] = num_asteroids_;
        data["asteroid_speed_min"] = asteroid_speed_min_;
        data["asteroid_speed_max"] = asteroid_speed_max_;
        std::ofstream file(filename);
        if (file) file << data.dump(4);
        else std::cerr << "Error saving config to " << filename << "\n";
    }
    
    // Геттеры
    double dt() const { return dt_; }
    int total_days() const { return total_days_; }
    int save_interval_hours() const { return save_interval_hours_; }
    std::string output_file() const { return output_file_; }
    std::string output_dir() const { return output_dir_; }
    std::string trajectories_file() const { return trajectories_file_; }
    std::string impact_stats_file() const { return impact_stats_file_; }
    bool remove_venus() const { return remove_venus_; }
    bool remove_mars() const { return remove_mars_; }
    bool remove_jupiter() const { return remove_jupiter_; }
    int num_asteroids() const { return num_asteroids_; }
    double asteroid_speed_min() const { return asteroid_speed_min_; }
    double asteroid_speed_max() const { return asteroid_speed_max_; }
    
    // Сеттеры для планет
    void set_remove_venus(bool b) { remove_venus_ = b; }
    void set_remove_mars(bool b) { remove_mars_ = b; }
    void set_remove_jupiter(bool b) { remove_jupiter_ = b; }
    
    // Сеттеры для симуляции
    void set_dt(double dt) { dt_ = dt; }
    void set_total_days(int days) { total_days_ = days; }
    void set_save_interval_hours(int hours) { save_interval_hours_ = hours; }
    void set_output_file(const std::string& path) { output_file_ = path; }
    void set_output_dir(const std::string& dir) { output_dir_ = dir; }
    void set_num_asteroids(int n) { num_asteroids_ = n; }
    void set_asteroid_speed_range(double vmin, double vmax) {
        asteroid_speed_min_ = vmin;
        asteroid_speed_max_ = vmax;
    }
};

#endif // CONFIG_H