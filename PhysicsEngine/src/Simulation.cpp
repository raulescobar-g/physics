#include "Simulation.h"

#include <memory>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Mesh.h"
#include "Material.h"
#include "State.h"
#include "dState.h"
#include "Entity.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Camera.h"
#include "GeometryUtil.h"


Simulation::Simulation() {
	for (int i = 0; i < 256; ++i) {
		options[i] = false;
		inputs[i] = false;
	}
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

	programs.push_back(Program("phong_vert.glsl", "phong_frag.glsl", mesh_attributes, mesh_uniforms));
}

void Simulation::init_cameras(){
	cameras.push_back(Camera());
}

void Simulation::set_scene() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	unsigned int sphere, bunny, cube, plane, red, grey, blue, default_program;

	default_program = 0;

	// init meshes
	meshes.push_back(Mesh("sphere.obj"));
	meshes.push_back(Mesh("bunny.obj"));
	meshes.push_back(Mesh("cube.obj"));
	meshes.push_back(Mesh("square.obj"));

	sphere = 0;
	bunny = 1;
	cube = 2;
	plane = 3;

	// init materials
	materials.push_back(Material(glm::vec3(0.9f, 0.4f, 0.3f)));
	materials.push_back(Material(glm::vec3(0.5f)));
	materials.push_back(Material(glm::vec3(0.3f, 0.4f, 0.9f)));

	red = 0;
	grey = 1;
	blue = 2;

	// init state 
	states.push_back(State(glm::vec3(0.0f), glm::vec3(0.0f), 1.0f));
	states.push_back(State(glm::vec3(0.0f, -7.0f, 0.0f), glm::vec3(0.0f), -1.0f));

	states[1].scale = 10.0f;

	// init entities
	dynamic_entities.push_back(Entity(bunny, blue, 0));
	static_entities.push_back(Entity(cube, red, 1));


	for (auto entity: dynamic_entities) {
		glm::mat3 I = extract_inertia_tensor(meshes[entity.mesh], states[entity.state].scale, states[entity.state].mass);
		states[entity.state].I_inv = glm::inverse(I);
	}

	// set all time params
	current_time = glfwGetTime();
	total_time = 0.0f;
}

void Simulation::fixed_timestep_update() {
	new_time = glfwGetTime();
	frame_time = new_time - current_time;
	current_time = new_time;
	total_time += frame_time;

	while (total_time >= dt) {
		if (!options[(unsigned) 'p']) update();
		total_time -= dt;
	}
}

std::vector<State> Simulation::integrate(std::vector<State> state, float h) {
	std::vector<State> new_state;
	new_state.resize(state.size());

	glm::vec3 t_ext = glm::vec3(0.0f);

	for (int i = 0; i < state.size(); ++i) {
		glm::vec3 f_ext = gravity * state[i].mass;
		
		if (state[i].type == DYNAMIC) {
			dState dstate(state[i], f_ext, t_ext); 
			new_state[i] = state[i] + dstate * h;
		} else if (state[i].type == STATIC) {
			new_state[i] = state[i];
		}
	}
	
	return new_state;
} 

void Simulation::update() {
	
	auto new_states = integrate(states, dt);
	glm::vec4 response;
	for (auto& entt: dynamic_entities) {
		for (auto& static_entt: static_entities) {
			
			if (are_colliding(entt, static_entt, response)) {
				glm::vec3 n = response;
				float d = response.w;
				options[(unsigned) 'p'] = true;
			}
		}
	}
	states = new_states;
}

bool Simulation::are_colliding(Entity& ent1, Entity& ent2, glm::vec4 response) {
	Mesh& mesh1 = meshes[ent1.mesh];
	Mesh& mesh2 = meshes[ent2.mesh];

	std::vector<glm::vec3> verts1 = mesh1.get_vertices();
	std::vector<glm::vec3> verts2 = mesh2.get_vertices();

	State& state1 = states[ent1.state];
	State& state2 = states[ent2.state];

	glm::mat4 transform1 = state1;
	glm::mat4 transform2 = state2;

	for (int i = 0; i < verts1.size(); i+=3) {
		glm::vec3 v1
		for (int j = 0; j < verts2.size(); j+=3) {

		}
	}


}

void Simulation::resize_window() {
	// try putting this elsewhere
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
}

