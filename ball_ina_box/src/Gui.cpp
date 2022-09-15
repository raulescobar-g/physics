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

    ImGui::Text("Simulation timestep size (0.00001 < dt < 1.0)");
    float h = options.get_dt();
    ImGui::InputFloat("timestep", &h);
    options.set_dt(h);
    ImGui::Separator();
    
    ImGui::Text("Size of Box");
    float r = options.get_box_size();
    ImGui::InputFloat("box size", &r);
    options.set_box_size(r);
    ImGui::Separator();

    ImGui::Text("Wind");
    glm::vec3 wind_buff = options.get_wind();
    ImGui::InputFloat("w_x", &wind_buff.x);  
    ImGui::InputFloat("w_y", &wind_buff.y);
    ImGui::InputFloat("w_z", &wind_buff.z);
    options.set_wind(wind_buff);
    ImGui::Separator();

    
    ImGui::Text("Gravity");
    glm::vec3 g = options.get_gravity();
    ImGui::InputFloat("g", &g.y);
    options.set_gravity(g);
    ImGui::Separator();

    for (int i = 0; i < options.ball_amount(); ++i) {
        std::string header = "Ball #" + std::to_string(i+1);
        if (ImGui::CollapsingHeader(header.c_str())) {
            ImGui::Indent();
            glm::vec3 vel_buf = options.get_ball_velocity(i);
            glm::vec3 pos_buff = options.get_ball_pos(i);
            float res = options.get_ball_res(i); 
            float mu = options.get_ball_friction(i);
            float size = options.get_ball_size(i); 
            float mass = options.get_ball_mass(i); 
            float drag = options.get_ball_drag(i);

            ImGui::Text("Position");
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " p_x").c_str(), &pos_buff.x); 
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " p_y").c_str(), &pos_buff.y);
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " p_z").c_str(), &pos_buff.z);
            pos_buff.x = glm::clamp(pos_buff.x, -r/2.0f + size, r/2.0f - size);
            pos_buff.y = glm::clamp(pos_buff.y, -r/2.0f + size, r/2.0f - size);
            pos_buff.z = glm::clamp(pos_buff.z, -r/2.0f + size, r/2.0f - size);
            options.set_ball_pos(i, pos_buff);

            ImGui::Text("Velocity");
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " v_x").c_str(), &vel_buf.x); 
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " v_y").c_str(), &vel_buf.y);
            ImGui::InputFloat(("ball#" + std::to_string(i+1) +  " v_z").c_str(), &vel_buf.z);
            options.set_ball_velocity(i, vel_buf);

            ImGui::Text("Ball Size");
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " size").c_str(), &size);
            size = glm::clamp(size, 0.1f, r/2.0f - 0.1f);
            options.set_ball_size(i, size);

            ImGui::Text("Restitution");
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " restitution").c_str(), &res);
            res = glm::clamp(res, 0.0000001f, 0.999999f);
            options.set_ball_res(i, res);

            ImGui::Text("Friction");
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " friction").c_str(), &mu);
            mu = glm::clamp(mu, 0.0f, 0.99999999f);
            options.set_ball_friction(i, mu);

            ImGui::Text("Drag");
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " drag").c_str(), &drag);
            drag = glm::max(0.0f, drag);
            options.set_ball_drag(i, drag);

            ImGui::Text("Mass");
            ImGui::InputFloat(("ball#" + std::to_string(i+1) + " mass").c_str(), &mass);
            options.set_ball_mass(i, mass);
            
            ImGui::Unindent();
            ImGui::Separator();
        }
    }

    if (ImGui::Button("Add a ball")) {
        options.add_a_ball();
    }
    //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Reset simulation")) {
        clicked_restart = true;
    }

    ImGui::Separator();

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