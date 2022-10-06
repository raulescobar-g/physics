#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdlib.h>  
#include <time.h> 
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSL.h"
#include "Simulation.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <chrono>

int main(int argc, char **argv)
{

	Simulation &sim = Simulation::get_instance();

	glfwSetErrorCallback(&Simulation::error_callback);
	
	// Initialize the library.
	if(!glfwInit()) {
		std::cout<<"Error initing glfw"<<std::endl;
		return -1;
	}

	const char* glsl_version = "#version 330";


	if (sim.create_window("Particles", glsl_version) == -1) {
		std::cout<<"Error creating simulation window."<<std::endl;
		return -1;
	}

	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}


	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	
	GLFWwindow * win = sim.get_window();

	
	glfwSetKeyCallback(win, &Simulation::key_callback);
	glfwSetCharCallback(win, &Simulation::char_callback);
	glfwSetCursorPosCallback(win, &Simulation::cursor_position_callback);
	glfwSetFramebufferSizeCallback(win, &Simulation::resize_callback);

	GLuint vaoId = 0;
	glGenVertexArrays(1, &vaoId);
	glBindVertexArray(vaoId);

	sim.init_programs();
	sim.init_camera();
	sim.set_scene();	

	while(!sim.window_closed()) {
		sim.move_camera();
		sim.fixed_timestep_update();
		sim.render_scene();
		sim.swap_buffers();
	}

	return 0;
}
