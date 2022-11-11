#pragma once
#ifndef STATE_H
#define STATE_H

#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

typedef enum Type: unsigned char {
    STATIC,
    DYNAMIC,
    KINEMATIC
};

// struct Box {
//     Box(glm::vec3 i, glm::vec3 a): mini(i - glm::vec3(0.0001f)), maxi(a + glm::vec3(0.0001f)){};
//     glm::vec3 mini, maxi;

//     void operator*=(glm::mat4 transform) {
//         // mini = transform * glm::vec4(mini, 1.0f);
//         // maxi = transform * glm::vec4(maxi, 1.0f);

//         for (int i = 0; i < 3; ++i) {

//         }
//     }
// };

class State {
    public:
        State(){};
        State(glm::vec3 x, glm::vec3 v, float mass): x(x), q(glm::quat(1.0f, glm::vec3(0.0f))), P(mass*v), L(glm::vec3(0.0f)), mass(mass), scale(1.0f) {
            type = mass <= 0.0f ? STATIC: DYNAMIC;
        };
        glm::vec3 x;
        glm::quat q;
        glm::vec3 P;
        glm::vec3 L;
        float mass;
        float scale;
        glm::mat3 I_inv;
        Type type;

        State operator+(State const& other_state){
            State new_state(*this);
            new_state.x = this->x + other_state.x;
            new_state.q = this->q + other_state.q;
            new_state.P = this->P + other_state.P;
            new_state.L = this->L + other_state.L;
            new_state.q = glm::normalize(this->q);
            return new_state;
        }

        operator glm::mat4() {
            return glm::translate(glm::mat4(1.0f), x)* glm::scale(glm::mat4(1.0f), glm::vec3(scale))* glm::toMat4(q);
        }

};

std::ostream &operator<<(std::ostream &os,  const State& s) { 
    os <<"----------\nposition: <" << s.x.x<< ", " << s.x.y<< ", " << s.x.z<< ">\n";
    os << "velocity: <" << s.P.x / s.mass<< ", " << s.P.y / s.mass<< ", " << s.P.z / s.mass<< ">\n";
    return os;
}

#endif