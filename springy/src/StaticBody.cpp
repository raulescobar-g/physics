#include "StaticBody.h"
#include "Softbody.h"
#include <iostream>

bool inside2(glm::vec3 collision_position , glm::vec3 vertex_0, glm::vec3 vertex_1, glm::vec3 vertex_2) {
	glm::vec3 u = vertex_1 - vertex_0;
	glm::vec3 v = vertex_2 - vertex_0;
	glm::vec3 n = glm::cross(u, v);
	glm::vec3 w = collision_position - vertex_0;

	float gamma = glm::dot(glm::cross(u, w), n) / glm::dot(n,n);
	float beta = glm::dot(glm::cross(w, v), n) / glm::dot(n,n);
	float alpha = 1.0f - gamma - beta;

	return alpha >= -0.00001f && beta >= -0.00001f && gamma >= -0.00001f;
};

void StaticBody::update(float dt, const glm::vec3& a){
    return;
}

void StaticBody::collision_response(std::shared_ptr<SoftBody> other,float dt) {
    float cr = 0.99999f;
    float cf = 0.1f;
    for (int i = 0; i < other->size()*3; i+=3) {
        int gid = i/3;
        glm::vec3 p(other->positions[i], other->positions[i+1], other->positions[i+2]);
        glm::vec3 prev_p(other->past_positions[i], other->past_positions[i+1], other->past_positions[i+2]);
        glm::vec3 v = glm::vec3(other->particles[gid].velocity);
        glm::vec3 prev_v = glm::vec3(other->past_velocities[i], other->past_velocities[i+1], other->past_velocities[i+2]);

        std::vector<float> posBuf = get_posbuf();
        for (int j = 0; j < posBuf.size(); j+=9) {
            glm::vec4 _v1(posBuf[j], posBuf[j+1], posBuf[j+2], 1.0f);
            glm::vec4 _v2(posBuf[j+3], posBuf[j+4], posBuf[j+5], 1.0f);
            glm::vec4 _v3(posBuf[j+6], posBuf[j+7], posBuf[j+8], 1.0f);
            
            glm::mat4 MV(1.0f);
			MV = glm::translate(MV, position);
			MV = glm::scale(MV, scale);
			if (glm::length(rotation) > 0.001f) MV = glm::rotate(MV, glm::length(rotation), rotation);
            

            glm::vec3 v0 = glm::vec3(MV * _v1);
            glm::vec3 v1 = glm::vec3(MV * _v2);
            glm::vec3 v2 = glm::vec3(MV * _v3);

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            float d_A = glm::dot(prev_p - v0, normal);
            float d_B = glm::dot(p - v0, normal);

            float f = abs(d_A) / abs(d_B - d_A);
            glm::vec3 collision_position = prev_p + prev_v * dt * f;

            if ( d_B * d_A < 0.0 && inside2(collision_position, v0, v1, v2)){
                glm::vec3 dp =  (1.0f + cr) * d_B * normal;
                other->positions[i] -= dp.x;
                other->positions[i+1] -= dp.y;
                other->positions[i+2] -= dp.z;

                glm::vec3 velocity_normal = glm::dot(v , normal) * normal;
                glm::vec3 velocity_tangent = v - velocity_normal;

                other->particles[gid].velocity = glm::vec4(-cr * velocity_normal + (1 - cf) * velocity_tangent, 0.0f);
			}
        }
    }
}