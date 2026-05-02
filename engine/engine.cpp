#include "engine.h"
#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <cassert>

using namespace std;

// Константы масс и радиусов планет
const double M_earth  = 5.9722e24;
const double M_mars   = 6.4171e23;
const double M_jupiter= 1.898e27;
const double M_venus  = 4.8675e24;

const double R_earth  = 6.371e6;
const double R_mars   = 3.3895e6;
const double R_jupiter= 6.9911e7;
const double R_venus  = 6.0518e6;

static double orbital_speed(double r) {
    return sqrt(G * Msun / r);
}

// Конструкторы
//по умолчанию
SimulationEngine::SimulationEngine()
    : dt_(120.0),
      total_seconds_(4 * 365 * 24 * 3600),
      save_interval_seconds_(24 * 3600),
      current_time_(0.0),
      output_dir_("."),
      trajectories_file_("trajectories.csv"),
      impact_stats_file_("impact_stats.csv"),
      num_asteroids_(0),
      ast_vmin_(0), ast_vmax_(0),
      last_save_time_(0.0),
      traj_open_(false) {
    cout << "SimulationEngine: параметры по умолчанию" << endl;
}

//с конфига
SimulationEngine::SimulationEngine(const ConfigManager& cfg)
    : dt_(cfg.dt()),
      total_seconds_(cfg.total_days() * 24 * 3600),
      save_interval_seconds_(cfg.save_interval_hours() * 3600),
      current_time_(0.0),
      output_dir_(cfg.output_dir()),
      output_file_(cfg.output_file()),      
      trajectories_file_(cfg.trajectories_file()),
      impact_stats_file_(cfg.impact_stats_file()),
      num_asteroids_(0),
      ast_vmin_(0), ast_vmax_(0),
      last_save_time_(0.0),
      traj_open_(false) { 
    cout << "SimulationEngine: загружена конфигурация" << endl;
    cout << "  dt = " << dt_ << " c" << endl;
    cout << "  total days = " << cfg.total_days() << endl;
    cout << "  save interval = " << cfg.save_interval_hours() << " h" << endl;
    cout << "  output dir = " << output_dir_ << endl;
    cout << "  trajectories file = " << trajectories_file_ << endl;
    cout << "  impact stats file = " << impact_stats_file_ << endl;
}

SimulationEngine::~SimulationEngine() {
    for (auto body : bodies_) delete body;
    if (traj_open_) traj_file_.close();
}

void SimulationEngine::init_planets() {
    // Солнце
    bodies_.push_back(new Sun());
    
    double r_merc = 5.790e10;
    bodies_.push_back(new Mercury(r_merc, 0, 0, orbital_speed(r_merc), 3.285e23));
    double r_ven = 1.082e11;
    bodies_.push_back(new Venus(r_ven, 0, 0, orbital_speed(r_ven), 4.867e24));
    double r_earth = 1.496e11;
    bodies_.push_back(new Earth(r_earth, 0, 0, orbital_speed(r_earth), 5.972e24));
    double r_mars = 2.279e11;
    bodies_.push_back(new Mars(r_mars, 0, 0, orbital_speed(r_mars), 6.417e23));
    double r_jup = 7.786e11;
    bodies_.push_back(new Jupiter(r_jup, 0, 0, orbital_speed(r_jup), 1.898e27));
    double r_sat = 1.434e12;
    bodies_.push_back(new Saturn(r_sat, 0, 0, orbital_speed(r_sat), 5.683e26));
    double r_uran = 2.871e12;
    bodies_.push_back(new Uranus(r_uran, 0, 0, orbital_speed(r_uran), 8.681e25));
    double r_nept = 4.495e12;
    bodies_.push_back(new Neptune(r_nept, 0, 0, orbital_speed(r_nept), 1.024e26));
    
    cout << "Солнечная система инициализирована" << endl;
}

void SimulationEngine::setup() {
    init_planets();
}

