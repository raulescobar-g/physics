#pragma once
#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "Entity.h"
#include "SoftBody.h"
#include "StaticBody.h"


bool inside(glm::vec3 collision_position , glm::vec3 vertex_0, glm::vec3 vertex_1, glm::vec3 vertex_2) {
	glm::vec3 u = vertex_1 - vertex_0;
	glm::vec3 v = vertex_2 - vertex_0;
	glm::vec3 n = glm::cross(u, v);
	glm::vec3 w = collision_position - vertex_0;

	float gamma = glm::dot(glm::cross(u, w), n) / glm::dot(n,n);
	float beta = glm::dot(glm::cross(w, v), n) / glm::dot(n,n);
	float alpha = 1.0 - gamma - beta;

	return alpha >= -0.00001 && beta >= -0.00001 && gamma >= -0.00001;
};


void collision_response(std::shared_ptr<SoftBody> soft_body, std::shared_ptr<StaticBody> static_body, float dt ) {

    for (int i = 0; i < soft_body->size()*3; i+=3) {
        glm::vec3 p(soft_body->positions[i], soft_body->positions[i+1], soft_body->positions[i+2]);
        glm::vec3 prev_p(soft_body->prev_positions[i], soft_body->prev_positions[i+1], soft_body->prev_positions[i+2]); // add this
        glm::vec3 v = glm::vec3(soft_body->particles[i].velocity);
        glm::vec3 prev_v = glm::vec3(soft_body->prev_velocity[i]); // add this

        std::vector<float> posBuf = static_body->get_posbuf();
        for (int j = 0; j < posBuf.size(); j+=9) {
            glm::vec4 _v1(posBuf[i], posBuf[i+1], posBuf[i+2], 1.0f);
            glm::vec4 _v2(posBuf[i+3], posBuf[i+4], posBuf[i+5], 1.0f);
            glm::vec4 _v3(posBuf[i+6], posBuf[i+7], posBuf[i+8], 1.0f);

            auto MV = static_body->get_transform();

            glm::vec3 v0 = glm::vec3(MV->topMatrix() * _v1);
            glm::vec3 v1 = glm::vec3(MV->topMatrix() * _v2);
            glm::vec3 v2 = glm::vec3(MV->topMatrix() * _v3);

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            float d_A = glm::dot(prev_p - v0, normal);
            float d_B = glm::dot(p - v0, normal);

            float f = abs(d_A) / abs(d_B - d_A);
            glm::vec3 collision_position = prev_p + prev_v * dt * f;

            if ( d_B * d_A < 0.0 && inside(collision_position, v0, v1, v2)){
                next_position = next_position - (1 + cr) * d_B * normal;

                glm::vec3 velocity_normal = glm::dot(next_velocity , normal) * normal;
                glm::vec3 velocity_tangent = next_velocity - velocity_normal;
                next_velocity = -cr * velocity_normal + (1 - cf) * velocity_tangent;
                break;
			}
        }
    }
}

// void collision_response(std::shared_ptr<SoftBody> soft_bodyA, std::shared_ptr<SoftBody> soft_bodyB ) {

// }

void collision_response(std::shared_ptr<StaticBody> entityA, std::shared_ptr<SoftBody> entityB, float dt ) {
    collision_response(entityB, entityA, dt);
}



#endif