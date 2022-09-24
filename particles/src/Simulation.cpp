#include "Simulation.h"

Simulation::Simulation() {
    o_x = -1.0;
    o_y = -1.0;


	for (int i = 0; i < 256; ++i) {
		keyToggles[i] = false;
		inputs[i] = false;
	}

	movement_speed = 0.5f;
	sensitivity = 0.005f;
	eps = 0.01f;
	dt = 1.0f/24.0f;
}

Simulation::~Simulation() {
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
	
}

int Simulation::create_window(const char * window_name, const char * glsl_version) {
	
    // Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640 * 2, 480 * 2, window_name, NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return 0;
}

void Simulation::init_programs(){
	std::vector<std::string> mesh_attributes = {"aPos", "aNor"};
	std::vector<std::string> mesh_uniforms = {"MV", "iMV", "P"};
	meshes_program = std::make_shared<Program>();
	meshes_program->init("../resources/phong_vert.glsl", "../resources/phong_frag.glsl", mesh_attributes, mesh_uniforms);


	std::vector<std::string> particles_attributes = {"position", "color", "vertices"};
	std::vector<std::string> particles_uniforms = {"pP","V"};
	particles_program = std::make_shared<Program>();
	particles_program->init("../resources/particle_vert.glsl", "../resources/particle_frag.glsl", particles_attributes, particles_uniforms);

	std::vector<std::string> compute_uniforms = {"dt", "gravity", "wind", "polygons", "objects" };//, , "attractors"};
	compute_program = std::make_shared<Program>();
	compute_program->init("../resources/particle_comp.glsl", compute_uniforms);
}

void Simulation::init_camera(){
	camera = std::make_shared<Camera>();
}

void Simulation::set_scene() {
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);

	meshes_program->bind();
	int resolution = 100;
	std::shared_ptr<Shape> ball = std::make_shared<Shape>();
	ball->createSphere(30);
	ball->fitToUnitBox();
	ball->init();
	meshes_program->unbind();

	std::shared_ptr<Object> ball_ptr = std::make_shared<Object>(ball, glm::vec3(0.0f , 0.0f, 0.0f));
	ball_ptr->set_scale(30.0f);
	ball_ptr->set_rotation(glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f));
	objects.push_back(ball_ptr);
	
	// set all time params
	current_time = glfwGetTime();
	total_time = 0.0f;
	
	gravity = glm::vec3(0.0f, -0.5f, 0.0f);
	wind = glm::vec3(0.0f, 0.0f, 0.0f);

	particles_program->bind();
	particles= std::make_shared<Particles>();
	particles->load_particle_mesh();
	particles->buffer_world_geometry(objects);
	particles->init(1024 * 1024 , compute_program);
	particles_program->unbind();

	GLSL::checkError(GET_FILE_LINE);
}

void Simulation::fixed_timestep_update() {
	glfwPollEvents();
	new_time = glfwGetTime();
	frame_time = new_time - current_time;
	
	current_time = new_time;

	total_time += frame_time;

	while (total_time >= dt) {
		update(dt);
		total_time -= dt;
	}
}

void Simulation::update(float _dt) {
	compute_program->bind();
	glUniform1f(compute_program->getUniform("dt"), _dt);
	glUniform1i(compute_program->getUniform("polygons"), particles->get_poly_count());
	glUniform1i(compute_program->getUniform("objects"), objects.size());
	glUniform3f(compute_program->getUniform("gravity"), gravity.x, gravity.y, gravity.z);
	glUniform3f(compute_program->getUniform("wind"), wind.x, wind.y, wind.z);
	particles->update();
	compute_program->unbind();
}

