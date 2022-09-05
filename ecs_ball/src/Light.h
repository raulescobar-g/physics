#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Light {
    public:
        Light(glm::vec3 position) : position(position){};
        glm::vec3 position;
};


#endif