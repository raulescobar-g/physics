#pragma once
#ifndef SOFTBODY_H
#define SOFTBODY_H

#include "Entity.h"
#include <vector>
#include "MeshUtil.h"



class SoftBody {
    public:
        SoftBody(const std::string& meshName);
        void update(float dt, const glm::vec3& a=glm::vec3(0.0f));
        void draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P);
        void initial_conditions(InitialConditions& start, std::shared_ptr<Material> material);

        void calculate_collision_response(std::shared_ptr<StaticBody> obj, float dt);

    private:
        void loadMesh(const std::string &meshName);
        void init();
        void extract_struts();
        void fitToUnitBox();
        std::vector<glm::vec3> integrate(std::vector<glm::vec3> s, float h, std::vector<glm::vec3> ds);
        std::vector<glm::vec3> calculate_forces(std::vector<glm::vec3> state);

        std::vector<glm::vec3> S;
        std::vector<glm::vec3> prev_S;

        std::vector<strut> struts;
        std::vector<face> faces;

        std::vector<glm::vec3> normBuf;
        std::vector<unsigned int> indices;

        // buffer id's
        unsigned int posBufID, normBufID, indBufID;

        // initial conditions should be abstracted out of here this is wack as all hell
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec3 acceleration = glm::vec3(0.0f);

        float mass = 10.0f;
        float L = 1.0f;
        float k = 1200.0f;
        float d = 50.0f;
        float tk = 900.0f;
        float td = 1.0f;

        float cd = 0.3f;
        float cl = 0.3f;
        float cr = 0.9f;
        float cf = 0.1f;

        glm::vec3 wind = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 gravity = glm::vec3(0.0f, -10.0f, 0.0f);


        std::vector<float> posBuf, norBuf, texBuf;
        std::shared_ptr<Material> material;

};

#endif
