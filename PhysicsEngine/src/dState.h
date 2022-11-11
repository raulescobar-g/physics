#pragma once
#ifndef DSTATE_H
#define DSTATE_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "State.h"

class dState {
    public:
        dState(State state, glm::vec3 F_ext, glm::vec3 tau_ext=glm::vec3(0.0f)){
            glm::mat3 R = glm::mat3_cast(state.q);
            v = state.P / state.mass;
            glm::quat w(0.0f, R*state.I_inv*glm::transpose(R) * state.L);
            wq = 0.5f * w * state.q;
            F = F_ext;
            Tau = tau_ext;
        };
        glm::vec3 v;
        glm::quat wq;
        glm::vec3 F;
        glm::vec3 Tau;

        State operator*(float h) {
            State integral;
            integral.x = v * h;
            integral.q = wq * h;
            integral.P = F * h;
            integral.L = Tau * h;
            return integral;
        }
};

#endif