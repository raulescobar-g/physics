#pragma once

#ifndef COLLIDER_H
#define COLLIDER_H

#include "GeometryUtil.h"
#include "Mesh.h"
#include "Transform.h"
#include <memory>
#include "Simplex.h"

using vectors = std::vector< glm::vec3 >;
using vec3 = glm::vec3;
using pairs = std::vector<std::pair<unsigned int, unsigned int>>;
using triplets = std::vector<std::tuple<unsigned int, unsigned int, unsigned int>>;
struct Contact {
    Contact() : exists(false) {}
    vec3 p, normal;
    float depth;
    bool exists;

    operator bool() { return exists; }
};

using contact_pair = std::tuple<std::vector<Contact>,std::vector<Contact>>;


struct Collider {
    Collider(const vectors& verts);
    vectors verts;
    triplets triangle_idx;
    pairs edges;
};


contact_pair are_colliding(Collider& meshA, Collider& meshB, const glm::mat4& TA, const glm::mat4& pTA, const glm::mat4& TB, const glm::mat4& pTB);

void check_vertices_against_faces(Collider& meshA, Collider& meshB, vectors& pvA, vectors& vA, vectors& pvB, vectors& vB, std::vector<Contact>& contactsA, std::vector<Contact>& contactsB);

void check_edges(Collider& meshA, Collider& meshB, vectors& pvA, vectors& vA, vectors& pvB, vectors& vB, std::vector<Contact>& contactsA, std::vector<Contact>& contactsB);

void vertex_triangle_collision(vec3 prev, vec3 p, vec3 v1, vec3 v2, vec3 v3, std::vector<Contact>& contacts_point, std::vector<Contact>& contacts_face);

bool inside(vec3 collision_position , vec3 vertex_0, vec3 vertex_1, vec3 vertex_2);

bool triangle_hit(vec3 prev, vec3 p, vec3 v1, vec3 v2, vec3 v3, vec3& contact);

#endif 