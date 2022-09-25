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

struct position;
struct velocity;
struct color;

class Particles {
    public:
        Particles(); 
        ~Particles();

        void init(int max, int target, int initial, const std::shared_ptr<Program> compute_program);
        void load_particle_mesh();
        void buffer_world_geometry(const std::vector<std::shared_ptr<Object> >& objects);
        void buffer_attractors(std::vector<glm::vec4> attractors);
        void add_to_queue();
        void update_buffers();
        void update();
        void draw(const std::shared_ptr<Program>, const std::shared_ptr<Program>) const;

        int get_poly_count();
        glm::vec4 get_particle_count_data();

    private: 
        std::queue<int> queue;

        GLuint billboard_vertex_buffer;
        GLfloat g_vertex_buffer_data[12];
        int initial, current, target, max_amount, triangle_count, spawns_per_cycle;

        std::default_random_engine engine;
        std::normal_distribution<float> unit_normal;

        // struct color *colors;
        struct position *positions;
        // struct velocity *velocities;

        int current_particle;

        GLuint posSSbo, velSSbo, colSSbo, objSSbo, transSSbo, dataSSbo, attrSSbo;
};

#endif