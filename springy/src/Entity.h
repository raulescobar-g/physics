#pragma once

#ifndef ENTITY_H
#define ENTITY_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


struct box {
    glm::vec3 vmin, vmax;
};

class Entity {
    Entity();
    ~Entity();
    virtual void update(float dt);
    virtual void draw();
};

#endif