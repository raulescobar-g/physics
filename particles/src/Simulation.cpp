#include "Simulation.h"

Simulation::Simulation() {
    o_x = 0.0f;
    o_y = 0.0f;

    vert_shader_path = "../resources/phong_vert.glsl";
    frag_shader_path = "../resources/phong_frag.glsl";

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
	glfwMakeContextCurrent(window);
	glfwDestroyWindow(window);
	
}

int Simulation::create_window(const char * window_name) {
	
    // Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640 * 2, 480 * 2, window_name, NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);

	
	glfwSwapInterval(1);
    return 0;
}

void Simulation::init_program(){
	std::vector<std::string> attributes = {"aPos", "aNor"};
	std::vector<std::string> uniforms = {"MV", "iMV", "P", "lightPos", "ka", "kd", "ks", "s"};
	program = std::make_shared<Program>(vert_shader_path, frag_shader_path, attributes, uniforms);
}

void Simulation::init_camera(){
	camera = std::make_shared<Camera>();
}

void Simulation::set_scene() {
	glfwMakeContextCurrent(window);
	objects.clear();

	dt = 0.01f;
	box_size = 10.0f;
	
	glm::vec4 background_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glClearColor(background_color.x, background_color.y, background_color.z, background_color.w);

	glEnable(GL_DEPTH_TEST);
	glfwGetFramebufferSize(window, &width, &height);

	
	std::shared_ptr<Shape> wall = create_wall_shape();
	Material wall_mat(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), 100.0f);
	std::shared_ptr<Material> wall_material = std::make_shared<Material>(wall_mat);

	float box_radius = box_size / 2.0f;
	objects.push_back(std::make_shared<Object>(wall_material, wall, glm::vec3(0.0f , box_radius, 0.0f), glm::vec3(1.0f,0.0f,0.0f) * glm::pi<float>()/2.0f, glm::vec3(0.0f,0.0f,0.0f), false, box_size));
	objects.push_back(std::make_shared<Object>(wall_material, wall, glm::vec3(0.0f, -box_radius, 0.0f), glm::vec3(1.0f,0.0f,0.0f) * -glm::pi<float>()/2.0f, glm::vec3(0.0f,0.0f,0.0f), false, box_size));
	objects.push_back(std::make_shared<Object>(wall_material, wall, glm::vec3(box_radius, 0.0f, 0.0f), glm::vec3(0.0f,1.0f,0.0f) * -glm::pi<float>()/2.0f, glm::vec3(0.0f,0.0f,0.0f), false, box_size));
	objects.push_back(std::make_shared<Object>(wall_material, wall, glm::vec3(-box_radius, 0.0f, 0.0f), glm::vec3(0.0f,1.0f,0.0f) * glm::pi<float>()/2.0f, glm::vec3(0.0f,0.0f,0.0f), false, box_size));
	objects.push_back(std::make_shared<Object>(wall_material, wall, glm::vec3(0.0f, 0.0f, -box_radius), glm::vec3(1.0f,0.0f,0.0f) * 0.0f, glm::vec3(0.0f,0.0f,0.0f), false, box_size));
	objects.push_back(std::make_shared<Object>(wall_material, wall, glm::vec3(0.0f, 0.0f, box_radius), glm::vec3(0.0f,1.0f,0.0f) * glm::pi<float>(), glm::vec3(0.0f,0.0f,0.0f), false, box_size));

	// set all time params
	glfwMakeContextCurrent(window);
	current_time = glfwGetTime();
	total_time = 0.0f;
	
	gravity = glm::vec3(0.0f, -9.8f, 0.0f);
	wind = glm::vec3(0.0f, 0.0f, 0.0f);

	// GLSL::checkError(GET_FILE_LINE);
}

void Simulation::fixed_timestep_update() {
	glfwMakeContextCurrent(window);
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

	// calc collision n stuff
}

void Simulation::render_scene() {
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get current frame buffer size.
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
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
		glUniform3f(program->getUniform("ka"), obj->material->ka.x, obj->material->ka.y, obj->material->ka.z);
		glUniform3f(program->getUniform("kd"), obj->material->kd.x, obj->material->kd.y, obj->material->kd.z);
		glUniform3f(program->getUniform("ks"), obj->material->ks.x, obj->material->ks.y, obj->material->ks.z);
		glUniform1f(program->getUniform("s"), obj->material->s );

		obj->shape->draw(program); 	
		MV->popMatrix();
	}

	program->unbind();

	MV->popMatrix();	
	P->popMatrix();	
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

std::shared_ptr<Shape> Simulation::create_wall_shape() {
	std::shared_ptr<Shape> wall = std::make_shared<Shape>();
	wall->loadMesh("../resources/square.obj"); 
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