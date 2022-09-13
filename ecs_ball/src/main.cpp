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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"
#include "Object.h"
#include "Simulation.h"

GLFWwindow *gui_window; // Main application window


int main(int argc, char **argv)
{
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}

	#if defined(IMGUI_IMPL_OPENGL_ES2)
		// GL ES 2.0 + GLSL 100
		const char* glsl_version = "#version 100";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	#elif defined(__APPLE__)
		// GL 3.2 + GLSL 150
		const char* glsl_version = "#version 150";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
	#else
		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
	#endif

	// ===================================================================
	Simulation &sim = Simulation::get_instance();
	
	glfwSetErrorCallback(&Simulation::error_callback);

	if (sim.create_window("Ball in a box") == -1) {
		std::cout<<"Error creating simulation window."<<std::endl;
		return -1;
	}
	

	GLFWwindow * win = sim.get_window();

	// Set keyboard callback.
	glfwSetKeyCallback(win, &Simulation::key_callback);
	// Set char callback.
	glfwSetCharCallback(win, &Simulation::char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(win, &Simulation::cursor_position_callback);

	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(win, &Simulation::resize_callback);
	
	sim.create_scene();
	// ===================================================================
	
	gui_window = glfwCreateWindow(640, 480, "Parameters", NULL, NULL);
	if(!gui_window) {
		glfwTerminate();
		return -1;
	}	
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(gui_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	

	

	// imgui parameters
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	

	// Loop until the user closes the window.
	while(!sim.window_closed() && !glfwWindowShouldClose(gui_window)) {
		
		// update physics until dt time passes, then renders
		sim.fixed_timestep_update();

		// update position and camera
		sim.move_camera();

		// Render scene.
		sim.render_scene();

		// Swap front and back buffers.
		sim.swap_buffers();
		
		//------------------------------------------------------
		//------------------------------------------------------
		//------------------------------------------------------
		//------------------------------------------------------
		//------------------------------------------------------
		//------------------------------------------------------
		//------------------------------------------------------
		//------------------------------------------------------
		//------------------------------------------------------
		//------------------------------------------------------

		
		glfwMakeContextCurrent(gui_window);
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Parameters");                        

		ImGui::Text("Edit the parameters and reset the simulation.");

		ImGui::SliderFloat("initial x position", &f, 0.0f, 100.0f);  // make this radius dependant
		ImGui::SliderFloat("initial y position", &f, 0.0f, 100.0f);
		ImGui::SliderFloat("initial y position", &f, 0.0f, 100.0f);

		ImGui::SliderFloat("initial x velocity", &f, 0.0f, 100.0f); // cap it at something 
		ImGui::SliderFloat("initial y velocity", &f, 0.0f, 100.0f);
		ImGui::SliderFloat("initial y velocity", &f, 0.0f, 100.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Restart"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);
		float fps = 1.0f;
		ImGui::Text("Application (%.1f FPS)", fps);
		ImGui::End();
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(gui_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(gui_window);
		
	}

	glfwMakeContextCurrent(gui_window);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(gui_window);

	sim.end();
	return 0;
}
