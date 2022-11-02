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

int main(int argc, char **argv)
{

	Simulation &sim = Simulation::get_instance();

	glfwSetErrorCallback(&Simulation::error_callback);
	if(!glfwInit()) return -1;

	sim.create_window("Springy",  "#version 460");

	glewExperimental = true;
	if(glewInit() != GLEW_OK) return -1;
	

	glGetError();
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	
	GLuint vaoId = 0;
	glGenVertexArrays(1, &vaoId);
	glBindVertexArray(vaoId);

	sim.init_programs();
	sim.init_camera();
	sim.set_scene();	
	while(!sim.window_closed()) {
		sim.input_capture();
		sim.move_camera();
		sim.fixed_timestep_update();
		sim.render_scene();
		sim.swap_buffers();
	}
	return 0;
}