void Simulation::render_scene() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui::Begin("info");
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::End();
	
	// Matrix stacks
	auto P = std::make_shared<MatrixStack>();
	auto MV = std::make_shared<MatrixStack>();
	glm::mat4 iMV;

	P->pushMatrix();	
	camera->applyProjectionMatrix(P);
	camera->applyViewMatrix(MV);	
	MV->pushMatrix();

	meshes_program->bind();
	for (auto obj : objects){ 
		MV->pushMatrix();
		MV->translate(obj->pos);
		MV->scale(obj->scale,obj->scale,obj->scale);
		if (glm::length(obj->rotation) > eps) MV->rotate(glm::length(obj->rotation), obj->rotation);
		
		iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));

		glUniformMatrix4fv(meshes_program->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(meshes_program->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(meshes_program->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));

		obj->shape->draw(meshes_program); 	

		MV->popMatrix();
	} 

	meshes_program->unbind();

	MV->popMatrix();	
	P->popMatrix();	

	auto _P = std::make_shared<MatrixStack>();
	auto _V = std::make_shared<MatrixStack>();
	_P->pushMatrix();
	camera->applyProjectionMatrix(_P);
	camera->applyViewMatrix(_V);	
	_V->pushMatrix();

	particles_program->bind();
	glUniformMatrix4fv(particles_program->getUniform("pP"), 1, GL_FALSE, glm::value_ptr(_P->topMatrix()));
	glUniformMatrix4fv(particles_program->getUniform("V"), 1, GL_FALSE, glm::value_ptr(_V->topMatrix()));
	particles->draw(particles_program, compute_program);

	particles_program->unbind();
	_V->popMatrix();
	_P->popMatrix();	

	// Rendering
	
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Simulation::window_closed() {
	return glfwWindowShouldClose(window);
}

void Simulation::swap_buffers() {
	glfwSwapBuffers(window);
}

void Simulation::move_camera() {
	
	glm::vec3 buff(0.0f,0.0f,0.0f);

	if (inputs[(unsigned)'w']) {
		buff += glm::normalize(glm::vec3( sin(camera->yaw)*cos(camera->pitch) , 0.0f, cos(camera->yaw)*cos(camera->pitch))); 
	}
	if (inputs[(unsigned)'s']) {
		buff -= glm::normalize(glm::vec3( sin(camera->yaw)*cos(camera->pitch) , 0.0f, cos(camera->yaw)*cos(camera->pitch))); 
	}
	if (inputs[(unsigned)'a']) {
		buff -= glm::normalize(glm::cross(glm::vec3( sin(camera->yaw)*cos(camera->pitch) , 0.0f, cos(camera->yaw)*cos(camera->pitch)), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	if (inputs[(unsigned)'d']) {
		buff += glm::normalize(glm::cross(glm::vec3( sin(camera->yaw)*cos(camera->pitch) , 0.0f, cos(camera->yaw)*cos(camera->pitch)), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	if (inputs[(unsigned)'q']) {
		buff += glm::vec3(0.0f,1.0f,0.0f);
	}
	if (inputs[(unsigned)'e']) {
		buff += glm::vec3(0.0f,-1.0f,0.0f);
	}

	if (glm::length(buff) > 0.00001f || glm::length(buff) < -0.00001f) {
		camera->pos += glm::normalize(buff) * movement_speed * 0.1f; // keeps the movement the same speed even if moving diagonally by summing direction of movement vectors and normalizing
	}

	if(!keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'v']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	if (inputs[(unsigned) 'z']){camera->decrement_fovy();}
	if (inputs[(unsigned) 'Z']){camera->increment_fovy();}
}

void Simulation::error_callback_impl(int error, const char *description) { 
	std::cerr << description << std::endl; 
}

void Simulation::key_callback_impl(GLFWwindow *window, int key, int scancode, int action, int mods){
	
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

	if (key == GLFW_KEY_Z && (mods == GLFW_MOD_SHIFT || inputs[(unsigned) 'Z'])) {
		inputs[(unsigned)'Z'] = action != GLFW_RELEASE;
	}
	if (key == GLFW_KEY_Z && mods != GLFW_MOD_SHIFT && !inputs[(unsigned) 'Z']) {
		inputs[(unsigned)'z'] = action != GLFW_RELEASE;
	}
	
}

void Simulation::cursor_position_callback_impl(GLFWwindow* window, double xmouse, double ymouse){
	if (keyToggles[(unsigned) 'f']) {
		if (o_x > 0.0 && o_y > 0.0) {
			float xdiff = (xmouse - o_x) * sensitivity;
			float ydiff = (ymouse - o_y) * sensitivity;

			camera->yaw -= xdiff;
			camera->pitch -= ydiff;

			camera->pitch = glm::min(glm::pi<float>()/3.0f, camera->pitch);
			camera->pitch = glm::max(-glm::pi<float>()/3.0f, camera->pitch);
			o_x = xmouse;
			o_y = ymouse;
		} else {
			o_x = xmouse;
			o_y = ymouse;
		}
	}
}

void Simulation::char_callback_impl(GLFWwindow *window, unsigned int key){
	keyToggles[key] = !keyToggles[key];
}

void Simulation::resize_callback_impl(GLFWwindow *window, int width, int height){ 
	glViewport(0, 0, width, height);
}

GLFWwindow * Simulation::get_window() {
	return window;
}