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

        void set_programs(std::shared_ptr<Program>, std::shared_ptr<Program>, std::shared_ptr<Program>);        

        

    protected:
        unsigned int partSSbo;
        unsigned int faceSSbo;
        unsigned int strutSSbo;

        void loadMesh(const std::string &meshName);
        void init();
        void extract_struts();

        struct particle *particles;
        struct face *faces;
        struct strut *struts;

        float *positions;

        glm::vec3 velocity = glm::vec3(0.0f), acceleration = glm::vec3(0.0f);
        float mass = 5.0f;

        int face_count, strut_count, particle_count;

        std::vector<face> temp_faces;
        std::vector<strut> temp_struts;
        std::vector<particle> temp_particles;
        std::vector<float> temp_positions, temp_normals;

        float L = 0.1f;
        float k = 1.0f;
        float d = 0.1f;
        float tk = 10.0f;
        float td = 1.0f;

        std::shared_ptr<Program> integration_compute, face_compute, strut_compute;
};

#endif