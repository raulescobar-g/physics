#include "Simulation.h"

#include "SoftBody.h"
#include "Cloth.h"
#include "StaticBody.h"

Simulation::Simulation() {
    o_x = -1.0;
    o_y = -1.0;


	for (int i = 0; i < 256; ++i) {
		options[i] = false;
		inputs[i] = false;
	}

	movement_speed = 0.5f;
	sensitivity = 0.005f;
	eps = 0.01f;
	dt = 1.0f/144.0f;
	lightPos = glm::vec3(0.0f, 30.0f, 0.0f);
	gravity = glm::vec3(0.0f, 0.0f, 0.0f);
	wind = glm::vec3(0.0f, 0.0f, 0.0f);


	options[(unsigned)'v'] = true;
	options[(unsigned)'c'] = true;
}

Simulation::~Simulation() {
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Simulation::create_window(const char * window_name, const char * glsl_version) {
    // Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640 * 3, 480 * 3, window_name, NULL, NULL);
	if(!window) {
		glfwTerminate();
		std::cout<<"Failed to setup window! \n";
		exit(-1);
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
	glfwSwapInterval(1);
}

void Simulation::init_programs(){
	std::vector<std::string> mesh_attributes = {"aPos", "aNor"};
	std::vector<std::string> mesh_uniforms = {"MV", "iMV", "P", "ka", "kd", "ks", "s", "a", "lightPos"};
	meshes_program = std::make_shared<Program>();
	meshes_program->init("C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\phong_vert.glsl", 
						"C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\phong_frag.glsl", mesh_attributes, mesh_uniforms);

	std::vector<std::string> cloth_attributes = {"aPos", "aTex"};
	std::vector<std::string> cloth_uniforms = {"MV", "P", "texture"};
	cloth_program = std::make_shared<Program>();
	cloth_program->init("C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\cloth_vert.glsl", 
						"C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\cloth_frag.glsl", cloth_attributes, cloth_uniforms);
}

void Simulation::init_camera(){
	camera = std::make_shared<Camera>();
}

void Simulation::set_scene() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	std::shared_ptr<Material> floor_material = std::make_shared<Material>();
	floor_material->ka = glm::vec3(0.2f, 0.5f, 0.7f);
	floor_material->kd = glm::vec3(0.2f, 0.5f, 0.7f);
	floor_material->ks = glm::vec3(0.7f, 0.7f, 0.7f);
	floor_material->s = 100.0f;

	std::shared_ptr<Material> cube_material = std::make_shared<Material>();
	cube_material->ka = glm::vec3(0.7f, 0.5f, 0.2f);
	cube_material->kd = glm::vec3(0.7f, 0.5f, 0.2f);
	cube_material->ks = glm::vec3(0.7f, 0.7f, 0.7f);
	cube_material->s = 100.0f;

	InitialConditions floor_start = {
		glm::vec3(0.0f, -2.0f, 0.0f),
		glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f),
		glm::vec3(100.0f),
		glm::vec3(0.0f),
		glm::vec3(0.0f),
	};

	InitialConditions cube_start = {
		glm::vec3(-3.0f, 2.0f, -3.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2.0f),
		glm::vec3(0.0f),
		glm::vec3(0.0f),
	};

	std::shared_ptr<StaticBody> floor = std::make_shared<StaticBody>("C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\square.obj");
	floor->initial_conditions(floor_start, floor_material);
	statics.push_back(floor);

	std::shared_ptr<SoftBody> cube = std::make_shared<SoftBody>("C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\sphere.obj");
	cube->initial_conditions(cube_start, cube_material);
	softies.push_back(cube);

	// std::shared_ptr<Cloth> flag = std::make_shared<Cloth>(5, "C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\cloth.jpg");
	// flag->initial_conditions(glm::vec3(0.0f, 5.0f, 0.0f), 3.0f);
	// cloths.push_back(flag);


	// set all time params
	current_time = glfwGetTime();
	total_time = 0.0f;

	GLSL::checkError(GET_FILE_LINE);
}

void Simulation::fixed_timestep_update() {
	new_time = glfwGetTime();
	frame_time = new_time - current_time;
	current_time = new_time;
	total_time += frame_time;

	while (total_time >= dt) {
		update(dt);
		total_time -= dt;
	}
}

std::vector<std::vector<int>> broad_phase(std::vector<std::shared_ptr<Entity>>& entities) {
	std::vector<std::vector<int>> result;

	for (int i = 0; i < entities.size()-1; ++i) {
		result.push_back(std::vector<int>());
		for (int j = i+1; j < entities.size(); ++j) {
			if (j == i) continue;
			// check here
			result[i].push_back(j);
		}
	}
	return result;
}


void Simulation::update(float _dt) {
	
	glm::vec3 global_acceleration = gravity + wind;

	for (auto cloth : cloths) {
		cloth->update(_dt);
	}
	for (auto staticc: statics) {
		staticc->update(_dt);
	}
	for( auto soft: softies) {
		soft->update(_dt);
	}

	
	softies[0]->calculate_collision_response(statics[0], _dt);
}

