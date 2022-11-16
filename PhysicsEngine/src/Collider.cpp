#include "Collider.h"
#include <iostream> 

#define TOL 1e-2f

bool operator==(glm::vec3 v1, glm::vec3 v2) {
	return glm::length(v1 - v2) < TOL;
}

int vec_idx(glm::vec3 p, std::vector<glm::vec3>& vecs) {
	for (int i = 0; i < vecs.size(); ++i) {
		if (p == vecs[i]) return i;
	}
    vecs.push_back(p);
	return vecs.size()-1;
}

bool operator==(const std::pair<unsigned int, unsigned int>& pair1, const std::pair<unsigned int, unsigned int>& pair2) {
    auto [e1,e2] = pair1;
    auto [d1, d2] = pair2;
    return (e1 == d1 && e2 == d2) || (e1 == d2 && e2 == d1);
}

void insert_nonredundant_edge(std::pair<unsigned int, unsigned int> edge, pairs& edges) {
    for (int i = 0; i < edges.size(); ++i) {
		if (edge == edges[i]) return;
	}
    edges.push_back(edge);
}


Collider::Collider(const vectors& posBuf) {
    
    for (int i = 0; i < posBuf.size(); i+=3) {
        
        unsigned int idx1 = vec_idx(posBuf[i], verts);
        unsigned int idx2 = vec_idx(posBuf[i+1], verts);
        unsigned int idx3 = vec_idx(posBuf[i+2], verts);

        triangle_idx.push_back({idx1, idx2, idx3});

        insert_nonredundant_edge(std::pair<unsigned int, unsigned int>(idx1, idx2), edges);
        insert_nonredundant_edge(std::pair<unsigned int, unsigned int>(idx2, idx3), edges);
        insert_nonredundant_edge(std::pair<unsigned int, unsigned int>(idx3, idx1), edges);
    }
   

}


contact_pair are_colliding(Collider& meshA, Collider& meshB, const glm::mat4& TA, const glm::mat4& pTA, const glm::mat4& TB, const glm::mat4& pTB){
    std::vector<Contact> contacts_on_A, contacts_on_B;
    vectors prev_verts_A, verts_A, prev_verts_B, verts_B;

    prev_verts_A.reserve(meshA.verts.size());
    verts_A.reserve(meshA.verts.size());
    prev_verts_B.reserve(meshB.verts.size());
    verts_B.reserve(meshB.verts.size());

    for (auto v : meshA.verts) {
        prev_verts_A.push_back(pTA * glm::vec4(v, 1.0f));
        verts_A.push_back(TA * glm::vec4(v, 1.0f));
    }

    for (auto v : meshB.verts) {
        prev_verts_B.push_back(pTB * glm::vec4(v, 1.0f));
        verts_B.push_back(TB * glm::vec4(v, 1.0f));
    }
    check_vertices_against_faces(meshA, meshB, prev_verts_A, verts_A, prev_verts_B, verts_B, contacts_on_A, contacts_on_B);
    check_vertices_against_faces(meshB, meshA, prev_verts_B, verts_B, prev_verts_A, verts_A, contacts_on_B, contacts_on_A);
    check_edges(meshA, meshB, prev_verts_A, verts_A, prev_verts_B, verts_B, contacts_on_A, contacts_on_B);

    return { contacts_on_A, contacts_on_B };
}  

void check_vertices_against_faces(Collider& meshA, Collider& meshB, vectors& pvA, vectors& vA, vectors& pvB, vectors& vB, std::vector<Contact>& contactsA, std::vector<Contact>& contactsB) {
    for (int i = 0; i < vA.size(); ++i) {
        for (int j = 0; j < meshB.triangle_idx.size(); ++j) {
            vec3 v1,v2,v3;

            auto [idx1, idx2, idx3] = meshB.triangle_idx[j];
            
            v1 = vB[idx1]; v2 = vB[idx2]; v3 = vB[idx3];

            vertex_triangle_collision(pvA[i], vA[i], v1, v2, v3, contactsA, contactsB);
        }
    }
}

