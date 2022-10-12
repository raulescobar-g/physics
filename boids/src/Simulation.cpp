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
	dt = 1.0f/24.0f;

	boids_k = glm::vec3(3.0f, 5.0f, 0.3f);
	float pi = glm::pi<float>();
	attention = glm::vec4(7.0f, 10.0f, pi/4.0, 2.0f*pi/4.0f );
	gravity = glm::vec3(0.0f, 0.0f, 0.0f);
	wind = glm::vec3(0.0f, 0.0f, 0.0f);
	lightPos = glm::vec3(0.0f, 300.0f, 0.0f);

	speed_limit = 10.0f;
	acceleration_limit = 30.0f;
	vision_distance = 20.0f;
	minimum_speed = 5.0f;
	predator_speed = 30.0f;
	predator_avoidance = 30.0f;
	predator_avoidance_internal = 90.0f;
	predator_attention_radius = 30.0f;

	steering_speed = 30.0f;
	box_sidelength = 100.0f;

	
	predator_amount = 0;
	boid_amount = 1024 * 8 - predator_amount;

	dups = 128;
	dims = (int) std::floor(box_sidelength / attention.y);
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
	window = glfwCreateWindow(640 * 3, 480 * 3, window_name, NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
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

    return 0;
}

void Simulation::init_programs(){
	std::vector<std::string> mesh_attributes = {"aPos", "aNor"};
	std::vector<std::string> mesh_uniforms = {"MV", "iMV", "P", "ka", "kd", "ks", "s", "lightPos"};
	meshes_program = std::make_shared<Program>();
	meshes_program->init("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\phong_vert.glsl", 
						"C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\phong_frag.glsl", mesh_attributes, mesh_uniforms);

	std::vector<std::string> boids_attributes = {"aPos", "aNor", "position", "color", "velocity"};
	std::vector<std::string> boids_uniforms = {"P","MV", "iMV", "ka", "kd", "ks", "s", "lightPos",};
	boids_program = std::make_shared<Program>();
	boids_program->init("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\boid_vert.glsl", 
						"C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\boid_frag.glsl", boids_attributes, boids_uniforms);

	std::vector<std::string> compute_uniforms = {"dt", "gravity", "wind", "objects", "time", "counters", "k", 
												"attention", "steering_speed", "boid_count", "speed_limit", 
												"acceleration_limit", "vision_distance", "minimum_speed", 
												"predator_speed", "predator_avoidance", "predator_attention_radius",
												"predator_avoidance_internal", "dims", "dups","spacing"};
	compute_program = std::make_shared<Program>();
	compute_program->init("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\boid_comp.glsl", compute_uniforms);
}

void Simulation::init_camera(){
	camera = std::make_shared<Camera>();
}

