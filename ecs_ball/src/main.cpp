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
	
	glfwSetErrorCallback(&Simulation::error_callback);

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

	glGetError();

	GLFWwindow * win = sim.get_window();

	// Set keyboard callback.
	glfwSetKeyCallback(win, &Simulation::key_callback);
	// Set char callback.
	glfwSetCharCallback(win, &Simulation::char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(win, &Simulation::cursor_position_callback);

	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(win, &Simulation::resize_callback);


	sim.init_program();
	sim.init_camera();

	Options options;
	// init program before the scene and camera too
	sim.set_scene(options);

	// ===================================================================
	Gui &gui = Gui::get_instance();

	if(gui.create_window("Parameters", glsl_version) == -1) {
		glfwTerminate();
		return -1;
	}	

	gui.set_options(options);

	// imgui parameters
	
	// Loop until the user closes the window.
	while(!sim.window_closed() && !gui.window_closed()) {
		
		// update physics until dt time passes, then renders
		sim.fixed_timestep_update();

		// update position and camera
		sim.move_camera();

		// Render scene.
		sim.render_scene();

		// Swap front and back buffers.
		sim.swap_buffers();
		
		//-----------------------------------------------------

		if (!gui.update()) {
			sim.set_scene(gui.get_options());
		}
	}

	glfwTerminate();
	return 0;
}
