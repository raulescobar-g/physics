#include "Simulation.h"

Simulation::Simulation() {
    o_x = 0.0f;
    o_y = 0.0f;


	for (int i = 0; i < 256; ++i) {
		keyToggles[i] = false;
		inputs[i] = false;
	}

	movement_speed = 5.0f;
	sensitivity = 0.005f;

	eps = 0.01f;

	fps = 0.0f;
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
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return 0;
}

void Simulation::init_program(){
	std::vector<std::string> attributes = {"aPos", "aNor"};
	std::vector<std::string> uniforms = {"MV", "iMV", "P"};
	program = std::make_shared<Program>("../resources/phong_vert.glsl", "../resources/phong_frag.glsl", attributes, uniforms);

	std::vector<std::string> attributes_p = {"position", "color", "vertices"};
	std::vector<std::string> uniforms_p = {"pP","V"};
	particles_program = std::make_shared<Program>("../resources/particle_vert.glsl", "../resources/particle_frag.glsl", attributes_p, uniforms_p);
}

void Simulation::init_camera(){
	camera = std::make_shared<Camera>();
}

void Simulation::set_scene() {
	objects.clear();

	dt = 1.0f/24.0f;
	ball_size = 10.0f;
	
	glm::vec4 background_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	glClearColor(background_color.x, background_color.y, background_color.z, background_color.w);

	glEnable(GL_DEPTH_TEST);

	program->bind();
	std::shared_ptr<Shape> ball = create_ball_shape(30);
	program->unbind();
	Material ball_mat(glm::vec3(0.01f, 0.01f, 0.3f), glm::vec3(0.01f, 0.01f, 0.3f), glm::vec3(0.0f, 0.0f, 0.0f), 10.0f);
	std::shared_ptr<Material> ball_material = std::make_shared<Material>(ball_mat);

	objects.push_back(std::make_shared<Object>(ball_material, ball, glm::vec3(1.0f , -1.0f, 2.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f), false, ball_size));
	
	// set all time params
	current_time = glfwGetTime();
	total_time = 0.0f;
	
	gravity = glm::vec3(0.0f, -9.8f, 0.0f);
	wind = glm::vec3(0.0f, 0.0f, 0.0f);

	particles_program->bind();
	p_sys = Particles(10000);
	p_sys.init();
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
	p_sys.update(_dt);
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

	program->bind();
	for (auto obj : objects){ 
		MV->pushMatrix();
		MV->translate(obj->pos);
		MV->scale(obj->scale,obj->scale,obj->scale);
		if (glm::length(obj->rotation) > eps) MV->rotate(glm::length(obj->rotation), obj->rotation);
		
		iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));

		glUniformMatrix4fv(program->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(program->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(program->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));

		obj->shape->draw(program); 	

		MV->popMatrix();
	} 

	program->unbind();

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
	p_sys.draw(particles_program);

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

std::shared_ptr<Shape> Simulation::create_ball_shape(int resolution) {
	std::shared_ptr<Shape> ball = std::make_shared<Shape>();
	ball->createSphere(resolution);
	ball->fitToUnitBox();
	ball->init();
	return ball;
}

std::shared_ptr<Shape> Simulation::create_wall_shape() {
	std::shared_ptr<Shape> wall = std::make_shared<Shape>();
	wall->loadMesh("../resources/square.obj"); // TODO: fix generation script later
	wall->fitToUnitBox();
	wall->init();
	return wall;
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
		float xdiff = (xmouse - o_x) * sensitivity;
		float ydiff = (ymouse - o_y) * sensitivity;

		camera->yaw -= xdiff;
		camera->pitch -= ydiff;

		camera->pitch = glm::min(glm::pi<float>()/3.0f, camera->pitch);
		camera->pitch = glm::max(-glm::pi<float>()/3.0f, camera->pitch);
		o_x = xmouse;
		o_y = ymouse;
	}
}

void Simulation::char_callback_impl(GLFWwindow *window, unsigned int key){
	keyToggles[key] = !keyToggles[key];
}

void Simulation::resize_callback_impl(GLFWwindow *window, int width, int height){ 
	glViewport(0, 0, width, height);
}