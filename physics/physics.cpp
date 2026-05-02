#include "physics.h"

const double Msun = 1.98847e30;
const double G = 6.67430e-11;


State::State(double x, double y, double vx, double vy, double t)
    : x(x), y(y), vx(vx), vy(vy), t(t) {}

Gravity_field::Mass::Mass(double x, double y, double m) : x(x), y(y), mass(m) {}
Gravity_field::Point::Point(double gx, double gy) : gx(gx), gy(gy) {}

void Gravity_field::update_from_bodies(const vector<Cosmic_bodies*>& bodies) {
    masses.clear();
    for (auto body : bodies) {
        masses.push_back(Mass(body->getX(), body->getY(), body->getMass()));
    }
}

void Gravity_field::add_mass(double x, double y, double m) {
    masses.push_back(Mass(x, y, m));
}

void Gravity_field::clear_mass() { masses.clear(); }

Gravity_field::Point Gravity_field::calculating_field(double x, double y) const {
    double total_gx = 0, total_gy = 0;
    for (const Mass& m : masses) {
        double rx = m.x - x;
        double ry = m.y - y;
        double r = sqrt(rx * rx + ry * ry);
        if (r > 1e-10) {
            double factor = G * m.mass / (r * r * r);
            total_gx += factor * rx;
            total_gy += factor * ry;
        }
    }
    return Point(total_gx, total_gy);
}

void Gravity_field::pos_update(const vector<Cosmic_bodies*>& bodies) {
    update_from_bodies(bodies);
}

Cosmic_bodies::Cosmic_bodies() : mass(0), pos_x(0), pos_y(0), vel_x(0), vel_y(0) {}
Cosmic_bodies::Cosmic_bodies(double m, double x0, double y0, double vx0, double vy0)
    : mass(m), pos_x(x0), pos_y(y0), vel_x(vx0), vel_y(vy0) {}
Cosmic_bodies::~Cosmic_bodies() {}

double Cosmic_bodies::getMass() const { return mass; }
double Cosmic_bodies::getX() const { return pos_x; }
double Cosmic_bodies::getY() const { return pos_y; }
double Cosmic_bodies::getVx() const { return vel_x; }
double Cosmic_bodies::getVy() const { return vel_y; }
void Cosmic_bodies::setPosition(double x, double y) { pos_x = x; pos_y = y; }
void Cosmic_bodies::setVelocity(double vx, double vy) { vel_x = vx; vel_y = vy; }

void Cosmic_bodies::update(double dt, const Gravity_field& field) {
    Gravity_field::Point g = field.calculating_field(pos_x, pos_y);
    vel_x += g.gx * dt;
    vel_y += g.gy * dt;
    pos_x += vel_x * dt;
    pos_y += vel_y * dt;
}

Planet::Planet(double m, double x, double y, double vx, double vy) 
    : Cosmic_bodies(m, x, y, vx, vy) {}

Sun::Sun() : Cosmic_bodies(Msun, 0, 0, 0, 0) {}
void Sun::update(double dt, const Gravity_field& field) { /* Солнце неподвижно */ }

Mercury::Mercury(double x, double y, double vx, double vy, double m) : Planet(m, x, y, vx, vy) {}
Venus::Venus(double x, double y, double vx, double vy, double m) : Planet(m, x, y, vx, vy) {}
Earth::Earth(double x, double y, double vx, double vy, double m) : Planet(m, x, y, vx, vy) {}
Mars::Mars(double x, double y, double vx, double vy, double m) : Planet(m, x, y, vx, vy) {}
Jupiter::Jupiter(double x, double y, double vx, double vy, double m) : Planet(m, x, y, vx, vy) {}
Saturn::Saturn(double x, double y, double vx, double vy, double m) : Planet(m, x, y, vx, vy) {}
Uranus::Uranus(double x, double y, double vx, double vy, double m) : Planet(m, x, y, vx, vy) {}
Neptune::Neptune(double x, double y, double vx, double vy, double m) : Planet(m, x, y, vx, vy) {}

Spacecraft::Spacecraft(double m, double x, double y, double vx, double vy) 
    : Cosmic_bodies(m, x, y, vx, vy) {}
void Spacecraft::update(double dt, const Gravity_field& field) { Cosmic_bodies::update(dt, field); }