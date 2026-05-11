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

//десткутров класса симуляции
SimulationEngine::~SimulationEngine() {
    for (auto body : bodies_) delete body;
    if (traj_open_) traj_file_.close();
}
//инициализация планет солнечной системы
//геоцентричная орбита
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

//инициализация астероидов
//создаем определенное число штук
//в заданом диапазоне скоростей
void SimulationEngine::set_asteroid_params(int count, double vmin, double vmax) {
    num_asteroids_ = count;
    ast_vmin_ = vmin;
    ast_vmax_ = vmax;
}



//интегрирование элейра 2го порядка
//Не вынесено в физику тк добавление астероидов уже надстройка над симуляцией
//они не учавстуют в гравиттировании
//(точечная масса)


// Обновление астероидов с проверками
void SimulationEngine::update_asteroids() {
    const double MAX_SPEED = 3e12;        // скорость света - физический предел
    const double MAX_DISTANCE = 1e14;    // 100 млрд км (~670 а.е.)
    
    for (auto& ast : asteroids_) {
        if (!ast.alive) continue;
        
        // Проверка на NaN/Inf в координатах
        if (!isfinite(ast.x) || !isfinite(ast.y) || 
            !isfinite(ast.vx) || !isfinite(ast.vy)) {
            cout << "Asteroid " << ast.id << " removed: invalid state (NaN/Inf)" << endl;
            ast.alive = false;
            continue;
        }
        
        // Проверка на вылет из солнечной системы
        double dist_from_sun = hypot(ast.x, ast.y);
        if (dist_from_sun > MAX_DISTANCE) {
            cout << "Asteroid " << ast.id << " removed: escaped solar system (distance = " << dist_from_sun/1e9 << " million km)" << endl;
            ast.alive = false;
            continue;
        }
        
        // Вычисляем гравитационное поле
        Gravity_field::Point g = field_.calculating_field(ast.x, ast.y);
        
        // Проверка на NaN в гравитации
        if (!isfinite(g.gx) || !isfinite(g.gy)) {
            cout << "Asteroid " << ast.id << " removed: invalid gravity field" << endl;
            ast.alive = false;
            continue;
        }
        
        // Обновляем скорость и позицию (Эйлер 2-го порядка)
        ast.vx += g.gx * dt_;
        ast.vy += g.gy * dt_;
        
        // Проверка на превышение скорости
        double speed = hypot(ast.vx, ast.vy);
        if (speed > MAX_SPEED || !isfinite(speed)) {
            cout << "Asteroid " << ast.id << " removed: exceeded max speed (speed = " << speed << " m/s)" << endl;
            ast.alive = false;
            continue;
        }
        
        ast.x += ast.vx * dt_;
        ast.y += ast.vy * dt_;
    }
}

