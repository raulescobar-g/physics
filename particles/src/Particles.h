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
#include "Object.h"

struct position;
struct velocity;
struct color;
struct triangle;

class Particles {
    public:
        Particles(); 
        ~Particles();

        void init(int max_amount, const std::shared_ptr<Program> compute_program);
        void load_particle_mesh();
        void buffer_world_geometry(const std::vector<std::shared_ptr<Object> >& objects);
        // void detach_spawner();
        void update();
        void draw(const std::shared_ptr<Program>, const std::shared_ptr<Program>) const;

        int get_poly_count();

    private: 
        GLuint billboard_vertex_buffer;
        GLfloat g_vertex_buffer_data[12];
        int amount, max_amount, triangle_count;

        std::default_random_engine engine;
        std::normal_distribution<float> unit_normal;

        struct color *colors;
        struct position *positions;
        struct velocity *velocities;
        struct triangle *triangles;

        GLuint posSSbo, velSSbo, colSSbo, objSSbo, transSSbo, dataSSbo;
};

#endif