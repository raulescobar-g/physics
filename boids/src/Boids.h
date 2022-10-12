#pragma  once
#ifndef BOIDS_H
#define BOIDS_H

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
struct cell;

class Boids {
    public:
        Boids(); 
        ~Boids();

        void init(int max, int _predators, int _dups, int _dims, float space);
        void load_boid_mesh();
        void buffer_world_geometry(const std::vector<std::shared_ptr<Object> >& objects);
        void update();
        void draw_boids(const std::shared_ptr<Program>) const;
        void draw_predators(const std::shared_ptr<Program>) const;
        void spawn_boids();

        int get_poly_count();
        glm::vec4 get_display_data();

    private: 
        GLuint  posSSbo, velSSbo, colSSbo, objSSbo, transSSbo, 
                dataSSbo, atomicsBuffer, aabbSSbo, pred_posSSbo, 
                pred_velSSbo, pred_colSSbo, gridSSbo;

        int max_amount, current_particle, counters, triangle_count, 
            spawns_per_cycle, predators, dims, dups;

        float spacing;

        std::default_random_engine engine;
        std::normal_distribution<float> unit_normal;

        struct color *colors;
        struct position *positions;
        struct velocity *velocities;
        struct color *predator_colors;
        struct position *predator_positions;
        struct velocity *predator_velocities;
        struct cell *grid;

        GLuint *atomic_counters;

        std::shared_ptr<Shape> boid_mesh, predator_mesh;
};

#endif