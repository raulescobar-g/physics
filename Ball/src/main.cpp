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

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"
#include "Object.h"

using namespace std;

#define OBJECT_AMOUNT 100
#define MATERIAL_COUNT 100
#define MOVEMENT_SPEED 10.0f
#define SENSITIVITY 0.005
#define SPIN_SPEED 1.0

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

shared_ptr<Camera> camera;

shared_ptr<Program> prog_p;

shared_ptr<Shape> sphere;
shared_ptr<Shape> ground;

bool keyToggles[256] = {false}; // only for English keyboards!
bool inputs[256] = {false}; // only for English keyboards!

vector<Material> materials;
vector<Light> lights;
vector<Object> objects;


// could not think of a better way to initialize these values probably bad practice
double o_x = 0.0;			
double o_y = 0.0;
float t, dt;

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
	
	if (key == GLFW_KEY_Z && (mods == GLFW_MOD_SHIFT || inputs[(unsigned) 'Z'])) {
		inputs[(unsigned)'Z'] = action != GLFW_RELEASE;
	}
	if (key == GLFW_KEY_Z && mods != GLFW_MOD_SHIFT && !inputs[(unsigned) 'Z']) {
		inputs[(unsigned)'z'] = action != GLFW_RELEASE;
	}
	
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse){

	float xdiff = (xmouse - o_x) * SENSITIVITY;
	float ydiff = (ymouse - o_y) * SENSITIVITY;

	camera->yaw -= xdiff;
	camera->pitch -= ydiff;

	camera->pitch = min(glm::pi<float>()/3.0f, camera->pitch);
	camera->pitch = max(-glm::pi<float>()/3.0f, camera->pitch);
	o_x = xmouse;
	o_y = ymouse;
}


static void char_callback(GLFWwindow *window, unsigned int key){
	keyToggles[key] = !keyToggles[key];
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height){ 
	glViewport(0, 0, width, height);
}

// This function is called once to initialize the scene and OpenGL
static void init(){
	glfwSetTime(0.0);	
	srand (time(NULL));
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	prog_p = make_shared<Program>();
	prog_p->setShaderNames(RESOURCE_DIR + "phong_vert.glsl", RESOURCE_DIR + "phong_frag.glsl");
	prog_p->setVerbose(true);
	prog_p->init();
	prog_p->addAttribute("aPos");
	prog_p->addAttribute("aNor");
	prog_p->addUniform("MV");
	prog_p->addUniform("iMV");
	prog_p->addUniform("P");
	prog_p->addUniform("lightPos1");
	prog_p->addUniform("ka");
	prog_p->addUniform("kd");
	prog_p->addUniform("ks");
	prog_p->addUniform("s");
	prog_p->setVerbose(false);


	Light l1("lightPos1", glm::vec3(100.0f, 50.0f, -100.0f));
	lights.push_back(l1);
	
	camera = make_shared<Camera>();

	sphere = make_shared<Shape>();
	sphere->createSphere(20);
	sphere->fitToUnitBox();
	sphere->init();
	sphere->set_id("sphere");

	ground = make_shared<Shape>();
	ground->loadMesh(RESOURCE_DIR + "square.obj");
	ground->fitToUnitBox();
	ground->init();
	ground->set_id("ground");
	

	for (int i = 0; i < MATERIAL_COUNT; ++i){ 
		materials.push_back(
			Material( 
				glm::vec3((rand()%101)/100.0f,  (rand()%101)/100.0f,  (rand()%101)/100.0f),
				glm::vec3((rand()%101)/100.0f,  (rand()%101)/100.0f,  (rand()%101)/100.0f),
				glm::vec3((rand()%101)/100.0f,  (rand()%101)/100.0f,  (rand()%101)/100.0f),
				(float)(rand()%1001)
			)
		);
	}

	// push object into objects vector
	
	GLSL::checkError(GET_FILE_LINE);
}

static void move_camera() {

	dt = glfwGetTime() - t;
	t = glfwGetTime();
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


	if (inputs[(unsigned) 'z']){camera->decrement_fovy();}
	if (inputs[(unsigned) 'Z']){camera->increment_fovy();}
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
		for (Object& obj : objects){ //loops over objects except lights and ground
			MV->pushMatrix();
				
				//obj.update(t);
				MV->translate(obj.x, obj.y, obj.z);
				MV->scale(obj.scale,obj.scale,obj.scale);
				MV->rotate(obj.rotation,0.0f,1.0f,0.0f);
				
				iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));

				glUniformMatrix4fv(prog_p->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
				glUniformMatrix4fv(prog_p->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(prog_p->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
				glUniform3f(prog_p->getUniform("ka"), obj.material->ka.x, obj.material->ka.y, obj.material->ka.z);
				glUniform3f(prog_p->getUniform("kd"), obj.material->kd.x, obj.material->kd.y, obj.material->kd.z);
				glUniform3f(prog_p->getUniform("ks"), obj.material->ks.x, obj.material->ks.y, obj.material->ks.z);
				glUniform1f(prog_p->getUniform("s"), obj.material->s );

				obj.shape->draw(prog_p); 	
				
			MV->popMatrix();
			
		}

		prog_p->unbind();

	MV->popMatrix();	
	P->popMatrix();	
	
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Usage: ./Ball ../resources" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
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

	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		
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
