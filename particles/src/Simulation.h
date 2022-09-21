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
#include "Shape.h"
#include "Object.h"
#include "MatrixStack.h"
#include "Particles.h"
#include "GLSL.h"


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Simulation {
    public:
        Simulation();
        Simulation(Simulation const&);
        void operator=(Simulation const&);
        ~Simulation();

        int create_window(const char * window_name, const char * glsl_version);
        void init_program();
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

        static void char_callback(GLFWwindow * window, unsigned int key){
            get_instance().char_callback_impl(window, key);
        }

        static void resize_callback(GLFWwindow *window, int width, int height){
            get_instance().resize_callback_impl(window, width, height);
        }

        static void cursor_position_callback(GLFWwindow *window, double xmouse, double ymouse){
            get_instance().cursor_position_callback_impl(window, xmouse, ymouse);
        }

        static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
            get_instance().key_callback_impl(window, key, scancode, action, mods);
        }

        GLFWwindow * get_window() {
            return window;
        } 
    
    private:
        Particles p_sys;

        std::shared_ptr<Shape> create_ball_shape(int res);
        std::shared_ptr<Shape> create_wall_shape();
        void update(float _dt);        

        void error_callback_impl(int error, const char *description);
        void char_callback_impl(GLFWwindow * window, unsigned int key);
        void resize_callback_impl(GLFWwindow *window, int width, int height);
        void cursor_position_callback_impl(GLFWwindow *window, double xmouse, double ymouse);
        void key_callback_impl(GLFWwindow *window, int key, int scancode, int action, int mods);

        
        float dt, current_time, total_time, new_time, frame_time, fps, eps, ball_size;

        glm::vec3 gravity, wind;

        double o_x, o_y;                                    // buffer for previous cursor positions
        int width, height;                                  // window details
        bool keyToggles[256], inputs[256];                  // user input storage

        float movement_speed, sensitivity;                  // camera movement speed, cursor sensitivity

        GLFWwindow *window;                                 // Main application window
        std::shared_ptr<Camera> camera;                     // application window
        std::shared_ptr<Program> program, particles_program; 
        std::vector< std::shared_ptr<Object> > objects;     // storage for all meshes + transforms + attributes
};

#endif