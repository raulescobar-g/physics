#pragma  once
#ifndef PARTICLES_H
#define PARTICLES_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <vector>
#include <memory>
#include <thread>
#include "atomic_queue/atomic_queue.h"
#include "Program.h"
#include "Object.h"

struct position;
struct velocity;
struct color;

using Element = uint32_t; 
Element constexpr NIL = static_cast<Element>(-1);
using Queue = atomic_queue::AtomicQueueB<Element, std::allocator<Element>, NIL>;

class Particles {
    public:
        Particles(); 
        ~Particles();

        void init(int max, int target, int initial, const std::shared_ptr<Program> compute_program);
        void load_particle_mesh();
        void buffer_world_geometry(const std::vector<std::shared_ptr<Object> >& objects);
        void buffer_attractors(std::vector<glm::vec4> attractors);
        void set_watcher();
        void set_reviver();
        void update();
        void draw(const std::shared_ptr<Program>, const std::shared_ptr<Program>) const;

        int get_poly_count();
        glm::vec4 get_particle_count_data();

    private: 
        Queue queue{1024};

        GLuint billboard_vertex_buffer;
        GLfloat g_vertex_buffer_data[12];
        int initial, current, target, max_amount, triangle_count;

        std::default_random_engine engine;
        std::normal_distribution<float> unit_normal;

        struct color *colors;
        struct position *positions;
        struct velocity *velocities;

        int current_particle;

        GLuint posSSbo, velSSbo, colSSbo, objSSbo, transSSbo, dataSSbo, attrSSbo;
};

#endif