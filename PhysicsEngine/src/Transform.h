#pragma once

#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

struct Mesh;

struct StateVector {
    StateVector()= default;
    StateVector(glm::vec3);
    StateVector(glm::vec3,glm::vec3,glm::vec3);
    StateVector(const StateVector&);
    glm::vec3 x;
    glm::quat q;
    glm::vec3 p;
    glm::vec3 L;

    glm::vec3 scale;

    StateVector operator+(const StateVector& s);
    operator glm::mat4();
};

struct Inertia {
    Inertia();
    Inertia(float);
    float mass;
    glm::mat3 I_inv;
    void operator=(float m);
    void operator=(glm::mat3 I);
    operator float();
    operator glm::mat3();
};

struct dStateVector {
    dStateVector();
    dStateVector(glm::vec3);
    dStateVector(const StateVector&, Inertia&, glm::vec3);
    glm::vec3 v;
    glm::quat wq;
    glm::vec3 F;
    glm::vec3 T;
    StateVector operator*(float h);
};


struct Transform {
    Transform() = default;
    Transform(glm::vec3); // x
    Transform(glm::vec3, glm::vec3, float); // x,p,scale,mass

    StateVector state;
    dStateVector dstate;
    Inertia inertia;

    

    glm::vec3 centroid;

    operator glm::mat4();
    void operator=(const StateVector& s);
};

StateVector integrate_state(Transform& state, float h, glm::vec3 a);

#endif