#pragma  once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

class Material {
    public:
        Material(){
            glm::vec3 _default(0.0f, 0.0f, 0.0f);
            ka = _default;
            kd = _default;
            ks = _default;
            s = 0.0f;
        };
        glm::vec3 ka;
        glm::vec3 kd;
        glm::vec3 ks;
        float s;
};
#endif
