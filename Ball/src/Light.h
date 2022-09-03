#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Light {
    public:
        Light(std::string name, glm::vec3 position) : pos_name(name), position(position){};
        std::string pos_name;
        glm::vec3 position;
        
};


#endif