#pragma once
#ifndef OPTIONS_H
#define OPTIONS_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include "Object.h"
#include "Material.h"

class Options {
    public:
        Options();

        float get_dt();
        void set_dt(float);

        float get_box_size();
        void set_box_size(float r);

        glm::vec4 get_background_color();
        glm::vec3 get_light_pos();
        float get_ball_res();
        int ball_amount();
        Material get_ball_mat(int i);
        Material get_wall_material();

        glm::vec3 get_ball_pos(int i);
        glm::vec3 get_ball_velocity(int i);
        float get_ball_size(int i);
        float get_ball_res(int i);
        float get_ball_friction(int i);
        float get_ball_mass(int i);
        float get_ball_drag(int i);

        void set_ball_pos(int i, glm::vec3);
        void set_ball_velocity(int i, glm::vec3);
        void set_ball_size(int i, float);
        void set_ball_res(int i, float res);
        void set_ball_friction(int i, float friction);
        void set_ball_mass(int i, float mass);
        void set_ball_drag(int i, float d);


        glm::vec3 get_gravity();
        glm::vec3 get_wind();
        void set_gravity(glm::vec3 g);
        void set_wind(glm::vec3 w);

        void add_a_ball();

    private:
        float dt;
        float box_size;
        glm::vec3 light_pos;
        glm::vec4 background_color;
        glm::vec3 gravity;
        glm::vec3 wind;

        float ball_resolution;
        std::vector<glm::vec3>  ball_positions;
        std::vector<glm::vec3>  ball_velocities;
        std::vector<float>      ball_sizes;
        std::vector<float>      ball_restitutions;
        std::vector<float>      ball_frictions;
        std::vector<float>      ball_masses;
        std::vector<float>      ball_drags;
        std::vector<Material>   ball_materials;
        Material                wall_material;
    };

#endif