#include "Simulation.h"

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
	dt = 1.0f/60.0f;
	lightPos = glm::vec3(0.0f, 300.0f, 0.0f);
	gravity = glm::vec3(0.0f, 0.0f, 0.0f);
	wind = glm::vec3(0.0f, 0.0f, 0.0f);
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
	std::vector<std::string> mesh_uniforms = {"MV", "iMV", "P", "ka", "kd", "ks", "s", "lightPos"};
	meshes_program = std::make_shared<Program>();
	meshes_program->init("C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\phong_vert.glsl", 
						"C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\phong_frag.glsl", mesh_attributes, mesh_uniforms);

	std::vector<std::string> compute_uniforms = {"dt", "gravity", "wind", "objects", "time", "counters"};
	compute_program = std::make_shared<Program>();
	compute_program->init("C:\\Users\\raul3\\Programming\\physics\\springy\\resources\\spring_comp.glsl", compute_uniforms);
}

void Simulation::init_camera(){
	camera = std::make_shared<Camera>();
}

void Simulation::set_scene() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	std::shared_ptr<Shape> ball = std::make_shared<Shape>();
	ball->loadMesh("C:\\Users\\raul3\\Programming\\physics\\particles\\resources\\sphere.obj");
	ball->fitToUnitBox();
	ball->init();

	std::shared_ptr<Shape> wall = std::make_shared<Shape>();
	wall->loadMesh("C:\\Users\\raul3\\Programming\\physics\\particles\\resources\\square.obj");
	wall->fitToUnitBox();
	wall->init();
	meshes_program->unbind();

	std::shared_ptr<Shape> bunny = std::make_shared<Shape>();
	bunny->loadMesh("C:\\Users\\raul3\\Programming\\physics\\particles\\resources\\bunny.obj");
	bunny->fitToUnitBox();
	bunny->init();

	std::shared_ptr<Object> bunny_ptr = std::make_shared<Object>(bunny, glm::vec3(0.0f , 0.0f, 0.0f));
	bunny_ptr->scale = glm::vec3(10.0f);
	bunny_ptr->rotation = glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	//objects.push_back(bunny_ptr);

	std::shared_ptr<Object> wall_ptr = std::make_shared<Object>(wall, glm::vec3(0.0f , -0.1f, 0.0f));
	wall_ptr->scale = glm::vec3(300.0f);
	wall_ptr->rotation = glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f);
	//objects.push_back(wall_ptr);

	std::shared_ptr<Object> second_wall_ptr = std::make_shared<Object>(wall, glm::vec3(0.0f , -15.0f, 10.0f));
	second_wall_ptr->scale = glm::vec3(5.0f);
	second_wall_ptr->rotation = glm::vec3(-glm::pi<float>() * 17.0f / 32.0f, 0.0f, 0.0f);
	//objects.push_back(second_wall_ptr);

	std::shared_ptr<Object> ball_ptr = std::make_shared<Object>(ball, glm::vec3(0.0f, 0.0f, 0.0f));
	ball_ptr->scale = glm::vec3(10.0f);
	ball_ptr->rotation = glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f);
	//objects.push_back(ball_ptr);

	// set all time params
	current_time = glfwGetTime();
	total_time = 0.0f;

	// particles_program->bind();
	// particles= std::make_shared<Particles>();
	// particles->load_particle_mesh();
	// particles->buffer_world_geometry(objects);
	// int amount = 1024 * 128;
	// particles->init(1024 * 1024, 512 ,amount, compute_program); // max at 1 million particles
	// particles_program->unbind();

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

void Simulation::update(float _dt) {
	// compute next positions

	// compute collisions

	// compute collision response

	// assign new position and velocity to objects
	// compute_program->bind();
	// glUniform1f(compute_program->getUniform("dt"), _dt);
	// glUniform1i(compute_program->getUniform("objects"), entities.size());
	// glUniform3f(compute_program->getUniform("gravity"), gravity.x, gravity.y, gravity.z);
	// glUniform3f(compute_program->getUniform("wind"), wind.x, wind.y, wind.z);
	// glUniform1f(compute_program->getUniform("time"), glfwGetTime());
	// struts->update();
	// compute_program->unbind();
}

void Simulation::render_scene() {
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Info: ");
	ImGui::Text("%.3f dt", dt);
	ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
	if (ImGui::Button("Cull backfaces"))
		options[(unsigned) 'c'] = !options[(unsigned) 'c'];
	if (ImGui::Button("Wireframes"))
		options[(unsigned) 'v'] = !options[(unsigned) 'v'];
	if (ImGui::Button("Camera Lock"))
		options[(unsigned) 'x'] = !options[(unsigned) 'x'];
	ImGui::End();

	ImGui::Render();

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
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
		MV->translate(obj->position);
		MV->scale(obj->scale);
		if (glm::length(obj->rotation) > eps) MV->rotate(glm::length(obj->rotation), obj->rotation);
		
		iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));

		glUniformMatrix4fv(meshes_program->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(meshes_program->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(meshes_program->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));

		obj->shape->draw(meshes_program); 	

		MV->popMatrix();
	}  

	meshes_program->unbind();

	// boids_program->bind();
	// glUniform3f(boids_program->getUniform("lightPos"), world_light_pos.x, world_light_pos.y, world_light_pos.z);
	// glUniformMatrix4fv(boids_program->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	// glUniformMatrix4fv(boids_program->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	// glUniformMatrix4fv(boids_program->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
	// glUniform3f(boids_program->getUniform("ka"), boid_material->ka.x, boid_material->ka.y, boid_material->ka.z);
	// glUniform3f(boids_program->getUniform("kd"), boid_material->kd.x, boid_material->kd.y, boid_material->kd.z);
	// glUniform3f(boids_program->getUniform("ks"), boid_material->ks.x, boid_material->ks.y, boid_material->ks.z);
	// glUniform1f(boids_program->getUniform("s"), boid_material->s );
	// boids->draw_boids(boids_program);

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

GLFWwindow * Simulation::get_window() {
	return window;
}