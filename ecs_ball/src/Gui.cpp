#include "Gui.h"


Gui::Gui() {}

Gui::~Gui() {
    glfwMakeContextCurrent(gui_window);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(gui_window);
}

int Gui::create_window(const char * window_name, const char * glsl_version) {
    gui_window = glfwCreateWindow(640, 480, window_name, NULL, NULL);

    if(!gui_window) {
		glfwTerminate();
		return -1;
	}

    // Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(gui_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
    glfwSwapInterval(1);
    return 0;
}

bool Gui::window_closed() {
    return glfwWindowShouldClose(gui_window);
}

void Gui::set_options(Options options){
    this->options = options;
}

bool Gui::update() {
    bool clicked_restart = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glfwMakeContextCurrent(gui_window);
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Initial conditions");                        

    ImGui::Text("Edit the parameters and reset the simulation.");

    float h = options.get_dt();
    ImGui::InputFloat("h ", &h);
    options.set_dt(h);

    for (int i = 0; i < options.ball_amount(); ++i) {
        ImGui::InputFloat("initial x position", &f);  // make this radius dependant
        ImGui::InputFloat("initial y position", &f);
        ImGui::InputFloat("initial y position", &f);

        ImGui::InputFloat("initial x velocity", &f); // cap it at something 
        ImGui::InputFloat("initial y velocity", &f);
        ImGui::InputFloat("initial y velocity", &f);
    }

    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Reset simulation")) {
        clicked_restart = true;
    }

    ImGui::Text("Application (%.1f FPS)", ImGui::GetIO().Framerate);
    ImGui::End();
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(gui_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);

    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(gui_window);
    return !clicked_restart;

}