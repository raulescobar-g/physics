#include "StaticBody.h"
#include "Softbody.h"
#include <iostream>


void StaticBody::update(float dt, const glm::vec3& a){
    position = position + constant_velocity * dt;
}

