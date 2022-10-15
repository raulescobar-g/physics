#pragma  once
#ifndef STRUTS_H
#define STRUTS_H

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

class Struts {
    public:
        Struts(); 
        ~Struts();

        void init(int max, int spawns, int initial, const std::shared_ptr<Program> compute_program);
        void load_struts_mesh();
        void buffer_world_geometry(const std::vector<std::shared_ptr<Object> >& objects);
        void update();
        void draw(const std::shared_ptr<Program>, const std::shared_ptr<Program>) const;

        int get_poly_count();
        glm::vec4 get_particle_count_data();

    private: 
        std::queue<int> queue;

        GLuint billboard_vertex_buffer;
        GLfloat g_vertex_buffer_data[12];
        int initial, current, max_amount, triangle_count, spawns_per_cycle;

        std::default_random_engine engine;
        std::normal_distribution<float> unit_normal;

        struct color *colors;
        struct position *positions;
        struct velocity *velocities;
        GLuint *atomic_counters;

        int current_particle, counters;

        GLuint posSSbo, velSSbo, colSSbo, objSSbo, transSSbo, dataSSbo, attrSSbo, atomicsBuffer;
};

#endif