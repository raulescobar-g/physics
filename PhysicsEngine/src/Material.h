#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct Material {
    Material();
    Material(glm::vec3);
    glm::vec3 ka,kd,ks;
    float s,a;
};

#endif