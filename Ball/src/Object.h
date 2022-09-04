#pragma once

#ifndef OBJECT_H
#define OBJECT_H
#include <stdlib.h>  
#include <time.h> 
#include <memory>


class Object {
    public: 
        

        // defined objects for the HUD
        Object(std::shared_ptr<Material> m, std::shared_ptr<Shape> s, glm::vec3 position, glm::vec3 rotation, float scale=1.0f) {
            material = m;
            shape = s;
            this->scale = scale;
            pos = position;
            this->rotation = rotation;
        }

        
        glm::vec3 pos;
        glm::vec3 rotation;
        float scale;
        std::shared_ptr<Material> material;
        std::shared_ptr<Shape> shape;
};

#endif