#pragma once

#ifndef OBJECT_H
#define OBJECT_H
#include <stdlib.h>  
#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shape.h"
#include "Material.h"

class Object {
    public: 
        Object() {
            glm::vec3 _default(0.0f, 0.0f, 0.0f);
            position = _default;
            rotation = _default;
            scale = glm::vec3(1.0f, 1.0f, 1.0f);
        };

        glm::vec3 position, rotation, scale;
        std::shared_ptr<Shape> shape;
        std::shared_ptr<Material> material;
};

#endif