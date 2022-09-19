#pragma once

#ifndef OBJECT_H
#define OBJECT_H
#include <stdlib.h>  
#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Material.h"
#include "Shape.h"

class Object {
    public: 
        Object(std::shared_ptr<Material> m, std::shared_ptr<Shape> s, glm::vec3 position, glm::vec3 rotation, glm::vec3 velocity, bool dynamic, float scale=1.0f) {
            material = m;
            shape = s;
            this->scale = scale;
            pos = position;
            this->rotation = rotation;
            this->velocity = velocity;
            this->dynamic = dynamic; 
            mass = 5.0f;
            sleeping = !dynamic;
            drag_coeff = 0.3f;
            restitution = 0.5f;
            mu = 0.2f;
        }

        bool dynamic, sleeping;
        float drag_coeff, restitution, mu, mass, scale;
        glm::vec3 pos, velocity, rotation;
        std::shared_ptr<Material> material;
        std::shared_ptr<Shape> shape;
};

#endif