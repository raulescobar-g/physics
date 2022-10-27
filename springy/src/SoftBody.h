#pragma once
#ifndef SOFTBODY_H
#define SOFTBODY_H

#include "Entity.h"
#include <vector>
#include "MeshUtil.h"



class SoftBody: public Entity {
    public:
        SoftBody(const std::string& meshName);
        SoftBody(std::vector<float>&, std::vector<float>&, std::vector<float>&, std::vector<unsigned int>&);
        void update(float dt, const glm::vec3& a) override;
        void set_programs(std::shared_ptr<Program> f, std::shared_ptr<Program>  s, std::shared_ptr<Program> i, std::shared_ptr<Program> p, std::shared_ptr<Program> fi);        

        int size() { return particle_count; };
        void collision_response(std::shared_ptr<StaticBody>,float);

        float *positions;
        float *past_positions;
        struct particle *particles;
        float *past_velocities;

    protected:
        void loadMesh(const std::string &meshName);
        void init();
        void extract_struts();
        void integrate(float dt, unsigned int start_positionId, unsigned int start_particleId, unsigned int particleId, unsigned int out_positionId, unsigned int out_particleId);
        void integrate_and_cleanup(float dt);
        void calculate_forces(unsigned int posId, unsigned int particleId);

        // buffer id's
        unsigned int posBufID_k1;
        unsigned int posBufID_k2;
        unsigned int posBufID_k3;
        unsigned int posBufID_k4;

        unsigned int partSSbo;
        unsigned int partSSbo_k1;
        unsigned int partSSbo_k2;
        unsigned int partSSbo_k3;
        unsigned int partSSbo_k4;

        // we can keep these solo
        unsigned int faceSSbo;
        unsigned int strutSSbo;

        unsigned int past_posBufID;
        unsigned int past_velBufID;

        // initial conditions should be abstracted out of here this is wack as all hell
        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec3 acceleration = glm::vec3(0.0f);
        float mass = 5.0f;
        float L = 1.0f;
        float k = 5.0f;
        float d = 0.0001f;
        float tk = 10.0f;
        float td = 0.0001f;
        glm::vec3 wind = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 gravity = glm::vec3(0.0f, -1.0f, 0.0f);

        // params for dispatch
        int face_count, strut_count, particle_count;

        //my temporary buffers
        std::vector<face> temp_faces;
        std::vector<strut> temp_struts;
        std::vector<particle> temp_particles;
        std::vector<float> temp_positions, temp_normals;

        // compute programs
        std::shared_ptr<Program> integration_compute, face_compute, strut_compute, particle_compute, final_integration;
};

#endif
