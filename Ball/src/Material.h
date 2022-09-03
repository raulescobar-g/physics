#pragma  once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

class Material {
    public:
        Material(glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float s);
        glm::vec3 ka;
        glm::vec3 kd;
        glm::vec3 ks;
        float s;

};
#endif
