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

struct Contact {
    Contact() : exists(false) {}
    vec3 p, normal;
    float depth;
    bool exists;

    operator bool() { return exists; }
};

struct Triangle {
    Triangle(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int n1): v1(i1), v2(i2), v3(i3), n1(n1) {}
    unsigned int v1,v2,v3, n1;
};
struct Edge {
    Edge(unsigned int i1, unsigned int i2): v1(i1), v2(i2) {}
    unsigned int v1,v2;
};

struct Collider {
    Collider(vectors verts);
    vectors verts;
    vectors normals;
};

vec3 get_farpoint(const vectors& verts, const vec3 dir);

bool same_dir(const vec3& dir, const vec3& ao);

vec3 get_support(const vectors& verts1, const vectors& verts2, vec3 dir);

bool two_points(Simplex& points, vec3& dir);

bool three_points(Simplex& points,vec3& dir);

bool four_points(Simplex& points, vec3& direction);

bool enclose_origin(Simplex& points, vec3& dir);

Contact GJK(Collider& colliderA, Collider& colliderB, const glm::mat4& T1, const glm::mat4& T2);

std::pair<std::vector<glm::vec4>, size_t> get_face_normals(const vectors& polytope, const std::vector<size_t>&  faces);

void add_unique_edge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces, size_t a, size_t b);

Contact EPA(const Simplex& simplex, const vectors& verts1, const vectors& verts);

vec3 barycentric(vec3 p, vec3 a, vec3 b, vec3 c);

#endif 