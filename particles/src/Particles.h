#pragma  once
#ifndef PARTICLES_H
#define PARTICLES_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <vector>
#include <memory>
#include "Program.h"

class Particles {
    public:
    Particles() = default; 
    Particles(int amount);
    ~Particles();

    void init();
    void update(float _dt);
    void draw(const std::shared_ptr<Program> prog);

    std::vector<float> positions;
    std::vector<float> velocities;
    std::vector<unsigned char> colors;
    std::vector<float> lifetimes;
    std::vector<float> masses;
    std::vector<float> sizes;

    std::normal_distribution<float> position_random;
    std::normal_distribution<float> velocity_random;
    std::normal_distribution<float> color_random;
    std::normal_distribution<float> lifetime_random;
    std::normal_distribution<float> mass_random;
    std::normal_distribution<float> size_random;

    std::default_random_engine engine;

    private: 
    GLuint pid, billboard_vertex_buffer, particles_position_buffer, particles_color_buffer, vaoId;
    GLfloat g_vertex_buffer_data[12];
    int amount, max_amount;
	std::map<std::string,GLint> attributes;
	std::map<std::string,GLint> uniforms;
	bool verbose;
};

#endif