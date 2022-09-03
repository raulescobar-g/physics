#pragma once

#ifndef OBJECT_H
#define OBJECT_H
#include <stdlib.h>  
#include <time.h> 
#include <memory>


class Object {
    public: 
        

        // defined objects for the HUD
        Object(std::shared_ptr<Material> m, std::shared_ptr<Shape> s, glm::vec3 position, float _scale) {
            material = m;
            shape = s;
            scale = _scale;
            x = position.x;
            y = position.y;
            z = position.z;
            rotation = 0.0f;
        }

        
        float x;
        float z;
        float y;

        float rotation;
        float scale;
        std::shared_ptr<Material> material;
        std::shared_ptr<Shape> shape;
};

#endif