#pragma  once
#ifndef PARTICLES_H
#define PARTICLES_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <vector>
#include <memory>
#include <queue>
#include "Program.h"
#include "Object.h"
#include "Shape.h"

struct position;
struct velocity;
struct color;
struct density;
struct pressure;

class Boids {
    public:
        Boids(); 
        ~Boids();

        void init(int max);
        void load_particle_mesh();
        void buffer_world_geometry(const std::vector<std::shared_ptr<Object> >& objects);
        void update();
        void update_density();
        void draw(const std::shared_ptr<Program>) const;
        void spawn_boids();

        int get_poly_count();
        glm::vec4 get_display_data();

    private: 
        
        int initial, current, max_amount, triangle_count, spawns_per_cycle;

        std::default_random_engine engine;
        std::normal_distribution<float> unit_normal;

        struct color *colors;
        struct position *positions;
        struct velocity *velocities;
        struct acceleration *accelerations;
        struct density *densities;
        struct pressure *pressures;

        GLuint *atomic_counters;

        std::shared_ptr<Shape> particle_mesh;

        int current_particle, counters;

        GLuint  posSSbo, velSSbo, colSSbo, 
                objSSbo, transSSbo, dataSSbo, 
                atomicsBuffer, aabbSSbo, denSSbo, 
                accSSbo, preSSbo;
};

#endif