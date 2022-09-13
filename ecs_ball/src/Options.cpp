#include "Options.h"


Options::Options() {
    dt = 1.0f/144.0f;
    box_size = 10.0f;
    light_pos = glm::vec3(box_size/2.0f - 0.5f, box_size/2.0f - 0.5f, box_size/2.0f - 0.5f);
    background_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    glm::vec3 wind = glm::vec3(1.0f, 0.0f, 1.0f);

    ball_resolution = 20;
    ball_positions = { glm::vec3(0.0f, 0.0f, 0.0f) };
    ball_velocities = { glm::vec3(100.0f, 20.0f, 77.0f) };
    ball_sizes = { 1.5f };
    ball_materials = { Material(glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.1f,0.2f,0.8f),glm::vec3(0.05f,0.95f,0.05f),20.0f) };
    wall_material = Material(glm::vec3(0.5f,0.5f,0.5f),glm::vec3(0.1f,0.1f,0.1f),glm::vec3(0.05f,0.05f,0.05f),500.0f);
}

float Options::get_dt() {return dt; }
float Options::get_box_size() {return box_size; }
glm::vec4 Options::get_background_color() {return background_color; }
glm::vec3 Options::get_light_pos() {return light_pos; }
float Options::get_ball_res(){return ball_resolution; }
int Options::ball_amount() {return ball_positions.size(); }
Material Options::get_ball_mat(int i) {return ball_materials[i]; }
Material Options::get_wall_material() {return wall_material; }

glm::vec3 Options::get_ball_pos(int i) {return ball_positions[i]; }
glm::vec3 Options::get_ball_velocity(int i){return ball_velocities[i]; }
float Options::get_ball_size(int i){return ball_sizes[i]; }


glm::vec3 Options::get_gravity() {return gravity; }
glm::vec3 Options::get_wind() {return wind; }