void Simulation::render_scene() {
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Info: ");
	ImGui::Text("%.3f dt", dt);
	ImGui::Text("%.1f frametime", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("<%.1f, %.1f, %.1f>", camera->pos.x, camera->pos.y, camera->pos.z);
	if (ImGui::Button("Cull backfaces"))
		options[(unsigned) 'c'] = !options[(unsigned) 'c'];
	if (ImGui::Button("Wireframes"))
		options[(unsigned) 'v'] = !options[(unsigned) 'v'];
	if (ImGui::Button("Camera Lock"))
		options[(unsigned) 'x'] = !options[(unsigned) 'x'];
	ImGui::End();

	ImGui::Render();

	// try putting this elsewhere
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Matrix stacks
	auto P = std::make_shared<MatrixStack>();
	auto MV = std::make_shared<MatrixStack>();

	P->pushMatrix();
	MV->pushMatrix();
		camera->applyProjectionMatrix(P);
		camera->applyViewMatrix(MV);
		
		P->pushMatrix();
		MV->pushMatrix();
		

			meshes_program->bind();
			MV->pushMatrix();
				MV->translate(lightPos);
				glm::vec3 world_light_pos = MV->topMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			MV->popMatrix();
			glUniform3f(meshes_program->getUniform("lightPos"), world_light_pos.x, world_light_pos.y, world_light_pos.z);

			for (auto entity : statics){ 
				MV->pushMatrix();
				entity->draw(meshes_program, MV, P); 	
				MV->popMatrix();
			} 
			for (auto entity : softies){ 
				MV->pushMatrix();
				entity->draw(meshes_program, MV, P); 	
				MV->popMatrix();
			} 
			meshes_program->unbind();

		MV->popMatrix();	
		P->popMatrix();

		// P->pushMatrix();
		// MV->pushMatrix();

		// cloth_program->bind();
		// for (auto cloth : cloths) {
		// 	cloth->draw(cloth_program, MV, P);
		// }
		// cloth_program->unbind();

		// MV->popMatrix();	
		// P->popMatrix();

	MV->popMatrix();	
	P->popMatrix();

	
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

	float x =  sin(camera->yaw)*cos(camera->pitch);
	float z =  cos(camera->yaw)*cos(camera->pitch);
	glm::vec3 dir(x , 0.0f, z);
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	if (inputs[(unsigned)'w']) 
		buff += glm::normalize(dir); 
	
	if (inputs[(unsigned)'s']) 
		buff -= glm::normalize(dir); 
	
	if (inputs[(unsigned)'a']) 
		buff -= glm::normalize(glm::cross(dir, up));
	
	if (inputs[(unsigned)'d']) 
		buff += glm::normalize(glm::cross(dir, up));
	
	if (inputs[(unsigned)'q']) 
		buff += up;
	
	if (inputs[(unsigned)'e']) 
		buff -= up;
	

	if (glm::length(buff) > eps) 
		camera->pos += glm::normalize(buff) * movement_speed * 0.1f; 
	

	!options[(unsigned)'c'] ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	options[(unsigned)'v'] ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	if (inputs[(unsigned) 'z']) 
		camera->decrement_fovy();

	if (inputs[(unsigned) 'Z']) 
		camera->increment_fovy();
}

void Simulation::error_callback_impl(int error, const char *description) { 
	std::cerr << description << std::endl; 
}

void Simulation::input_capture(){
	
	if (ImGui::IsKeyDown(ImGuiKey_Escape)) { 
		glfwSetWindowShouldClose(window, GL_TRUE);
	} 	

	inputs[(unsigned)'w'] = ImGui::IsKeyDown(ImGuiKey_W);								 
	inputs[(unsigned)'s'] = ImGui::IsKeyDown(ImGuiKey_S);
	inputs[(unsigned)'d'] = ImGui::IsKeyDown(ImGuiKey_D);
	inputs[(unsigned)'a'] = ImGui::IsKeyDown(ImGuiKey_A);
	inputs[(unsigned)'q'] = ImGui::IsKeyDown(ImGuiKey_Q);
	inputs[(unsigned)'e'] = ImGui::IsKeyDown(ImGuiKey_E);
	inputs[(unsigned)'c'] = ImGui::IsKeyDown(ImGuiKey_C);
	inputs[(unsigned)'v'] = ImGui::IsKeyDown(ImGuiKey_V);
	inputs[(unsigned)'Z'] = ImGui::IsKeyDown(ImGuiKey_Z) && ImGui::IsKeyDown(ImGuiKey_LeftShift);
	inputs[(unsigned)'z'] = ImGui::IsKeyDown(ImGuiKey_Z) && !ImGui::IsKeyDown(ImGuiKey_LeftShift);
	
	options[(unsigned) 'x'] = ImGui::IsKeyReleased(ImGuiKey_F) ? !options[(unsigned) 'x'] : options[(unsigned) 'x'];
	
	ImVec2 mouse = ImGui::GetMousePos();
	if (options[(unsigned) 'x']) {
		if (o_x > 0.0 && o_y > 0.0) {
			float xdiff = (mouse.x - o_x) * sensitivity;
			float ydiff = (mouse.y - o_y) * sensitivity;

			camera->yaw -= xdiff;
			camera->pitch -= ydiff;

			camera->pitch = glm::min(glm::pi<float>()/2.0f, camera->pitch);
			camera->pitch = glm::max(-glm::pi<float>()/2.0f, camera->pitch);
		}
		o_x = mouse.x;
		o_y = mouse.y;
	}
	
}