void Simulation::render_scene() {
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Info: ");
	ImGui::Text("%.3f dt", dt);
	ImGui::Text("%.1f frametime", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("<%.1f, %.1f, %.1f>", cameras[0].pos.x, cameras[0].pos.y, cameras[0].pos.z);
	if (ImGui::Button("Cull backfaces"))
		options[(unsigned) 'c'] = !options[(unsigned) 'c'];
	if (ImGui::Button("Wireframes"))
		options[(unsigned) 'v'] = !options[(unsigned) 'v'];
	if (ImGui::Button("Camera Lock"))
		options[(unsigned) 'x'] = !options[(unsigned) 'x'];
	if (ImGui::Button("Reset"))
		options[(unsigned) 'r'] = !options[(unsigned) 'r'];

	ImGui::End();
	ImGui::Render();

	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Matrix stacks
	auto P = std::make_shared<MatrixStack>();
	auto MV = std::make_shared<MatrixStack>();
	

	P->pushMatrix();
	MV->pushMatrix();

		cameras[0].applyProjectionMatrix(P);
		cameras[0].applyViewMatrix(MV);

		draw_entities(static_entities, MV, P);
		draw_entities(dynamic_entities, MV, P);

	MV->popMatrix();	
	P->popMatrix();
	
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Simulation::draw_entities(std::vector<Entity>& drawables, std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P) {
	glm::mat4 iMV;

	glm::vec3 world_light_pos = MV->topMatrix() * glm::vec4(lightPos, 1.0f);

	for (auto& entity: drawables) {
		Material& material = materials[entity.material];
		Mesh& mesh = meshes[entity.mesh];
		Program& program = programs[entity.program];
		State& state = states[entity.state];

		program.bind();
		P->pushMatrix();
		MV->pushMatrix();

			MV->multMatrix((glm::mat4) state);

			iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));
			glUniform3f(program.getUniform("lightPos"), world_light_pos.x, world_light_pos.y, world_light_pos.z);
			glUniformMatrix4fv(program.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(program.getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(program.getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
			glUniform3f(program.getUniform("ka"), material.ka.x, material.ka.y, material.ka.z);
			glUniform3f(program.getUniform("kd"), material.kd.x, material.kd.y, material.kd.z);
			glUniform3f(program.getUniform("ks"), material.ks.x, material.ks.y, material.ks.z);
			glUniform1f(program.getUniform("s"), material.s );
			glUniform1f(program.getUniform("a"), material.a );
			mesh.draw(program); 	

		MV->popMatrix();	
		P->popMatrix();
		program.unbind();
	}
}

bool Simulation::window_closed() {
	return glfwWindowShouldClose(window);
}

void Simulation::swap_buffers() {
	glfwSwapBuffers(window);
}

void Simulation::move_camera() {
	glm::vec3 buff(0.0f,0.0f,0.0f);

	float x =  sin(cameras[0].yaw)*cos(cameras[0].pitch);
	float z =  cos(cameras[0].yaw)*cos(cameras[0].pitch);
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
		cameras[0].pos += glm::normalize(buff) * movement_speed * 0.1f; 
	

	!options[(unsigned)'c'] ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	options[(unsigned)'v'] ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	if (inputs[(unsigned) 'z']) 
		cameras[0].decrement_fovy();

	if (inputs[(unsigned) 'Z']) 
		cameras[0].increment_fovy();
}

void Simulation::error_callback_impl(int error, const char *description) { 
	std::cerr << description << std::endl; 
}

void Simulation::look_around() {
	ImVec2 mouse = ImGui::GetMousePos();

	if (options[(unsigned) 'x']) {
		if (o_x > 0.0 && o_y > 0.0) {
			float xdiff = (mouse.x - o_x) * sensitivity;
			float ydiff = (mouse.y - o_y) * sensitivity;

			cameras[0].yaw -= xdiff;
			cameras[0].pitch -= ydiff;

			cameras[0].pitch = glm::min(glm::pi<float>()/2.0f, cameras[0].pitch);
			cameras[0].pitch = glm::max(-glm::pi<float>()/2.0f, cameras[0].pitch);
		}
		o_x = mouse.x;
		o_y = mouse.y;
	} else {
		o_x = -1.0;
		o_y = -1.0;
	}
}

void Simulation::input_capture(){
	
	if (ImGui::IsKeyDown(ImGuiKey_Escape)) { 
		glfwSetWindowShouldClose(window, GL_TRUE);
	} 	

	inputs[(unsigned)'w'] = ImGui::IsKeyDown(ImGuiKey_W);								 
	inputs[(unsigned)'s'] = ImGui::IsKeyDown(ImGuiKey_S);
	inputs[(unsigned)'d'] = ImGui::IsKeyDown(ImGuiKey_D);
	inputs[(unsigned)'a'] = ImGui::IsKeyDown(ImGuiKey_A);
	inputs[(unsigned)'q'] = ImGui::IsKeyDown(ImGuiKey_Space);
	inputs[(unsigned)'e'] = ImGui::IsKeyDown(ImGuiKey_LeftShift);
	inputs[(unsigned)'Z'] = ImGui::IsKeyDown(ImGuiKey_Z) && ImGui::IsKeyDown(ImGuiKey_LeftShift);
	inputs[(unsigned)'z'] = ImGui::IsKeyDown(ImGuiKey_Z) && !ImGui::IsKeyDown(ImGuiKey_LeftShift);
	
	options[(unsigned) 'x'] = ImGui::IsKeyReleased(ImGuiKey_F) ? !options[(unsigned) 'x'] : options[(unsigned) 'x'];
	options[(unsigned)'c'] = ImGui::IsKeyReleased(ImGuiKey_C) ? !options[(unsigned) 'c'] : options[(unsigned) 'c'];
	options[(unsigned)'v'] = ImGui::IsKeyReleased(ImGuiKey_V) ? !options[(unsigned) 'v'] : options[(unsigned) 'v'];
	
	

	if (options[(unsigned) 'r']) {
		dynamic_entities.clear();
        materials.clear();
        states.clear();
        meshes.clear();   
        //textures.clear(); 

		set_scene();

		options[(unsigned) 'r'] = false;
	}
}