void Simulation::set_scene() {

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	std::shared_ptr<Shape> ball = std::make_shared<Shape>();
	ball->loadMesh("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\sphere.obj");
	ball->fitToUnitBox();
	ball->init();

	std::shared_ptr<Shape> wall = std::make_shared<Shape>();
	wall->loadMesh("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\square.obj");
	wall->fitToUnitBox();
	wall->init();
	meshes_program->unbind();

	std::shared_ptr<Shape> bunny = std::make_shared<Shape>();
	bunny->loadMesh("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\bunny.obj");
	bunny->fitToUnitBox();
	bunny->init();

	std::shared_ptr<Shape> ring = std::make_shared<Shape>();
	ring->loadMesh("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\torus.obj");
	ring->fitToUnitBox();
	ring->init();

	std::shared_ptr<Material> obstacle_material = std::make_shared<Material>();
	obstacle_material->ka = glm::vec3(0.3f, 0.3f, 0.3f);
	obstacle_material->kd = glm::vec3(0.5f, 0.5f, 0.1f);
	obstacle_material->ks = glm::vec3(0.5f, 0.5f, 0.5f);
	obstacle_material->s = 1.0f;

	std::shared_ptr<Material> wall_material = std::make_shared<Material>();
	wall_material->ka = glm::vec3(0.3f, 0.3f, 1.0f);
	wall_material->kd = glm::vec3(0.5f, 0.5f, 0.1f);
	wall_material->ks = glm::vec3(0.5f, 0.5f, 0.5f);
	wall_material->s = 1.0f;

	std::shared_ptr<Material> floor_material = std::make_shared<Material>();
	floor_material->ka = glm::vec3(1.0f, 0.6f, 0.6f);
	floor_material->kd = glm::vec3(0.5f, 0.5f, 0.1f);
	floor_material->ks = glm::vec3(0.5f, 0.5f, 0.5f);
	floor_material->s = 10000.0f;

	boid_material = std::make_shared<Material>();
	boid_material->ka = glm::vec3(1.0f, 0.6f, 0.3f);
	boid_material->kd = glm::vec3(0.7f, 0.7f, 0.7f);
	boid_material->ks = glm::vec3(0.7f, 0.7f, 0.7f);
	boid_material->s = 1.0f;

	predator_material = std::make_shared<Material>();
	predator_material->ka = glm::vec3(0.3f, 0.3f, 0.5f);
	predator_material->kd = glm::vec3(0.7f, 0.7f, 0.7f);
	predator_material->ks = glm::vec3(0.7f, 0.7f, 0.7f);
	predator_material->s = 1.0f;

	std::shared_ptr<Object> ball_obstacle = std::make_shared<Object>();
	ball_obstacle->shape = ball;
	ball_obstacle->material = obstacle_material;
	ball_obstacle->position = glm::vec3(0.0f, 0.0f, 0.0f);
	ball_obstacle->scale = glm::vec3(10.0f);
	objects.push_back(ball_obstacle);

	std::shared_ptr<Object> ring_obstacle1 = std::make_shared<Object>();
	ring_obstacle1->shape = ring;
	ring_obstacle1->material = obstacle_material;
	ring_obstacle1->position = glm::vec3(0.0f, -30.0f, 0.0f);
	ring_obstacle1->scale = glm::vec3(20.0f);
	objects.push_back(ring_obstacle1);

	std::shared_ptr<Object> ring_obstacle2 = std::make_shared<Object>();
	ring_obstacle2->shape = ring;
	ring_obstacle2->material = obstacle_material;
	ring_obstacle2->position = glm::vec3(0.0f, 30.0f, 0.0f);
	ring_obstacle2->scale = glm::vec3(20.0f);
	objects.push_back(ring_obstacle2);

	std::shared_ptr<Object> ring_obstacle3 = std::make_shared<Object>();
	ring_obstacle3->shape = ring;
	ring_obstacle3->material = obstacle_material;
	ring_obstacle3->position = glm::vec3(-30.0f, 0.0f, 0.0f);
	ring_obstacle3->scale = glm::vec3(20.0f);
	ring_obstacle3->rotation = glm::vec3(0.0f, 0.0f, -glm::pi<float>() / 2.0f);
	objects.push_back(ring_obstacle3);

	std::shared_ptr<Object> ring_obstacle4 = std::make_shared<Object>();
	ring_obstacle4->shape = ring;
	ring_obstacle4->material = obstacle_material;
	ring_obstacle4->position = glm::vec3(30.0f, 0.0f, 0.0f);
	ring_obstacle4->scale = glm::vec3(20.0f);
	ring_obstacle4->rotation = glm::vec3(0.0f, 0.0f, -glm::pi<float>() / 2.0f);
	objects.push_back(ring_obstacle4);

	std::shared_ptr<Object> first_wall_ptr = std::make_shared<Object>();
	first_wall_ptr->shape = wall;
	first_wall_ptr->material = wall_material;
	first_wall_ptr->position = glm::vec3(0.0f , box_sidelength/2.0f, 0.0f);
	first_wall_ptr->scale = glm::vec3(box_sidelength + 0.000001f);
	first_wall_ptr->rotation = glm::vec3(glm::pi<float>() / 2.0f, 0.0f, 0.0f);
	objects.push_back(first_wall_ptr);

	std::shared_ptr<Object> second_wall_ptr = std::make_shared<Object>();
	second_wall_ptr->shape = wall;
	second_wall_ptr->material = floor_material;
	second_wall_ptr->position = glm::vec3(0.0f , -box_sidelength/2.0f, 0.0f);
	second_wall_ptr->scale = glm::vec3(box_sidelength + 0.000001f);
	second_wall_ptr->rotation = glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f);
	objects.push_back(second_wall_ptr);

	std::shared_ptr<Object> third_wall_ptr = std::make_shared<Object>();
	third_wall_ptr->shape = wall;
	third_wall_ptr->material = wall_material;
	third_wall_ptr->position =  glm::vec3(box_sidelength/2.0f , 0.0f, 0.0f);
	third_wall_ptr->scale = glm::vec3(box_sidelength + 0.000001f);
	third_wall_ptr->rotation = glm::vec3(0.0f, -glm::pi<float>() / 2.0f, 0.0f);
	objects.push_back(third_wall_ptr);

	std::shared_ptr<Object> fourth_wall_ptr = std::make_shared<Object>();
	fourth_wall_ptr->shape = wall;
	fourth_wall_ptr->material = wall_material;
	fourth_wall_ptr->position = glm::vec3(-box_sidelength/2.0f , 0.0f, 0.0f);
	fourth_wall_ptr->scale = glm::vec3(box_sidelength + 0.000001f);
	fourth_wall_ptr->rotation = glm::vec3(0.0f, glm::pi<float>() / 2.0f, 0.0f);
	objects.push_back(fourth_wall_ptr);

	std::shared_ptr<Object> fifth_wall_ptr = std::make_shared<Object>();
	fifth_wall_ptr->shape = wall;
	fifth_wall_ptr->material = wall_material;
	fifth_wall_ptr->position = glm::vec3(0.0f , 0.0f, box_sidelength/2.0f);
	fifth_wall_ptr->scale = glm::vec3(box_sidelength + 0.000001f);
	fifth_wall_ptr->rotation = glm::vec3(0.0f,  glm::pi<float>(), 0.0f);
	objects.push_back(fifth_wall_ptr);

	std::shared_ptr<Object> sixth_wall_ptr = std::make_shared<Object>();
	sixth_wall_ptr->shape = wall;
	sixth_wall_ptr->material = wall_material;
	sixth_wall_ptr->position = glm::vec3(0.0f , 0.0f, -box_sidelength/2.0f);
	sixth_wall_ptr->scale = glm::vec3(box_sidelength + 0.000001f);
	objects.push_back(sixth_wall_ptr);


	// set all time params
	current_time = glfwGetTime();
	total_time = 0.0f;
	
	
	boids_program->bind();
	
	boids = std::make_shared<Boids>();
	boids->load_boid_mesh();
	boids->buffer_world_geometry(objects);
	
	boids->init(boid_amount, predator_amount, dups, dims, box_sidelength);
	boids_program->unbind();
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
	compute_program->bind();
	glUniform1f(compute_program->getUniform("dt"), _dt);
	glUniform1i(compute_program->getUniform("objects"), objects.size());
	glUniform3f(compute_program->getUniform("gravity"), gravity.x, gravity.y, gravity.z);
	glUniform3f(compute_program->getUniform("wind"), wind.x, wind.y, wind.z);
	glUniform1f(compute_program->getUniform("time"), glfwGetTime());
	glUniform3f(compute_program->getUniform("k"), boids_k.x, boids_k.y, boids_k.z);
	glUniform4f(compute_program->getUniform("attention"), attention.x, attention.y, attention.z, attention.w);
	glUniform1f(compute_program->getUniform("steering_speed"), steering_speed);
	glUniform1f(compute_program->getUniform("speed_limit"), speed_limit);
	glUniform1f(compute_program->getUniform("acceleration_limit"), acceleration_limit);
	glUniform1f(compute_program->getUniform("vision_distance"), vision_distance);
	glUniform1f(compute_program->getUniform("minimum_speed"), minimum_speed);
	glUniform1f(compute_program->getUniform("predator_avoidance"), predator_avoidance);
	glUniform1f(compute_program->getUniform("predator_avoidance_internal"), predator_avoidance_internal);
	glUniform1f(compute_program->getUniform("predator_speed") ,predator_speed);
	glUniform1f(compute_program->getUniform("predator_attention_radius"), predator_attention_radius);
	glUniform1i(compute_program->getUniform("boid_count"), boid_amount);
	glUniform1i(compute_program->getUniform("dims"), dims);
	glUniform1i(compute_program->getUniform("dups"), dups);
	glUniform1f(compute_program->getUniform("spacing"), box_sidelength);
	boids->update();
	compute_program->unbind();
}

