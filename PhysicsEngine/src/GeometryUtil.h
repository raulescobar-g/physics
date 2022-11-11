#pragma once

#ifndef GEOMETRYUTIL_H
#define GEOMETRYUTIL_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.h"

inline glm::mat3 extract_inertia_tensor(Mesh& mesh, float scale, float mass){
    glm::mat3 I(0.0f);
    std::vector<glm::vec3> p = mesh * scale;

    glm::vec3 com = glm::vec3(0.0f);
    float total_volume = 0.0f;

    for (int i = 0; i < p.size(); i+=3) {
        float volume = (glm::dot(p[i], glm::cross(p[i+1], p[i+2]))) / 6.0f;
        glm::vec3 x = (p[i] + p[i+1] + p[i+2]) / 4.0f;

        com += volume * x;
        total_volume += volume;
    }

    com = com / total_volume;

    float volume = 0.0f;
    glm::vec3 diag(0.0f);
    glm::vec3 offd(0.0f);

    for (int i = 0; i < p.size(); i+=3) {
        glm::mat3 A(p[i]-com, p[i+1]-com, p[i+2]-com);
        float d = glm::determinant(A);
        volume += d;

        for(int j = 0; j < 3; j++) {
            int j1=(j+1)%3;   
            int j2=(j+2)%3;   
            diag[j] += (A[0][j]*A[1][j] + A[1][j]*A[2][j] + A[2][j]*A[0][j] + 
                        A[0][j]*A[0][j] + A[1][j]*A[1][j] + A[2][j]*A[2][j]  ) *d; // divide by 60.0f later;
            offd[j] += (A[0][j1]*A[1][j2]  + A[1][j1]*A[2][j2]  + A[2][j1]*A[0][j2]  +
                        A[0][j1]*A[2][j2]  + A[1][j1]*A[0][j2]  + A[2][j1]*A[1][j2]  +
                        A[0][j1]*A[0][j2]*2.0f+ A[1][j1]*A[1][j2]*2.0f+ A[2][j1]*A[2][j2]*2.0f ) *d; // divide by 120.0f later
        }
    }

    diag /= volume*(60.0f /6.0f);  // divide by total volume (vol/6) since density=1/volume
    offd /= volume*(120.0f/6.0f);
    return mass * glm::mat3(diag.y+diag.z  , -offd.z      , -offd.y,
                            -offd.z        , diag.x+diag.z, -offd.x,
                            -offd.y        , -offd.x      , diag.x+diag.y );
} 


// struct CollisionDetermination {
//     glm::vec3 n;
//     float d;
//     int mesh1, mesh2;
// };

// inline Collision are_colliding(std::vector<glm::vec3>& triangles, ) {

// }


inline bool point_triangle(glm::vec3 p, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3& n, float& d) {

}

#endif