// --------------------------------------------------------------
// Астероиды
// --------------------------------------------------------------
void SimulationEngine::set_asteroid_params(int count, double vmin, double vmax) {
    num_asteroids_ = count;
    ast_vmin_ = vmin;
    ast_vmax_ = vmax;
}

void SimulationEngine::generate_asteroids() {
    if (bodies_.size() < 4) {
        cerr << "Ошибка: Земля ещё не создана. Вызовите setup() сначала." << endl;
        return;
    }
    double earth_x = bodies_[3]->getX();
    double earth_y = bodies_[3]->getY();
    double earth_vx = bodies_[3]->getVx();
    double earth_vy = bodies_[3]->getVy();
    
    mt19937 gen(chrono::steady_clock::now().time_since_epoch().count());
    uniform_real_distribution<> angle_dist(0, 2 * M_PI);
    //подгружаем с файла минимальную и максимальную скорость
    uniform_real_distribution<> vel_dist(ast_vmin_, ast_vmax_);
    uniform_real_distribution<> rad_dist(3.844e8, 4.0e9);
    
    asteroids_.clear();
    //клво тоже подгружаем
    asteroids_.reserve(num_asteroids_);
    for (int i = 0; i < num_asteroids_; ++i) {
        double angle = angle_dist(gen);
        double r = rad_dist(gen);
        double dx = r * cos(angle);
        double dy = r * sin(angle);
        double vx = earth_vx + vel_dist(gen);
        double vy = earth_vy + vel_dist(gen);
        asteroids_.push_back({1e12, earth_x + dx, earth_y + dy, vx, vy, true, i});
    }
    cout << "Сгенерировано астероидов: " << num_asteroids_ << endl;
}

void SimulationEngine::update_asteroids() {
    for (auto& ast : asteroids_) {
        if (!ast.alive) continue;
        Gravity_field::Point g = field_.calculating_field(ast.x, ast.y);
        ast.vx += g.gx * dt_;
        ast.vy += g.gy * dt_;
        ast.x += ast.vx * dt_;
        ast.y += ast.vy * dt_;
    }
}

void SimulationEngine::check_collisions() {
    int earth_idx = -1, mars_idx = -1, jupiter_idx = -1, venus_idx = -1;
    for (size_t i = 0; i < bodies_.size(); ++i) {
        double m = bodies_[i]->getMass();
        if (m == M_earth) earth_idx = i;
        else if (m == M_mars) mars_idx = i;
        else if (m == M_jupiter) jupiter_idx = i;
        else if (m == M_venus) venus_idx = i;
    }
    if (earth_idx == -1) return;
    
    for (auto& ast : asteroids_) {
        if (!ast.alive) continue;
        double dx, dy;
        dx = ast.x - bodies_[earth_idx]->getX();
        dy = ast.y - bodies_[earth_idx]->getY();
        if (hypot(dx, dy) < R_earth) {
            impact_stats_.hits_earth++;
            ast.alive = false;
            continue;
        }
        if (mars_idx != -1) {
            dx = ast.x - bodies_[mars_idx]->getX();
            dy = ast.y - bodies_[mars_idx]->getY();
            if (hypot(dx, dy) < R_mars) {
                impact_stats_.hits_mars++;
                ast.alive = false;
                continue;
            }
        }
        if (jupiter_idx != -1) {
            dx = ast.x - bodies_[jupiter_idx]->getX();
            dy = ast.y - bodies_[jupiter_idx]->getY();
            if (hypot(dx, dy) < R_jupiter) {
                impact_stats_.hits_jupiter++;
                ast.alive = false;
                continue;
            }
        }
        if (venus_idx != -1) {
            dx = ast.x - bodies_[venus_idx]->getX();
            dy = ast.y - bodies_[venus_idx]->getY();
            if (hypot(dx, dy) < R_venus) {
                impact_stats_.hits_venus++;
                ast.alive = false;
                continue;
            }
        }
    }
}