void Simulation::render_scene() {
	glfwPollEvents();	

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
	ImGui::NewFrame();
	ImGui::Begin("Toolbar: ",nullptr, flags);
	ImGui::Text("%.3f dt / %.1f fps", dt, ImGui::GetIO().Framerate);
	ImGui::Text("Boids: %d ", boid_amount);

	float *lightPos_pointer = &lightPos.x;

	ImGui::DragFloat3("light position", lightPos_pointer);
	ImGui::DragFloat("predator avoidance", &predator_avoidance, 0.5f, 0.0001f, 50.0f);
	ImGui::DragFloat("predator avoidance (internal)", &predator_avoidance_internal, 0.5f, 0.0001f, 50.0f);
	ImGui::DragFloat("predator instince", &predator_speed, 0.5f, 0.0001f, 50.0f);
	ImGui::DragFloat("predator attention radius", &predator_attention_radius, 0.5f, 0.0001f, 50.0f);
	ImGui::DragFloat("velocity limit", &speed_limit, 0.5f, 0.0001f, 50.0f);
	ImGui::DragFloat("acceleration budget", &acceleration_limit, 0.5f, 0.0001f, 50.0f);
	ImGui::DragFloat("obstacle vision distance",&vision_distance, 0.5f, 0.0001f, 50.0f);
	ImGui::DragFloat("minimum speed", &minimum_speed, 0.1f, 0.0001f, 10.0f);
	ImGui::DragFloat("steering speed", &steering_speed, 0.5f, 0.0001f, 50.0f);
	ImGui::DragFloat("avoidance", &boids_k.x, 0.1f, 0.0001f, 50.0f);
	ImGui::DragFloat("velocity matching", &boids_k.y, 0.1f, 0.0001f, 50.0f);
	ImGui::DragFloat("centering", &boids_k.z, 0.1f, 0.0001f, 50.0f);

	ImGui::DragFloat("attention radius", &attention.x, 0.1f, 0.01f, 50.0f);
	ImGui::DragFloat("attention radius max", &attention.y, 0.1f, attention.x, 50.0f);
	ImGui::DragFloat("attention angle", &attention.z, 0.1f, 0.0f, 50.0f);
	ImGui::DragFloat("attention angle max", &attention.w, 0.1f, attention.z, 50.0f);

	if (ImGui::Button("Cull backfaces"))
		options[(unsigned) 'c'] = !options[(unsigned) 'c'];
	if (ImGui::Button("Wireframes"))
		options[(unsigned) 'v'] = !options[(unsigned) 'v'];
	
	if (ImGui::Button("Camera Lock"))
		options[(unsigned) 'x'] = !options[(unsigned) 'x'];
	if (ImGui::Button("Fullscreen"))
		options[(unsigned) 'f'] = !options[(unsigned) 'f'];
	if (ImGui::Button("Reset Simulation"))
		boids->spawn_boids();	

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
	MV->pushMatrix();
		MV->translate(lightPos);
		glm::vec3 world_light_pos = MV->topMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	MV->popMatrix();
	glUniform3f(meshes_program->getUniform("lightPos"), world_light_pos.x, world_light_pos.y, world_light_pos.z);
	for (auto obj : objects){ 
		MV->pushMatrix();
		MV->translate(obj->position);
		MV->scale(obj->scale);
		if (glm::length(obj->rotation) > eps) MV->rotate(glm::length(obj->rotation), obj->rotation);
		
		iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));

		glUniformMatrix4fv(meshes_program->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(meshes_program->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(meshes_program->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
		glUniform3f(meshes_program->getUniform("ka"), obj->material->ka.x, obj->material->ka.y, obj->material->ka.z);
		glUniform3f(meshes_program->getUniform("kd"), obj->material->kd.x, obj->material->kd.y, obj->material->kd.z);
		glUniform3f(meshes_program->getUniform("ks"), obj->material->ks.x, obj->material->ks.y, obj->material->ks.z);
		glUniform1f(meshes_program->getUniform("s"), obj->material->s );

		obj->shape->draw(meshes_program); 	

		MV->popMatrix();
	} 

	meshes_program->unbind();

	boids_program->bind();
	glUniform3f(boids_program->getUniform("lightPos"), world_light_pos.x, world_light_pos.y, world_light_pos.z);
	glUniformMatrix4fv(boids_program->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(boids_program->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniformMatrix4fv(boids_program->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
	glUniform3f(boids_program->getUniform("ka"), boid_material->ka.x, boid_material->ka.y, boid_material->ka.z);
	glUniform3f(boids_program->getUniform("kd"), boid_material->kd.x, boid_material->kd.y, boid_material->kd.z);
	glUniform3f(boids_program->getUniform("ks"), boid_material->ks.x, boid_material->ks.y, boid_material->ks.z);
	glUniform1f(boids_program->getUniform("s"), boid_material->s );
	boids->draw_boids(boids_program);


	glUniform3f(boids_program->getUniform("lightPos"), world_light_pos.x, world_light_pos.y, world_light_pos.z);
	glUniformMatrix4fv(boids_program->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(boids_program->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniformMatrix4fv(boids_program->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
	glUniform3f(boids_program->getUniform("ka"), predator_material->ka.x, predator_material->ka.y, predator_material->ka.z);
	glUniform3f(boids_program->getUniform("kd"), predator_material->kd.x, predator_material->kd.y, predator_material->kd.z);
	glUniform3f(boids_program->getUniform("ks"), predator_material->ks.x, predator_material->ks.y, predator_material->ks.z);
	glUniform1f(boids_program->getUniform("s"), predator_material->s );
	boids->draw_predators(boids_program);
	boids_program->unbind();

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
	inputs[(unsigned)'z'] = ImGui::IsKeyDown(ImGuiKey_Z) && !ImGui::IsKeyDown(ImGuiKey_LeftShift);;
	
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