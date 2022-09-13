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

#include <entt/entt.hpp>

#include "GLSL.h"
#include "Simulation.h"
#include "Options.h"
#include "Gui.h"

int main(int argc, char **argv)
{
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}

	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac


	Simulation &sim = Simulation::get_instance();
	Gui &gui = Gui::get_instance();
	Options options;

	if (sim.create_window("Ball in a box") == -1) {
		std::cout<<"Error creating simulation window."<<std::endl;
		return -1;
	}

	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	GLFWwindow * win = sim.get_window();

	glfwSetErrorCallback(&Simulation::error_callback);
	glfwSetKeyCallback(win, &Simulation::key_callback);
	glfwSetCharCallback(win, &Simulation::char_callback);
	glfwSetCursorPosCallback(win, &Simulation::cursor_position_callback);
	glfwSetFramebufferSizeCallback(win, &Simulation::resize_callback);

	sim.init_program();
	sim.init_camera();
	sim.set_scene(options);	

	if(gui.create_window("Parameters", glsl_version) == -1) {
		std::cout<<"Error creating gui window"<<std::endl;
		glfwTerminate();
		return -1;
	}	

	gui.set_options(options);
	

	while(!sim.window_closed() && !gui.window_closed()) {
		
		sim.fixed_timestep_update();

		sim.move_camera();

		sim.render_scene();

		sim.swap_buffers();
		
		//-----------------------------------------------------

		if (!gui.update()) {
			sim.set_scene(gui.get_options());
		}
	}

	glfwTerminate();
	return 0;
}
