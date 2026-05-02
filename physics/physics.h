#ifndef PHYSICS_H
#define PHYSICS_H

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

using std::cout;
using std::endl;
using std::vector;
using std::sqrt;
using std::string;

extern const double Msun;
extern const double G;

struct State {
    double x, y, vx, vy, t;
    State(double x = 0, double y = 0, double vx = 0, double vy = 0, double t = 0);
};

class Cosmic_bodies;

class Gravity_field {
private:
    struct Mass {
        double x, y, mass;
        Mass(double x, double y, double m);
    };
    vector<Mass> masses;
    
public:
    void update_from_bodies(const vector<Cosmic_bodies*>& bodies);
    void add_mass(double x, double y, double m);
    void clear_mass();
    
    struct Point {
        double gx, gy;
        Point(double gx = 0, double gy = 0);
    };
    
    Point calculating_field(double x, double y) const;
    void pos_update(const vector<Cosmic_bodies*>& bodies);
};

class Cosmic_bodies {
protected:
    double mass;
    double pos_x, pos_y;
    double vel_x, vel_y;
    
public:
    Cosmic_bodies();
    Cosmic_bodies(double m, double x0, double y0, double vx0, double vy0);
    virtual ~Cosmic_bodies();

    double getMass() const;
    double getX() const;
    double getY() const;
    double getVx() const;
    double getVy() const;

    void setPosition(double x, double y);
    void setVelocity(double vx, double vy);
    
    virtual void update(double dt, const Gravity_field& field);
};

class Planet : public Cosmic_bodies {
public:
    Planet(double m, double x, double y, double vx, double vy);
};

class Sun : public Cosmic_bodies {
public:
    Sun();
    void update(double dt, const Gravity_field& field) override;
};

class Mercury : public Planet {
public:
    Mercury(double x, double y, double vx, double vy, double m);
};

class Venus : public Planet {
public:
    Venus(double x, double y, double vx, double vy, double m);
};

class Earth : public Planet {
public:
    Earth(double x, double y, double vx, double vy, double m);
};

class Mars : public Planet {
public:
    Mars(double x, double y, double vx, double vy, double m);
};

class Jupiter : public Planet {
public:
    Jupiter(double x, double y, double vx, double vy, double m);
};

class Saturn : public Planet {
public:
    Saturn(double x, double y, double vx, double vy, double m);
};

class Uranus : public Planet {
public:
    Uranus(double x, double y, double vx, double vy, double m);
};

class Neptune : public Planet {
public:
    Neptune(double x, double y, double vx, double vy, double m);
};

class Spacecraft : public Cosmic_bodies {
public:
    Spacecraft(double m, double x, double y, double vx, double vy);
    void update(double dt, const Gravity_field& field) override;
};

#endif