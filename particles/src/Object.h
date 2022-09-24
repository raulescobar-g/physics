#pragma once

#ifndef OBJECT_H
#define OBJECT_H
#include <stdlib.h>  
#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shape.h"

class Object {
    public: 
        Object(std::shared_ptr<Shape> s, glm::vec3 position) {
            shape = s;
            this->scale = 1.0f;
            pos = position;
            this->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        void set_scale(float obj_scale) {
            scale = obj_scale;
        }
        void set_rotation(glm::vec3 rot) {
            rotation = rot;
        }

        float scale;
        glm::vec3 pos, rotation;
        std::shared_ptr<Shape> shape;
};

#endif