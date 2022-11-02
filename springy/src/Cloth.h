#pragma once
#ifndef CLOTH_H
#define CLOTH_H

#include "MeshUtil.h"
#include "Texture.h"
#include "Program.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include <vector>
#include "StaticBody.h"

class Cloth  {
    public:
        Cloth(int n, const char* file);
        void update(float dt);
        void draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P);
        void initial_conditions(glm::vec3,float=1.0f); 
        void calculate_collision_response(std::shared_ptr<StaticBody> obj, float dt);
    
    private:
        float mass = 0.005f;
        std::vector<glm::vec3> forces;
        std::vector<glm::vec3> prev_S;
        std::vector<glm::vec3> S;

        std::vector<strut> struts;
        std::vector<face> faces;

        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> indices;

        unsigned int posBufID, texBufID, indBufID;

        void init();
        std::vector<glm::vec3> integrate(std::vector<glm::vec3> s, float h, std::vector<glm::vec3> ds);
        std::vector<glm::vec3> calculate_forces(std::vector<glm::vec3> state);

        std::shared_ptr<Texture> texture;

        float L = 0.1f;
        float k = 7000.0f;
        float d = 100.0f;

        float cd = 0.03f;
        float cl = 0.03f;
        float cr = 0.999f;
        float cf = 0.0f;

        glm::vec3 gravity = glm::vec3(0.0f, -5.0f, 0.0f);
        glm::vec3 wind = glm::vec3(5.0f, 0.0f, 5.0f);

        float scale = 1.0f;

        glm::vec3 anchor_translation = glm::vec3(0.0f, 5.0f, 0.0f);
        int anchor1=-1, anchor2=-1;
};


#endif