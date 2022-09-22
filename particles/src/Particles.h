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
#include "Compute.h"

class Particles {
    public:
    Particles() = default; 
    Particles(int amount);
    ~Particles();

    void init();
    void update(float _dt);
    void draw(const std::shared_ptr<Program> prog);


    private: 
    Compute compute;
    GLuint pid, billboard_vertex_buffer;
    GLfloat g_vertex_buffer_data[12];
    int amount, max_amount;
	std::map<std::string,GLint> attributes;
	std::map<std::string,GLint> uniforms;
	bool verbose;
};

#endif