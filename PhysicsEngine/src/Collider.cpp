#include "Collider.h"
#include <iostream> 

#define TOL 1e-3f

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

bool operator==(const Edge& e1, const Edge& e2) {
    return (e1.v1 == e2.v1 && e1.v2 == e2.v2) || (e1.v1 == e2.v2 && e1.v2 == e2.v1);
}

void insert_nonredundant_edge(Edge& edge, std::vector<Edge>& edges) {
    for (int i = 0; i < edges.size(); ++i) {
		if (edge == edges[i]) return;
	}
    edges.push_back(edge);
}


Collider::Collider(vectors posBuf) {
    verts = posBuf;
    for (int i = 0; i < posBuf.size(); i+=3) {
        normals.push_back(glm::normalize(glm::cross(posBuf[i+1] - posBuf[i], posBuf[i+2] - posBuf[i+1])));
    }
}


vec3 get_farpoint(const vectors& verts, const vec3 dir) {
    vec3 maxPoint;
    float maxDistance = -FLT_MAX;
    float distance;
    for (vec3 v : verts) {
        distance = glm::dot(v, dir);
        if (distance > maxDistance) {
            maxDistance = distance;
            maxPoint = v;
        }
    }
    return maxPoint;
}

bool same_dir(const vec3& dir, const vec3& ao){
	return glm::dot(dir, ao) > 0.0f;
}

vec3 get_support(const vectors& verts1, const vectors& verts2, vec3 dir){
	return  get_farpoint(verts1, dir) - get_farpoint(verts2, -dir);
}

bool two_points(Simplex& points, vec3& dir){
    vec3 a,ab,ao;

	a = points[0];
	ab = points[1] - a;
	ao = -a;
 
	if (same_dir(ab, ao)) {
		dir = glm::cross(glm::cross(ab, ao), ab);
	} else {
		points = { a };
		dir = ao;
	}

	return false;
}

bool three_points(Simplex& points,vec3& dir){
	vec3 a = points[0];
	vec3 b = points[1];
	vec3 c = points[2];

	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 ao =   - a;
 
	vec3 abc = glm::cross(ab, ac);
 
	if (same_dir(glm::cross(abc, ac), ao)) {
		if (same_dir(ac, ao)) {
			points = { a, c };
			dir = glm::cross(glm::cross(ac, ao), ac); 
		}

		else {
			return two_points(points = { a, b }, dir);
		}
	}
 
	else {
		if (same_dir(glm::cross(ab, abc), ao)) {
			return two_points(points = { a, b }, dir);
		}

		else {
			if (same_dir(abc, ao)) {
				dir = abc;
			}

			else {
				points = { a, c, b };
				dir = -abc;
			}
		}
	}

	return false;
}

bool four_points(Simplex& points, vec3& direction){
	vec3 a = points[0];
	vec3 b = points[1];
	vec3 c = points[2];
	vec3 d = points[3];

	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 ad = d - a;
	vec3 ao =   - a;
 
	vec3 abc = glm::cross(ab, ac);
	vec3 acd = glm::cross(ac, ad);
	vec3 adb = glm::cross(ad, ab);
 
	if (same_dir(abc, ao)) {
		return three_points(points = { a, b, c }, direction);
	}
		
	if (same_dir(acd, ao)) {
		return three_points(points = { a, c, d }, direction);
	}
 
	if (same_dir(adb, ao)) {
		return three_points(points = { a, d, b }, direction);
	}
 
	return true;
}

bool enclose_origin(Simplex& points, vec3& dir){
	switch (points.size()) {
		case 2: return two_points(points, dir);
		case 3: return three_points(points, dir);
		case 4: return four_points(points, dir);
	}
 
	// never should be here
	return false;
}


Contact GJK(Collider& colliderA, Collider& colliderB, const glm::mat4& T1, const glm::mat4& T2) {
    Contact contact;
    Simplex points;
    vectors verts1, verts2;
    vec3 support, dir;

    verts1.reserve(colliderA.verts.size()); 
    verts2.reserve(colliderB.verts.size());

    for (auto v : colliderA.verts) 
        verts1.push_back(T1 * glm::vec4(v, 1.0f));
    for (auto v : colliderB.verts) 
        verts2.push_back(T2 * glm::vec4(v, 1.0f));
    

    support = get_support(verts1, verts2, glm::vec3(1.0f, 0.0f, 0.0f));
    points.push_front(support);
    dir = -support;

    while (true) {
        support = get_support(verts1, verts2, dir);
        
		if (glm::dot(support, dir) <= 0.0f) 
			return contact; // no collision
		
		points.push_front(support);
        
        if (enclose_origin(points, dir)) 
			return EPA(points, verts1, verts2);
    }
}


