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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int main(int argc, char **argv)
{

	Simulation &sim = Simulation::get_instance();

	glfwSetErrorCallback(&Simulation::error_callback);
	
	// Initialize the library.
	if(!glfwInit()) {
		std::cout<<"Error initing glfw"<<std::endl;
		return -1;
	}

	// const char* glsl_version = "#version 150";
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac


	if (sim.create_window("Particles") == -1) {
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
	int tmp;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp);
	std::cout << "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = " << tmp << std::endl;
	// Check how many uniforms are supported in the vertex shader
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &tmp);
	std::cout << "GL_MAX_VERTEX_UNIFORM_COMPONENTS = " << tmp << std::endl;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
	std::cout << "GL_MAX_VERTEX_ATTRIBS = " << tmp << std::endl;

	//GLSL::checkVersion();

	GLFWwindow * win = sim.get_window();

	
	glfwSetKeyCallback(win, &Simulation::key_callback);
	glfwSetCharCallback(win, &Simulation::char_callback);
	glfwSetCursorPosCallback(win, &Simulation::cursor_position_callback);
	glfwSetFramebufferSizeCallback(win, &Simulation::resize_callback);

	sim.init_program();
	sim.init_camera();
	sim.set_scene();	
	

	while(!sim.window_closed()) {
		sim.fixed_timestep_update();
		sim.move_camera();
		sim.render_scene();
		sim.swap_buffers();
	}

	glfwTerminate();
	return 0;
}
