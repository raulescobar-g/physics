#pragma once
#ifndef SIMULATION_H
#define SIMULATION_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "Program.h"
#include "Camera.h"
#include "Shape.h"
#include "Object.h"

class Simulation {
    public:
    Simulation();

    int create_window(std::string window_name);
    void create_scene();
    void reset();
    void render_scene(float dt);
    

    private:
    void error_callback(int error, const char *description);
    void char_callback(GLFWwindow * window, unsigned int key);
    void resize_callback(GLFWwindow *window, int width, int height);
    void cursor_position_callback(GLFWwindow *window, double xmouse, double ymouse);
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    float dt;
    double o_x, o_y;
    int width, height;
    string vert_shader_path, frag_shader_path;

    GLFWwindow *window; // Main application window
    shared_ptr<Camera> camera;
    shared_ptr<Program> program;
    vector< shared_ptr<Object> > objects;
    shared_ptr<Material> ball_material;
    shared_ptr<Material> wall_material;
    shared_ptr<Light> light;
    shared_ptr<Shape> ball;
    shared_ptr<Shape> wall;



}

#endif