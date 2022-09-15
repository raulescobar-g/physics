#pragma once
#ifndef GUI_H
#define GUI_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Options.h"

class Gui {
    public:
        Gui();
        Gui(Gui const&);
        void operator=(Gui const&);
        ~Gui();

        static Gui& get_instance() {
            static Gui instance; 
            return instance;
        }

        int create_window(const char * window_name, const char * glsl_version);
        void set_options(Options options);
        bool update();
        bool window_closed();

        Options get_options() {return options; }

    private:
        GLFWwindow *gui_window;
        Options options;
};

#endif