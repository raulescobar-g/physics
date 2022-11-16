#include "Transform.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

StateVector::StateVector(glm::vec3 x): x(x), q(1.0f,glm::vec3(0.0f)), p(0.0f), L(0.0f), scale(1.0f) {}
StateVector::StateVector(glm::vec3 x,glm::vec3 p,glm::vec3 L) : x(x), q(1.0f,glm::vec3(0.0f)), p(p), L(L), scale(1.0f) {}
StateVector::StateVector(const StateVector& s) : x(s.x), q(s.q), p(s.p), L(s.L), scale(s.scale) {}
StateVector::operator glm::mat4(){ return glm::translate(glm::mat4(1.0f), x) * glm::scale(glm::mat4(1.0f), scale) * glm::toMat4(q);}

void Inertia::operator=(float m) {mass=m;}
void Inertia::operator=(glm::mat3 I) {I_inv=glm::inverse(I);}
Inertia::operator float() {return mass;}
Inertia::operator glm::mat3() {return I_inv;}
Inertia::Inertia(): mass(1.0f){}
Inertia::Inertia(float mass) : mass(mass) {} 

StateVector StateVector::operator+(const StateVector& rhs_state){
    StateVector new_state(*this);

    new_state.x = rhs_state.x + x; 
    new_state.q = glm::normalize(rhs_state.q + q); 
    new_state.p = rhs_state.p + p; 
    new_state.L = rhs_state.L + L; 

    return new_state;
}

dStateVector::dStateVector() : v(0.0f),wq(1.0f,glm::vec3(0.0f)), F(0.0f), T(0.0f) {}
dStateVector::dStateVector(glm::vec3 v) : v(v),wq(1.0f,glm::vec3(0.0f)), F(0.0f), T(0.0f) {}
dStateVector::dStateVector(const StateVector& state, Inertia& inertia, glm::vec3 a){
    glm::mat3 I_inv = inertia;
    float mass = inertia;

    glm::mat3 R = glm::mat3_cast(state.q);
    glm::quat w(0.0f, R*I_inv*glm::transpose(R) * state.L);

    v = state.p / mass;
    wq = 0.5f * w * state.q;
    F = a * mass;
    T = glm::vec3(0.0f);
}

StateVector dStateVector::operator*(float h) {
    StateVector dstate;
    dstate.x = v * h;
    dstate.q = wq * h;
    dstate.p = F * h;
    dstate.L = T * h;
    return dstate;
}

Transform::Transform(glm::vec3 translation, 
                    glm::vec3 velocity, 
                    float mass) : 
                    state(translation), 
                    dstate(velocity),
                    inertia(mass){};

Transform::Transform(glm::vec3 translation) : 
                    state(translation), 
                    dstate(),
                    inertia() {};

//Transform::Transform(const Transform&) 


Transform::operator glm::mat4() { return state;}
void Transform::operator=(const StateVector& s){ state = s;}

StateVector integrate_state(Transform& transform, float h, glm::vec3 a) {
    float mass = transform.inertia;
    a -= (0.1f / mass) * (transform.state.p / mass);
    dStateVector dstate(transform.state, transform.inertia, a);
    return transform.state + (dstate * h);
}

