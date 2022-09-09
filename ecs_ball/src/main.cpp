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

// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"
#include "Object.h"

using namespace std;

#define MOVEMENT_SPEED 5.0f
#define SENSITIVITY 0.005f
#define VERT "../resources/phong_vert.glsl"
#define FRAG "../resources/phong_frag.glsl"
#define GRAVITY glm::vec3(0.0f, -9.81f, 0.0f)
#define WIND glm::vec3(1.0f, 0.0f, 0.0f)
#define DRAG 0.3f
#define WALL_RADIUS 5.0f
#define RADIUS 0.5f
#define EPS 0.01f
#define RESTITUTION 0.5f
#define MU 0.2f
#define dT 1.0f/60.0f

GLFWwindow *window, *gui_window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

shared_ptr<Camera> camera;

shared_ptr<Program> prog_p;

shared_ptr<Shape> ball;
shared_ptr<Shape> wall;

bool keyToggles[256] = {false}; // only for English keyboards!
bool inputs[256] = {false}; // only for English keyboards!

shared_ptr<Material> ball_material;
shared_ptr<Material> wall_material;
shared_ptr<Light> light;
vector< shared_ptr<Object> > objects;

// could not think of a better way to initialize these values probably bad practice
double o_x = 0.0;		 //TODO: changte to ptr	
double o_y = 0.0;

struct Triangle {
	Triangle()=default;
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 n) : v1(a), v2(b), v3(c), n(glm::normalize(n)), empty(false) {};
	Triangle(bool empty) : empty(empty) {};
	glm::vec3 v1, v2, v3;
	glm::vec3 n;
	bool empty;
	operator bool() {return empty;};
};

