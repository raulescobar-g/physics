#pragma once
#ifndef SOFTBODY_H
#define SOFTBODY_H

#include "Entity.h"
#include <vector>


struct particle {
    glm::vec4 velocity,force;
};

struct face {
    int s1,s2,s3;       //strut index 
    float a12,a23,a31;  //angle
};

struct strut {
    float k,d,lo;
    float tk,td,to;   // k-stiffness, damper, rest length
    int v1,v2;      // the vertex indices
    int f1,f2;      // the faces indices can be only 1 if edge
};

class SoftBody: public Entity {
    public:
        SoftBody(const std::string& meshName);
        SoftBody(std::vector<float>&, std::vector<float>&, std::vector<float>&, std::vector<unsigned int>&);

        void update(float dt, const glm::vec3& a) override;

        void set_programs(std::shared_ptr<Program> f, std::shared_ptr<Program>  s, std::shared_ptr<Program> i, std::shared_ptr<Program> p, std::shared_ptr<Program> fi);        

        

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

        float *positions; // for debuggery
        struct particle *particles; // for debuggery

        // initial conditions should be abstracted out of here this is wack as all hell
        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec3 acceleration = glm::vec3(0.0f);
        float mass = 10.0f;
        float L = 0.1f;
        float k = 2.0f;
        float d = 0.01f;
        float tk = 1.0f;
        float td = 1.0f;
        glm::vec3 wind = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 gravity = glm::vec3(0.0f, 0.0f, 0.0f);

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

//  float mass = 10.0f;
//         float L = 0.1f;
//         float k = -20.0f;
//         float d = -10.0f;
//         float tk = 100.0f;
//         float td = 10.0f;