// Проверка столкновений с планетами
void SimulationEngine::check_collisions() {
    // Ищем индексы планет по массе (с относительным допуском)
    int earth_idx = -1, mars_idx = -1, jupiter_idx = -1, venus_idx = -1;
    const double rel_eps = 1e-3;  // 0.1% допуск
    
    for (size_t i = 0; i < bodies_.size(); ++i) {
        double m = bodies_[i]->getMass();
        
        if (abs(m - M_earth) < rel_eps * M_earth)
            earth_idx = i;
        else if (abs(m - M_mars) < rel_eps * M_mars)
            mars_idx = i;
        else if (abs(m - M_jupiter) < rel_eps * M_jupiter)
            jupiter_idx = i;
        else if (abs(m - M_venus) < rel_eps * M_venus)
            venus_idx = i;
    }
    
    if (earth_idx == -1) {
        cerr << "ERROR: Earth not found in bodies!" << endl;
        return;
    }
    
    // Проверяем каждый астероид
    for (auto& ast : asteroids_) {
        if (!ast.alive) continue;
        
        double dx, dy, dist;
        
        // Проверка столкновения с Землёй
        dx = ast.x - bodies_[earth_idx]->getX();
        dy = ast.y - bodies_[earth_idx]->getY();
        dist = hypot(dx, dy);
        if (dist < 2*R_earth) {
            impact_stats_.hits_earth++;
            cout << "Asteroid " << ast.id << " removed: hit Earth" << endl;
            ast.alive = false;
            continue;
        }
        
        // Проверка Марса (если найден)
        if (mars_idx != -1) {
            dx = ast.x - bodies_[mars_idx]->getX();
            dy = ast.y - bodies_[mars_idx]->getY();
            if (hypot(dx, dy) < R_mars) {
                impact_stats_.hits_mars++;
                cout << "Asteroid " << ast.id << " removed: hit Mars" << endl;
                ast.alive = false;
                continue;
            }
        }
        
        // Проверка Юпитера (если найден)
        if (jupiter_idx != -1) {
            dx = ast.x - bodies_[jupiter_idx]->getX();
            dy = ast.y - bodies_[jupiter_idx]->getY();
            if (hypot(dx, dy) < R_jupiter) {
                impact_stats_.hits_jupiter++;
                cout << "Asteroid " << ast.id << " removed: hit Jupiter" << endl;
                ast.alive = false;
                continue;
            }
        }
        
        // Проверка Венеры (если найдена)
        if (venus_idx != -1) {
            dx = ast.x - bodies_[venus_idx]->getX();
            dy = ast.y - bodies_[venus_idx]->getY();
            if (hypot(dx, dy) < R_venus) {
                impact_stats_.hits_venus++;
                cout << "Asteroid " << ast.id << " removed: hit Venus" << endl;
                ast.alive = false;
                continue;
            }
        }
        
        // Дополнительно: проверка столкновения с Солнцем
        if (dist < 6.957e8) {  // R_sun = 6.957e8 м
            cout << "Asteroid " << ast.id << " removed: hit Sun" << endl;
            ast.alive = false;  // Астероид упал на Солнце
        }
    }
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
    
    // Инициализация генератора случайных чисел
    mt19937 gen(chrono::steady_clock::now().time_since_epoch().count());
    uniform_real_distribution<> angle_dist(0.0, 2.0 * M_PI);
    uniform_real_distribution<> vel_angle_dist(0.0, 2.0 * M_PI);
    uniform_real_distribution<> vel_mag_dist(ast_vmin_, ast_vmax_);
    uniform_real_distribution<> rad_dist(1.0e8, 5.0e8);  
    
    asteroids_.clear();
    asteroids_.reserve(num_asteroids_);
    
    for (int i = 0; i < num_asteroids_; ++i) {
        // Случайная позиция вокруг Земли
        double theta = angle_dist(gen);
        double r = rad_dist(gen);
        double dx = r * cos(theta);
        double dy = r * sin(theta);
        
        // Случайное направление скорости
        double vel_angle = vel_angle_dist(gen);
        double vel_mag = vel_mag_dist(gen);
        double v_rel_x = vel_mag * cos(vel_angle);
        double v_rel_y = vel_mag * sin(vel_angle);
        
        // Итоговая скорость = скорость Земли + относительная
        double vx = v_rel_x;
        double vy = v_rel_y;
        
        asteroids_.push_back({
            1e12,                        // масса
            earth_x + dx,                // x
            earth_y + dy,                // y
            vx,                          // vx
            vy,                          // vy
            true,                        // alive
            i                            // id
        });
    }
    
    cout << "Сгенерировано " << num_asteroids_ << " астероидов" << endl;
    cout << "  Скорости: " << ast_vmin_ << " - " << ast_vmax_ << " м/с" << endl;
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
        traj_file_ << t_days << ",-1," << bodies_[3]->getX()/10e9 << "," << bodies_[3]->getY()/10e9  << "\n";
    for (const auto& ast : asteroids_) {
        if (ast.alive) {
            traj_file_ << t_days << "," << ast.id << "," << ast.x/10e9 << "," << ast.y/10e9 << "\n";
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