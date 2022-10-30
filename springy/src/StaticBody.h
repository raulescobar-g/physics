#pragma once
#ifndef STATICBODY_H
#define STATICBODY_H

#include "Entity.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


class StaticBody: public Entity {
    public:
        using Entity::Entity;
        void update(float dt, const glm::vec3& a=glm::vec3(0.0f)) override;

        std::vector<float> get_posbuf() { return posBuf; }
};

#endif