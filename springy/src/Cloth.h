#pragma once
#ifndef CLOTH_H
#define CLOTH_H

#include "MeshUtil.h"
#include "Texture.h"
#include "Program.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include <vector>

class Cloth  {
    public:
        Cloth(int n, const char* file);
        void update(float dt);
        void initial_conditions(); // implement
    
    private:
        float mass;
        std::vector<glm::vec3> forces;
        std::vector<glm::vec3> prev_S;
        std::vector<glm::vec3> S;

        std::vector<strut> struts;
        std::vector<face> faces;

        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> indices;

        unsigned int posBufID, texBufID, indBufID;

        void init();
        void draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P);
        std::vector<glm::vec3> integrate(std::vector<glm::vec3> s, float h, std::vector<glm::vec3> ds);
        std::vector<glm::vec3> calculate_forces(std::vector<glm::vec3> state);
        

        std::shared_ptr<Texture> texture;

        float L = 0.1f;
        float k = 10.0f;
        float d = 0.1f;

        float cd = 0.3f;
        float cl = 0.3f;
        glm::vec3 gravity = glm::vec3(0.0f, -9.8f, 0.0f);
        glm::vec3 wind = glm::vec3(1.0f, 0.0f, 0.0f);
};


#endif