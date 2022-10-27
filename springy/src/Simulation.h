#pragma once
#ifndef SIMULATION_H
#define SIMULATION_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "GLSL.h"
#include "Entity.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "abby.h"

class Simulation {
    public:
        Simulation();
        Simulation(Simulation const&);
        void operator=(Simulation const&);
        ~Simulation();

        void create_window(const char * window_name, const char * glsl_version);
        void init_programs();
        void init_camera();
        void set_scene();
        void render_scene();
        void move_camera();
        void fixed_timestep_update();
        
        void swap_buffers();
        bool window_closed();

        static Simulation& get_instance() {
            static Simulation instance; 
            return instance;
        }

        static void error_callback(int error, const char *description){
            get_instance().error_callback_impl(error, description);
        }

        void input_capture();
    private:
        void update(float dt);        

        void error_callback_impl(int error, const char *description);
        
        float   dt, current_time, total_time, new_time, frame_time, eps, movement_speed, sensitivity;

        glm::vec3 gravity, wind, lightPos;

        double o_x, o_y;                                    
        int width, height;                                  
        bool options[256], inputs[256];                 

        GLFWwindow *window;                             
        std::shared_ptr<Camera> camera;                     
        std::shared_ptr<Program> meshes_program, cloth_program, integration_compute, strut_compute, face_compute, particle_compute, cleanup_compute; 
        std::vector< std::shared_ptr<Entity> > entities;    
         
        std::shared_ptr<abby::tree<int,float>> box_tree;
};


#endif