std::pair<std::vector<glm::vec4>, size_t> get_face_normals(const vectors& polytope, const std::vector<size_t>&  faces) {
	std::vector<glm::vec4> normals;
	size_t minTriangle = 0;
	float  minDistance = FLT_MAX;

	for (size_t i = 0; i < faces.size(); i += 3) {
		vec3 a = polytope[faces[i    ]];
		vec3 b = polytope[faces[i + 1]];
		vec3 c = polytope[faces[i + 2]];

		vec3 normal = glm::normalize(glm::cross(b - a, c - a));
		float distance = glm::dot(normal, a);

		if (distance < 0.0f) {
			normal   *= -1.0f;
			distance *= -1.0f;
		}

		normals.emplace_back(normal, distance);

		if (distance < minDistance) {
			minTriangle = i / 3;
			minDistance = distance;
		}
	}

	return { normals, minTriangle };
}

void add_unique_edge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces, size_t a, size_t b){
	auto reverse = std::find(               //      0--<--3
		edges.begin(),                     //     / \ B /   A: 2-0
		edges.end(),                       //    / A \ /    B: 0-2
		std::make_pair(faces[b], faces[a]) //   1-->--2
	);
 
	if (reverse != edges.end()) {
		edges.erase(reverse);
	}
 
	else {
		edges.emplace_back(faces[a], faces[b]);
	}
}

vec3 barycentric(vec3 p, vec3 a, vec3 b, vec3 c){
    vec3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;
    return vec3(u,v,w);
}

Contact EPA(const Simplex& simplex, const vectors& verts1, const vectors& verts2){
    Contact contact;
    vectors polytope(simplex.begin(), simplex.end());
	std::vector<size_t>  faces = {
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};

	// list: vector4(normal, distance), index: min distance
	auto [normals, minFace] = get_face_normals(polytope, faces);
    vec3 minNormal;
	float minDistance = FLT_MAX;
	
	while (minDistance == FLT_MAX) {
		minNormal   = normals[minFace];
		minDistance = normals[minFace].w;
 
		vec3 support = get_support(verts1, verts2, minNormal);
		float sDistance = glm::dot(minNormal, support);
 
		if (glm::abs(sDistance - minDistance) > 0.0000001f) {
			minDistance = FLT_MAX;
            std::vector<std::pair<size_t, size_t>> uniqueEdges;

			for (size_t i = 0; i < normals.size(); i++) {
				if (same_dir(normals[i], support)) {
					size_t f = i * 3;

					add_unique_edge(uniqueEdges, faces, f,     f + 1);
					add_unique_edge(uniqueEdges, faces, f + 1, f + 2);
					add_unique_edge(uniqueEdges, faces, f + 2, f    );

					faces[f + 2] = faces.back(); faces.pop_back();
					faces[f + 1] = faces.back(); faces.pop_back();
					faces[f    ] = faces.back(); faces.pop_back();

					normals[i] = normals.back(); normals.pop_back();

					i--;
				}
			}
            std::vector<size_t> newFaces;
			for (auto [edgeIndex1, edgeIndex2] : uniqueEdges) {
				newFaces.push_back(edgeIndex1);
				newFaces.push_back(edgeIndex2);
				newFaces.push_back(polytope.size());
			}
			 
			polytope.push_back(support);

			auto [newNormals, newMinFace] = get_face_normals(polytope, newFaces);
            float oldMinDistance = FLT_MAX;
			for (size_t i = 0; i < normals.size(); i++) {
				if (normals[i].w < oldMinDistance) {
					oldMinDistance = normals[i].w;
					minFace = i;
				}
			}
 
			if (newNormals[newMinFace].w < oldMinDistance) {
				minFace = newMinFace + normals.size();
			}
 
			faces  .insert(faces  .end(), newFaces  .begin(), newFaces  .end());
			normals.insert(normals.end(), newNormals.begin(), newNormals.end());
		} else {
            vec3 v1 = polytope[faces[minFace*3]];
            vec3 v2 = polytope[faces[minFace*3+1]];
            vec3 v3 = polytope[faces[minFace*3+2]];

            vec3 p = -1.0f*minNormal * sDistance;

            vec3 c = barycentric(p,v1,v2,v3);
            
            contact.p = p;
            std::cout<<"v1 <"<<v1.x<<", "<<v1.y<<", "<<v1.z<<">\n";
            std::cout<<"v2 <"<<v2.x<<", "<<v2.y<<", "<<v2.z<<">\n";
            std::cout<<"v3 <"<<v3.x<<", "<<v3.y<<", "<<v3.z<<">\n";
            std::cout<<p.x<<", "<<p.y<<", "<<p.z<<std::endl;
        }
	}
	contact.normal = minNormal;
	contact.depth = minDistance + 0.0000001f;
	contact.exists = true;
 
	return contact;
}


