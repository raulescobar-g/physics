#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Material {
    public:
    Material(){};
    Material(glm::vec3 ambient) : ka(ambient), kd(glm::vec3(0.5f)), ks(glm::vec3(0.5f)), s(10.0f), a(1.0f) {};
    glm::vec3 ka = glm::vec3(0.5f);
    glm::vec3 kd = glm::vec3(0.5f);
    glm::vec3 ks = glm::vec3(0.5f);
    float s = 10.0f;
    float a = 1.0f;
};

#endif