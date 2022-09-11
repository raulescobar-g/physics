#include "Simulation.h"

Simulation::Simulation() {
    o_x = 0.0f;
    o_y = 0.0f;

    vert_shader_path = "../resources/phong_vert.glsl";
    frag_shader_path = "../resources/phong_frag.glsl";

}

int Simulation::create_window(std::string window_name) {
    // Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}

    // Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640 * 2, 480 * 2, window_name, NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);

	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}

	glGetError();
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);

	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);

    return 0;
}

void Simulation::create_scene() {
    
	glfwSetTime(0.0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glfwGetFramebufferSize(window, &width, &height);

	vector<string> attributes = {"aPos", "aNor"};
	vector<string> uniforms = {"MV", "iMV", "P", "lightPos", "ka", "kd", "ks", "s"};
	program = make_shared<Program>(vert_shader_path, frag_shader_path, attributes, uniforms);

	light = make_shared<Light>(glm::vec3(0.0f, 4.5f, 0.0f)); // TODO
	
	camera = make_shared<Camera>();

	ball = make_shared<Shape>();
	ball->createSphere(20);
	ball->fitToUnitBox();
	ball->init();
	ball_material = make_shared<Material>(glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.1f,0.2f,0.8f),glm::vec3(0.05f,0.95f,0.05f),200.0f);
	objects.push_back(make_shared<Object>(ball_material, ball, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(70.0f,50.0f,-20.2f), true));

	wall = make_shared<Shape>();
	wall->loadMesh("../resources/square.obj"); // TODO: fix generation script later
	wall->fitToUnitBox();
	wall->init();
	wall_material = make_shared<Material>(glm::vec3(0.2f,0.2f,0.2f),glm::vec3(0.6f,0.6f,0.6f),glm::vec3(0.01f,0.01f,0.01f),0.1f);

	objects.push_back(make_shared<Object>(wall_material, wall, glm::vec3(0.0f , 5.0f, 0.0f), glm::vec3(1.0f,0.0f,0.0f) * glm::pi<float>()/2.0f, glm::vec3(0.0f,0.0f,0.0f), false, 10.0f));
	objects.push_back(make_shared<Object>(wall_material, wall, glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(1.0f,0.0f,0.0f) * -glm::pi<float>()/2.0f, glm::vec3(0.0f,0.0f,0.0f), false, 10.0f));
	objects.push_back(make_shared<Object>(wall_material, wall, glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f,1.0f,0.0f) * -glm::pi<float>()/2.0f, glm::vec3(0.0f,0.0f,0.0f), false, 10.0f));
	objects.push_back(make_shared<Object>(wall_material, wall, glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(0.0f,1.0f,0.0f) * glm::pi<float>()/2.0f, glm::vec3(0.0f,0.0f,0.0f), false, 10.0f));
	objects.push_back(make_shared<Object>(wall_material, wall, glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(1.0f,0.0f,0.0f) * 0.0f, glm::vec3(0.0f,0.0f,0.0f), false, 10.0f));
	objects.push_back(make_shared<Object>(wall_material, wall, glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f,1.0f,0.0f) * glm::pi<float>(), glm::vec3(0.0f,0.0f,0.0f), false, 10.0f));

	// GLSL::checkError(GET_FILE_LINE);
}

void Simulation::reset() {

}

void Simulation::render_scene() {

}

void error_callback(int error, const char *description) { 
	cerr << description << endl; 
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { 
		glfwSetWindowShouldClose(window, GL_TRUE);
	} 	

	if (key == GLFW_KEY_W) 
		inputs[(unsigned)'w'] = action != GLFW_RELEASE;								
	if (key == GLFW_KEY_S) 
		inputs[(unsigned)'s'] = action != GLFW_RELEASE;
	if (key == GLFW_KEY_D) 
		inputs[(unsigned)'d'] = action != GLFW_RELEASE;
	if (key == GLFW_KEY_A) 
		inputs[(unsigned)'a'] = action != GLFW_RELEASE;
	if (key == GLFW_KEY_Q) 
		inputs[(unsigned)'q'] = action != GLFW_RELEASE;
	if (key == GLFW_KEY_E) 
		inputs[(unsigned)'e'] = action != GLFW_RELEASE;
	if (key == GLFW_KEY_C) 
		inputs[(unsigned)'c'] = action != GLFW_RELEASE;
	if (key == GLFW_KEY_V) 
		inputs[(unsigned)'v'] = action != GLFW_RELEASE;
	if (key == GLFW_KEY_F) 
		inputs[(unsigned) 'f'] = true;
	if (key == GLFW_KEY_Z && (mods == GLFW_MOD_SHIFT || inputs[(unsigned) 'Z'])) {
		inputs[(unsigned)'Z'] = action != GLFW_RELEASE;
	}
	if (key == GLFW_KEY_Z && mods != GLFW_MOD_SHIFT && !inputs[(unsigned) 'Z']) {
		inputs[(unsigned)'z'] = action != GLFW_RELEASE;
	}
	
}

void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse){
	if (inputs[(unsigned) 'f']) {
		float xdiff = (xmouse - o_x) * SENSITIVITY;
		float ydiff = (ymouse - o_y) * SENSITIVITY;

		camera->yaw -= xdiff;
		camera->pitch -= ydiff;

		camera->pitch = min(glm::pi<float>()/3.0f, camera->pitch);
		camera->pitch = max(-glm::pi<float>()/3.0f, camera->pitch);
		o_x = xmouse;
		o_y = ymouse;
	}
}

void char_callback(GLFWwindow *window, unsigned int key){
	keyToggles[key] = !keyToggles[key];
}

void resize_callback(GLFWwindow *window, int width, int height){ 
	glViewport(0, 0, width, height);
}