static void error_callback(int error, const char *description) { 
	cerr << description << endl; 
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
	
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

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse){
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

static void char_callback(GLFWwindow *window, unsigned int key){
	keyToggles[key] = !keyToggles[key];
}

static void resize_callback(GLFWwindow *window, int width, int height){ 
	glViewport(0, 0, width, height);
}

static void init(){
	int width, height;

	glfwSetTime(0.0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glfwGetFramebufferSize(window, &width, &height);

	vector<string> attributes = {"aPos", "aNor"};
	vector<string> uniforms = {"MV", "iMV", "P", "lightPos", "ka", "kd", "ks", "s"};
	prog_p = make_shared<Program>(VERT, FRAG, attributes, uniforms);

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



	GLSL::checkError(GET_FILE_LINE);
}

static void move_camera() {

	float dt = dT;
	
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
		camera->pos += glm::normalize(buff) * MOVEMENT_SPEED * dt; // keeps the movement the same speed even if moving diagonally by summing direction of movement vectors and normalizing
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

glm::vec3 calculate_acceleration(float mass, glm::vec3 velocity) {
	glm::vec3 air = (DRAG * (WIND - velocity))  / mass;
	return air + GRAVITY;
}

pair<float, glm::vec3> collision_found(glm::vec3 old_pos, glm::vec3 new_pos) {
	pair<float, glm::vec3> result(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	
	if (glm::abs(new_pos.x) >= WALL_RADIUS - RADIUS){
		float a = glm::abs(old_pos.x) - (WALL_RADIUS - RADIUS);
		float b = glm::abs(new_pos.x - old_pos.x) - a;
		result.first = a / (a - b);
		result.second += new_pos.x < 0.0f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(-1.0f, 0.0f, 0.0f);
	}
	if (glm::abs(new_pos.y) >= WALL_RADIUS - RADIUS) {
		float a = glm::abs(old_pos.y) - (WALL_RADIUS - RADIUS);
		float b = glm::abs(new_pos.y - old_pos.y) - a;
		result.first = glm::min(result.first, a / (a - b));
		result.second += new_pos.y < 0.0f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, -1.0f, 0.0f);
	} 
	if (glm::abs(new_pos.z) >= WALL_RADIUS - RADIUS) {
		float a = glm::abs(old_pos.z) - (WALL_RADIUS - RADIUS);
		float b = glm::abs(new_pos.z - old_pos.z) - a;
		result.first = glm::min(result.first, a / (a - b));
		result.second += new_pos.z < 0.0f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 0.0f, -1.0f);
	}
	result.second = glm::length(result.second) > 0.1f ? glm::normalize(result.second): result.second;
	return result;
}

bool is_sleeping(shared_ptr<Object> obj) {
	if (glm::length(obj->velocity) < 10.0f * EPS && obj->pos.y < RADIUS - WALL_RADIUS + EPS) {
		cout<<"sleeping"<<endl;
		return true;
	}
	return false;
}

// game physics
static void update(float dt) { // reorder into a while loop

	float f1 = 1.0f;
	glm::vec3 normal;
	shared_ptr<Object> collider;

	for (auto obj : objects) {
		if (obj->dynamic && !obj->sleeping) {
			if (is_sleeping(obj)) {
				obj->sleeping = true;
				continue;
			}
			glm::vec3 new_pos = obj->pos + obj->velocity * dt;

			pair<float,glm::vec3> c = collision_found(obj->pos, new_pos);
			if (c.first < f1) {
				f1 = c.first;
				normal = c.second;
				collider = obj;
			}
		}
	}

	for (auto obj : objects) {
		if (obj->dynamic && !obj->sleeping) {
			glm::vec3 a = calculate_acceleration(obj->mass, obj->velocity);
			obj->pos = obj->pos + obj->velocity * dt * f1;
			obj->velocity = obj->velocity + a * dt * f1;
		}
	}
	
	if (f1 < 1.0f) {
		glm::vec3 vn = glm::dot(collider->velocity, normal) * normal;
		glm::vec3 vt = collider->velocity - vn;

		collider->velocity = -RESTITUTION * vn  + vt - glm::min(MU * glm::length(vn), glm::length(vt)) * glm::normalize(vt);

		update(dt * (1.0f - f1));
	}
}

// This function is called to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);
	
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	glm::mat4 iMV;

	
	P->pushMatrix();	
	camera->applyProjectionMatrix(P);
	camera->applyViewMatrix(MV);	
	MV->pushMatrix();

	prog_p->bind();
	glm::vec3 light_coord = MV->topMatrix() * glm::vec4(light->position, 1.0f);
	glUniform3f(prog_p->getUniform("lightPos"), light_coord.x, light_coord.y, light_coord.z);

	for (auto obj : objects){ //loops over objects except lights and wall
		MV->pushMatrix();
			
		MV->translate(obj->pos);
		MV->scale(obj->scale,obj->scale,obj->scale);
		if (glm::length(obj->rotation) > 0.0000f) MV->rotate(glm::length(obj->rotation), obj->rotation);
		
		iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));

		glUniformMatrix4fv(prog_p->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog_p->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog_p->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
		glUniform3f(prog_p->getUniform("ka"), obj->material->ka.x, obj->material->ka.y, obj->material->ka.z);
		glUniform3f(prog_p->getUniform("kd"), obj->material->kd.x, obj->material->kd.y, obj->material->kd.z);
		glUniform3f(prog_p->getUniform("ks"), obj->material->ks.x, obj->material->ks.y, obj->material->ks.z);
		glUniform1f(prog_p->getUniform("s"), obj->material->s );

		obj->shape->draw(prog_p); 	
			
		MV->popMatrix();
		
	}

	prog_p->unbind();

	MV->popMatrix();	
	P->popMatrix();	
	
}

// time loop based on this blog post https://gafferongames.com/post/fix_your_timestep/
int main(int argc, char **argv)
{
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Set error callback.
	glfwSetErrorCallback(error_callback);

	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640 * 2, 480 * 2, "Raul Escobar", NULL, NULL);
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

	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);

	// Set mouse button callback.
	// glfwSetMouseButtonCallback(window, mouse_button_callback);

	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // THIS MAKES THE CURSOR DISAPPEAR

	// Initialize scene.
	init();

	// gui_window = glfwCreateWindow(640 * 2, 480 * 2, "Raul Escobar", NULL, NULL);
	// // Setup Dear ImGui context
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); (void)io;
    // //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    // //ImGui::StyleColorsClassic();

    // // Setup Platform/Renderer backends
    // ImGui_ImplGlfw_InitForOpenGL(gui_window, true);
	// const char* glsl_version = "#version 130";
    // ImGui_ImplOpenGL3_Init(glsl_version);

	float t = 0.0f;
	float dt = dT;


	float currentTime = glfwGetTime();
	float totalTime = 0.0f;
	float newTime, frameTime;

	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {

		newTime = glfwGetTime();
		frameTime = newTime - currentTime;
		currentTime = newTime;

		totalTime += frameTime;

		while (totalTime >= dt) {
			update(dt);
			totalTime -= dt;
			t += dt;
		}

		// update position and camera
		move_camera();

		// Render scene.
		render();

		// Swap front and back buffers.
		glfwSwapBuffers(window);

		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
