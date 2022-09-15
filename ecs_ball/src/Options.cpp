#include "Options.h"


Options::Options() {
    dt = 1.0f/60.0f;
    box_size = 10.0f;
    light_pos = glm::vec3(box_size/2.0f - 0.5f, box_size/2.0f - 0.5f, box_size/2.0f - 0.5f);
    background_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    wind = glm::vec3(1.0f, 0.0f, 1.0f);

    ball_resolution = 20;
    ball_positions = { glm::vec3(0.0f, 0.0f, 0.0f) };
    ball_velocities = { glm::vec3(10.0f, 1.0f, 7.0f) };
    ball_sizes = { 1.5f };
    ball_restitutions = { 0.3f };
    ball_frictions = { 0.3f };
    ball_masses = { 1.0f };
    ball_drags = { 0.3f };
    ball_materials = { Material(glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.1f,0.2f,0.8f),glm::vec3(0.05f,0.95f,0.05f),20.0f) };
    wall_material = Material(glm::vec3(0.5f,0.5f,0.5f),glm::vec3(0.1f,0.1f,0.1f),glm::vec3(0.05f,0.05f,0.05f),500.0f);
}

float Options::get_dt() {return dt; }
void Options::set_dt(float _dt) {dt=_dt;}

float Options::get_box_size() {return box_size; }
void Options::set_box_size(float r) {box_size = r;}

glm::vec4 Options::get_background_color() {return background_color; }
glm::vec3 Options::get_light_pos() {return light_pos; }
float Options::get_ball_res(){return ball_resolution; }
int Options::ball_amount() {return ball_positions.size(); }
Material Options::get_ball_mat(int i) {return ball_materials[i]; }
Material Options::get_wall_material() {return wall_material; }

glm::vec3 Options::get_ball_pos(int i) {return ball_positions[i]; }
glm::vec3 Options::get_ball_velocity(int i){return ball_velocities[i]; }
float Options::get_ball_size(int i){return ball_sizes[i]; }
float Options::get_ball_res(int i){ return ball_restitutions[i]; }
float Options::get_ball_friction(int i) { return ball_frictions[i]; }
float Options::get_ball_mass(int i) { return ball_masses[i]; }
float Options::get_ball_drag(int i) { return ball_drags[i]; }

void Options::set_ball_pos(int i, glm::vec3 p){ ball_positions[i] = p; }
void Options::set_ball_velocity(int i, glm::vec3 v){ ball_velocities[i] = v; }
void Options::set_ball_size(int i, float s){ ball_sizes[i] = s; }
void Options::set_ball_res(int i, float res){ ball_restitutions[i] = res; }
void Options::set_ball_friction(int i, float friction) { ball_frictions[i] = friction; }
void Options::set_ball_mass(int i, float mass) { ball_masses[i] = mass; }
void Options::set_ball_drag(int i, float d) { ball_drags[i] = d; }

void Options::add_a_ball() {
    ball_positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    ball_velocities.push_back(glm::vec3(100.0f, 20.0f, 77.0f));
    ball_sizes.push_back(1.5f);
    ball_restitutions.push_back(0.3f);
    ball_frictions.push_back(0.3f);
    ball_masses.push_back(1.0f);
    ball_drags.push_back(0.3f);
    ball_materials.push_back(Material(glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.1f,0.2f,0.8f),glm::vec3(0.05f,0.95f,0.05f),20.0f));
}


glm::vec3 Options::get_gravity() {return gravity; }
glm::vec3 Options::get_wind() {return wind; }
void Options::set_gravity(glm::vec3 g) { gravity = g; }
void Options::set_wind(glm::vec3 w) { wind = w; }