// Сохранение результатов
void SimulationEngine::save_trajectory(double t_days) {
    if (!traj_open_) {
        string path = output_dir_ + "/" + trajectories_file_;
        traj_file_.open(path);
        if (!traj_file_.is_open()) {
            cerr << "Не удалось открыть " << path << endl;
            return;
        }
        traj_file_ << "time,id,x,y\n";
        traj_open_ = true;
    }
    if (bodies_.size() > 3)
        traj_file_ << t_days << ",-1," << bodies_[3]->getX() << "," << bodies_[3]->getY() << "\n";
    for (const auto& ast : asteroids_) {
        if (ast.alive) {
            traj_file_ << t_days << "," << ast.id << "," << ast.x << "," << ast.y << "\n";
        }
    }
}

void SimulationEngine::save_impact_stats() {
    string path = output_dir_ + "/" + impact_stats_file_;
    ofstream file(path);
    if (!file) {
        cerr << "Не удалось записать " << path << endl;
        return;
    }
    int missed = 0;
    for (const auto& ast : asteroids_) if (ast.alive) missed++;
    file << "Config,ImpactsOnEarth,ImpactsOnMars,ImpactsOnJupiter,ImpactsOnVenus,Missed\n";
    file << "simulation,"
         << impact_stats_.hits_earth << ","
         << impact_stats_.hits_mars << ","
         << impact_stats_.hits_jupiter << ","
         << impact_stats_.hits_venus << ","
         << missed << "\n";
    file.close();
    cout << "Статистика сохранена в " << path << endl;
}

void SimulationEngine::save_csv(const string& filename) {
    string actual = filename.empty() ? output_dir_ + "/" + output_file_ : filename;
    ofstream file(actual);
    if (!file) {
        cerr << "Ошибка создания " << actual << endl;
        return;
    }
    vector<string> names = {"Mercury","Venus","Earth","Mars","Jupiter","Saturn","Uranus","Neptune"};
    file << "t";
    for (const auto& n : names) file << "," << n << "_x," << n << "_y";
    file << "\n";
    for (size_t i = 0; i < times_.size(); ++i) {
        file << times_[i];
        for (size_t j = 0; j < planets_x_.size(); ++j)
            file << "," << planets_x_[j][i] << "," << planets_y_[j][i];
        file << "\n";
    }
    file.close();
    cout << "CSV с планетами сохранён в " << actual << endl;
}

void SimulationEngine::save_all_results() {
    save_csv();
    save_impact_stats();
}

// ЯДРО СИМУЛЯЦИИ 
// основной цикл
void SimulationEngine::run() {
    cout << "Запуск симуляции..." << endl;
    times_.clear();
    planets_x_.assign(bodies_.size() - 1, vector<double>());
    planets_y_.assign(bodies_.size() - 1, vector<double>());
    last_save_time_ = 0.0;
    traj_open_ = false;
    
    for (current_time_ = 0; current_time_ <= total_seconds_; current_time_ += dt_) {
        field_.update_from_bodies(bodies_);
        for (size_t i = 1; i < bodies_.size(); ++i)
            bodies_[i]->update(dt_, field_);
        update_asteroids();
        check_collisions();
        
        double t_days = current_time_ / (24.0 * 3600.0);
        if (current_time_ - last_save_time_ >= save_interval_seconds_ - 1e-9) {
            save_trajectory(t_days);
            times_.push_back(t_days);
            for (size_t i = 1; i < bodies_.size(); ++i) {
                planets_x_[i-1].push_back(bodies_[i]->getX() / 1e9);
                planets_y_[i-1].push_back(bodies_[i]->getY() / 1e9);
            }
            last_save_time_ = current_time_;
        }
    }
    if (traj_open_) traj_file_.close();
    save_impact_stats();
    cout << "Симуляция завершена." << endl;
}