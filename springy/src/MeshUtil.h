#pragma once

#ifndef UTIL_H
#define UTIL_H

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

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
    strut t;
    t.v1 = r1;
    t.v2 = r2;
    strut_table.push_back(t);
    return true;
}

inline float angle_v(glm::vec3 a, glm::vec3 b){
    return glm::acos( glm::clamp(glm::dot( glm::normalize(a), glm::normalize(b)),-0.99999f, 0.99999f) );
}

#endif