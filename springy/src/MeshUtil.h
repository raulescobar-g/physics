#pragma once

#ifndef UTIL_H
#define UTIL_H

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

struct Material {
	glm::vec3 ka = glm::vec3(0.5f);
	glm::vec3 kd = glm::vec3(0.5f);
	glm::vec3 ks = glm::vec3(0.5f);
	float s = 10.0f;
};

struct InitialConditions {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);
};


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

inline bool near(glm::vec3 v1, glm::vec3 v2){
    return glm::abs(glm::length(v1 - v2)) < 0.001f;
}

inline bool unique_particle(glm::vec3 vertex, std::vector<glm::vec3>& vec, int& result) {
    result = vec.size();
    for (int i = 0; i < vec.size(); ++i) {
        if (near(vertex, vec[i])){
            result = i;
            return false;
        }
    }
    vec.push_back(vertex);
    return true;
}

inline bool unique_strut(int r1, int r2, std::vector<strut>& strut_table, int& result){
    result = strut_table.size();
    for (int i = 0; i < strut_table.size(); ++i) {
        int _r1 = strut_table[i].v1;
        int _r2 = strut_table[i].v2;
        if ( (r1 == _r1 && r2 == _r2) || (r1 == _r2 && r2 == _r1)){
            result = i;
            return false;
        }
    }
    return true;
}

inline float angle_v(glm::vec3 a, glm::vec3 b){
    return glm::acos( glm::clamp(glm::dot( glm::normalize(a), glm::normalize(b)),-0.99999f, 0.99999f) );
}

inline bool inside(glm::vec3 collision_position , glm::vec3 vertex_0, glm::vec3 vertex_1, glm::vec3 vertex_2) {
	glm::vec3 u = vertex_1 - vertex_0;
	glm::vec3 v = vertex_2 - vertex_0;
	glm::vec3 n = glm::cross(u, v);
	glm::vec3 w = collision_position - vertex_0;

	float gamma = glm::dot(glm::cross(u, w), n) / glm::dot(n,n);
	float beta = glm::dot(glm::cross(w, v), n) / glm::dot(n,n);
	float alpha = 1.0f - gamma - beta;

	return (alpha >= -0.00001f && beta >= -0.00001f && gamma >= -0.00001f) || (alpha <= 0.00001f && beta <= 0.00001f && gamma <= 0.00001f);
};

inline std::vector<glm::vec3> runge_kutta(std::vector<glm::vec3> s, float h, std::vector<glm::vec3> k1,  std::vector<glm::vec3> k2,  std::vector<glm::vec3> k3,  std::vector<glm::vec3> k4) {
    std::vector<glm::vec3> res;
    res.resize(s.size());
    for (int i = 0; i < s.size(); ++i) {
        res[i] = s[i] + (h/6.0f) * (k1[i] + 2.0f * k2[i] + 2.0f * k3[i] + k4[i]);
    }
    return res;
}

#endif