#pragma  once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

class Material {
    public:
        Material(){};
        Material(glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float s): ka(ka), kd(kd), ks(ks), s(s) {};
        glm::vec3 ka;
        glm::vec3 kd;
        glm::vec3 ks;
        float s;

};
#endif