glm::vec4 closest_point(vec3 q0, vec3 q1, vec3 p0, vec3 p1) {
    vec3 r = q0 - p0;

    vec3 a = p1 - p0;
    vec3 b = q1 - q0;

    if (glm::abs(glm::dot(glm::normalize(a),glm::normalize(b))) > 0.99f) return glm::vec4(0.0f,0.0f,0.0f,-1.0f);

    vec3 n = glm::normalize(glm::cross(a,b));

    vec3 b_cross_n =  glm::cross(glm::normalize(b), glm::normalize(n));
    vec3 a_cross_n  = glm::cross(glm::normalize(a), glm::normalize(n));

    float s = glm::dot(r,b_cross_n) / glm::dot(a,b_cross_n );

    float t = glm::dot(-r, a_cross_n) / glm::dot(b, a_cross_n );

    if (s < 0.0001f || t < 0.0001f || s > 0.999f || t > 0.999f) return glm::vec4(0.0f,0.0f,0.0f,-1.0f);

    vec3 qa = q0 + t * b;
    vec3 pa = p0 + s * a;

    return glm::vec4(qa - pa, t);
}

void check_edges(Collider& meshA, Collider& meshB, vectors& pvA, vectors& vA, vectors& pvB, vectors& vB, std::vector<Contact>& contactsA, std::vector<Contact>& contactsB){

    for (auto [e1_1, e1_2] : meshA.edges) {

        vec3 edge1_1 = vA[e1_1];
        vec3 edge1_2 = vA[e1_2];
        vec3 prev_edge1_1 = pvA[e1_1];
        vec3 prev_edge1_2 = pvA[e1_2];

        for (auto [e2_1, e2_2] : meshB.edges) {
            

            vec3 edge2_1 = vB[e2_1];
            vec3 edge2_2 = vB[e2_2];
            vec3 prev_edge2_1 = pvB[e2_1];
            vec3 prev_edge2_2 = pvB[e2_2];

            glm::vec4 _m = closest_point(edge1_1, edge1_2, edge2_1, edge2_2);
            glm::vec4 _prev_m = closest_point(prev_edge1_1, prev_edge1_2, prev_edge2_1, prev_edge2_2);

            float t = _m.w;
            float pt = _prev_m.w;

            vec3 m = _m;
            vec3 prev_m = _prev_m;

            if ((t < 0.0f || pt < 0.0f) || glm::dot(m, prev_m) > 0.0f) continue;
            std::cout<<"-";

            vec3 n = glm::normalize(glm::cross(edge1_2 - edge1_1,edge2_2 - edge2_1));

            float s = glm::dot(m,n) > 0.0f ? -1.0f: 1.0f;

            Contact contactA, contactB;
            contactA.exists = true;
            contactB.exists = true;
            contactA.normal = s * n;
            contactB.normal = -s * n;

            contactA.p = edge1_1 + (edge1_2 - edge1_1) * t;
            contactB.p = contactA.p;

            contactsA.push_back(contactA);
            contactsB.push_back(contactB);
        }
    }
}


void vertex_triangle_collision(vec3 prev, vec3 p, vec3 v1, vec3 v2, vec3 v3, std::vector<Contact>& contacts_point, std::vector<Contact>& contacts_face) {
    vec3 point_of_collision;
    if (triangle_hit(prev, p, v1, v2, v3, point_of_collision)) {
        std::cout<<".";
        Contact point, face;
        point.normal = glm::cross(v2 - v1, v3 - v1);
        point.exists = true;
        point.p = point_of_collision;

        face.normal = -1.0f * point.normal;
        face.exists = true;
        face.p = point_of_collision;

        contacts_point.push_back(point);
        contacts_face.push_back(face);
    }
}

bool inside(vec3 collision_position , vec3 vertex_0, vec3 vertex_1, vec3 vertex_2) {
	vec3 u = vertex_1 - vertex_0;
	vec3 v = vertex_2 - vertex_0;
	vec3 n = glm::cross(u, v);
	vec3 w = collision_position - vertex_0;

	float gamma = glm::dot(glm::cross(u, w), n) / glm::dot(n,n);
	float beta = glm::dot(glm::cross(w, v), n) / glm::dot(n,n);
	float alpha = 1.0f - gamma - beta;

	return alpha >= -0.00001 && beta >= -0.00001 && gamma >= -0.00001;
};

bool triangle_hit(vec3 prev, vec3 p, vec3 v1, vec3 v2, vec3 v3, vec3& contact) {
   vec3 n = glm::normalize(glm::cross(v2 - v1, v3 - v1));

    float d_A = glm::dot(prev - v1, n);
    float d_B = glm::dot(p - v1, n);

    float t = glm::abs(d_A) / glm::abs(d_B - d_A);

    contact = prev + (p-prev) * glm::length(p-prev) * t;

    return d_B * d_A < 0.0 && inside(contact, v1, v2, v3);
}