#pragma once
#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"

#include <entt/entt.hpp>

class Program;
class Mesh;
class State;
class Material;
class Entity;
class Camera;
class Texture;
class MatrixStack;

class Simulation {
    public:
        Simulation();
        Simulation(Simulation const&);
        void operator=(Simulation const&);
        ~Simulation();

        void create_window(const char * window_name, const char * glsl_version);
        void init_programs();
        void init_cameras();
        void set_scene();
        void render_scene();
        void move_camera();
        void fixed_timestep_update();
        void resize_window();
        void swap_buffers();
        bool window_closed();
        void input_capture();
        void look_around();
        

        static Simulation& get_instance() {
            static Simulation instance; 
            return instance;
        }

        static void error_callback(int error, const char *description){
            get_instance().error_callback_impl(error, description);
        }

        
    private:
        void update();        
        std::vector<State> integrate(std::vector<State> state, float h);
        bool are_colliding(Entity& ent1, Entity& ent2, glm::vec4 response);
        void draw_entities(std::vector<Entity>& drawables, std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P);
        void error_callback_impl(int error, const char *description);
        
        float   dt = 1.0f/144.0f, 
                current_time, 
                total_time, 
                new_time, 
                frame_time, 
                movement_speed = 0.5f,
                sensitivity = 0.005f,
                eps = 0.01f;

        glm::vec3   lightPos = glm::vec3(0.0f, 30.0f, 0.0f),
                    gravity = glm::vec3(0.0f, -0.1f, 0.0f),
                    wind = glm::vec3(1.0f, 0.0f, 1.0f);

        double  o_x= -1.0, 
                o_y= -1.0;                                    
        bool options[256], inputs[256];                 

        GLFWwindow *window;   

        std::vector<Camera> cameras;   
        std::vector<Program> programs;
        std::vector<Entity> dynamic_entities;
        std::vector<Entity> static_entities;
        std::vector<Material> materials;
        std::vector<State> states;
        std::vector<Mesh> meshes;   
        //std::vector<Texture> textures;      
};


#endif