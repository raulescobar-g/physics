#include "Softbody.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "tiny_obj_loader.h"
#include <iostream>
#include <unordered_map>

#include "StaticBody.h"


void SoftBody::calculate_collision_response(std::shared_ptr<StaticBody> obj, float dt) {
    int div = S.size()/2;
    for (int i = 0; i < S.size()/2; ++i) {
        glm::vec3 p = S[i];
        glm::vec3 prev_p = prev_S[i];

        glm::vec3 v = S[div+i];
        glm::vec3 prev_v = prev_S[div+i];

        std::vector<float> posBuf = obj->get_posbuf();
        for (int j = 0; j < posBuf.size(); j+=9) {
            glm::vec4 _v1(posBuf[j], posBuf[j+1], posBuf[j+2], 1.0f);
            glm::vec4 _v2(posBuf[j+3], posBuf[j+4], posBuf[j+5], 1.0f);
            glm::vec4 _v3(posBuf[j+6], posBuf[j+7], posBuf[j+8], 1.0f);

            auto MV = obj->get_transform();

            glm::vec3 v0 = glm::vec3(MV * _v1);
            glm::vec3 v1 = glm::vec3(MV * _v2);
            glm::vec3 v2 = glm::vec3(MV * _v3);

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            float d_A = glm::dot(prev_p - v0, normal);
            float d_B = glm::dot(p - v0, normal);

            float f = glm::abs(d_A) / glm::abs(d_B - d_A);
            glm::vec3 collision_position = prev_p + prev_v * dt * f;

            if ( d_B * d_A < 0.0f && inside(collision_position, v0, v1, v2)){
                S[i] +=  -(1.0f + cr) * d_B * normal;

                glm::vec3 velocity_normal = glm::dot(v , normal) * normal;
                glm::vec3 velocity_tangent = v - velocity_normal;

                S[div+i] = -cr * velocity_normal + (1.0f - cf) * velocity_tangent;
			}
        }
    }

    auto obj_struts = obj->get_struts();
    auto obj_pos = obj->get_posbuf();
    auto MV = obj->get_transform();
    unsigned int p0i, p1i;

    for (int i = 0; i < obj_struts.size(); ++i) {
        p0i = obj_struts[i].v1 * 3;
        p1i = obj_struts[i].v2 * 3;

        glm::vec3 p0 = MV * glm::vec4(obj_pos[p0i], obj_pos[p0i+1], obj_pos[p0i+2], 1.0f);
        glm::vec3 p1 = MV * glm::vec4(obj_pos[p1i], obj_pos[p1i+1], obj_pos[p1i+2], 1.0f);

        for (int j = 0; j < struts.size(); ++j) {
            glm::vec3 q0 = S[struts[j].v1];
            glm::vec3 q1 = S[struts[j].v2];
            glm::vec3 p_q0 = prev_S[struts[j].v1];
            glm::vec3 p_q1 = prev_S[struts[j].v2];

            glm::vec4 _m = closest_point(q0, q1, p0, p1);
            glm::vec4 _p_m = closest_point(p_q0, p_q1, p0, p1);

            float t = _m.w;
            float p_t = _p_m.w;

            if (t < 0.0f && p_t < 0.0f) continue;

            glm::vec3 m = glm::vec3(_m);
            glm::vec3 p_m = glm::vec3(_p_m);

            glm::vec3 a = p1 - p0;
            glm::vec3 b = q1 - q0;

            glm::vec3 n = glm::normalize(glm::cross(a,b));
            
            if (glm::dot(m, p_m) < 0.0f) {
                glm::vec3 v0 = S[S.size()/2 + struts[j].v1];
                glm::vec3 v1 = S[S.size()/2 + struts[j].v2];

                float u = 1.0f - t;
                float v = t;
                float s = glm::dot(m,n) > 0.0f ? -1.0f: 1.0f;

                S[struts[j].v1] += s * (1.0f + cr) * glm::length(m) * n;
                S[struts[j].v2] += s * (1.0f + cr) * glm::length(m) * n;

                glm::vec3 mid_v = v0 * u + v1 * v;

                glm::vec3 velocity_normal = glm::dot(mid_v , n) * n;
                glm::vec3 velocity_tangent = mid_v - velocity_normal;

               
                glm::vec3 dv = (-mid_v -cr * velocity_normal + (1.0f - cf) * velocity_tangent) / (u*u + v*v);

                v0 = u * dv;
                v1 = v * dv;

                S[S.size()/2 + struts[j].v1] += v0;
                S[S.size()/2 + struts[j].v2] += v1;
            }
        }
    }
}

void SoftBody::fitToUnitBox(){
    // Scale the vertex positions so that they fit within [-1, +1] in all three dimensions.
	glm::vec3 vmin(posBuf[0], posBuf[1], posBuf[2]);
	glm::vec3 vmax(posBuf[0], posBuf[1], posBuf[2]);
	for(int i = 0; i < (int)posBuf.size(); i += 3) {
		glm::vec3 v(posBuf[i], posBuf[i+1], posBuf[i+2]);
		vmin.x = std::min(vmin.x, v.x);
		vmin.y = std::min(vmin.y, v.y);
		vmin.z = std::min(vmin.z, v.z);
		vmax.x = std::max(vmax.x, v.x);
		vmax.y = std::max(vmax.y, v.y);
		vmax.z = std::max(vmax.z, v.z);
	}

	glm::vec3 center = 0.5f*(vmin + vmax);
	glm::vec3 diff = vmax - vmin;
	float diffmax = diff.x;
	diffmax = std::max(diffmax, diff.y);
	diffmax = std::max(diffmax, diff.z);
	float scale = 1.0f / diffmax;
	for(int i = 0; i < (int)posBuf.size(); i += 3) {
		posBuf[i  ] = (posBuf[i  ] - center.x) * scale;
		posBuf[i+1] = (posBuf[i+1] - center.y) * scale;
		posBuf[i+2] = (posBuf[i+2] - center.z) * scale;
	}

}

SoftBody::SoftBody(const std::string& meshName){
    loadMesh(meshName);
    fitToUnitBox();
    extract_struts();
    init();
}

std::vector<glm::vec3> SoftBody::integrate(std::vector<glm::vec3> s, float h, std::vector<glm::vec3> ds) {
    std::vector<glm::vec3> res;
    res.resize(s.size());

    for (int i = 0; i < s.size(); ++i) {
        res[i] = s[i] + h * ds[i];
    }
    return res;
}

std::vector<glm::vec3> SoftBody::calculate_forces(std::vector<glm::vec3> state) {
    std::vector<glm::vec3> res;
    res.reserve(state.size());

    int div = state.size() / 2;
    for (int i = 0; i < div; ++i) {
        res.push_back(state[div+i]);
    }

    for (int i = 0; i < div; ++i) {
        res.push_back(gravity * mass); 
    }
    
    
    for (int i = 0; i < faces.size(); ++i) {
        strut s1 = struts[faces[i].s1];
        strut s2 = struts[faces[i].s2];
        strut s3 = struts[faces[i].s3];

        int p1 = s1.v1;
        int p2 = s1.v2;
        int p3 = s2.v1 == p1 || s2.v1 == p2 ? s2.v2 : s2.v1;

        glm::vec3 v1 = state[div + p1];
        glm::vec3 v2 = state[div + p2];
        glm::vec3 v3 = state[div + p3];
        glm::vec3 v = (v1 + v2 + v3) / 3.0f; 

        if (glm::length(wind) > 0.01f && glm::length(v) > 0.01f) {
           
            glm::vec3 x1 = state[p1];
            glm::vec3 x2 = state[p2];
            glm::vec3 x3 = state[p3];

            //calculate force on face            
            glm::vec3 norm = glm::cross(x2 -x1, x3 - x1);

            glm::vec3 n = glm::normalize(norm);
            glm::vec3 vr = v - wind;
            float A = glm::length(norm) / 2.0f;
            glm::vec3 q = glm::cross(n, vr);
            float Ae = A * glm::dot(n,glm::normalize(vr));
            glm::vec3 lift_dir = glm::dot(n,vr) < 0.001f ? glm::vec3(0.0f) : glm::cross(vr, glm::normalize(q));

            glm::vec3 Fda = cd * Ae * vr;
            glm::vec3 Fl = cl * Ae * lift_dir;
            glm::vec3 f = Fda + Fl;

            // distribute force on face to vertices proportional to angle size
            res[div+p1] += (f * (faces[i].a12/3.14159f));
            res[div+p2] += (f * (faces[i].a23/3.14159f));
            res[div+p3] += (f * (faces[i].a31/3.14159f));
        }
    }
    

    for (int i = 0; i < struts.size(); ++i) {
        int p0 = struts[i].v1;
        int p1 = struts[i].v2;
        glm::vec3 x0 = state[p0];
        glm::vec3 x1 = state[p1];

        if (struts[i].f1 != -1 && struts[i].f2 != -1) {
            face face_left = faces[ struts[i].f1 ];
            face face_right = faces[ struts[i].f2 ];

            strut _s = face_left.s1 == i ? struts[face_left.s2] : struts[face_left.s1];
            int p2 = _s.v1 == p0 || _s.v1 == p1 ? _s.v2 : _s.v1;

            _s = face_right.s1 == i ? struts[face_right.s2] : struts[face_right.s1];
            int p3 = _s.v1 == p0 || _s.v1 == p1 ? _s.v2 : _s.v1;

            
            glm::vec3 x2 = state[p2];
            glm::vec3 x3 = state[p3];

            if (struts[i].f1 != -1 && struts[i].f2 != -1) {
                glm::vec3 _nl = glm::cross(x1-x0, x2-x0);
                glm::vec3 _nr = glm::cross(x3-x0, x1- x0);

                glm::vec3 nl = glm::normalize(_nl);
                glm::vec3 nr = glm::normalize(_nr);

                glm::vec3 h = glm::normalize(x1-x0);
                float d02 = glm::dot(x2-x0, h);
                float d03 = glm::dot(x3-x0, h);
                glm::vec3 rl = (x2-x0) - d02*h;
                glm::vec3 rr = (x3-x0) - d03*h;

                float sl = glm::dot(state[div+p2], nl);
                float sr = glm::dot(state[div+p3], nr);

                float theta = glm::atan(glm::dot(glm::cross(nl, nr), h), glm::dot(nl, nr));

                float dtheta_left =  sl/ glm::length(rl);
                float dtheta_right =  sr/ glm::length(rr);

                float torsion = struts[i].tk * (theta - struts[i].to);
                float damping = -struts[i].td * (dtheta_left + dtheta_right);
                glm::vec3 torque = (torsion + damping) * h;

                glm::vec3 f2 = (glm::dot(torque, h) / glm::length(rl)) * nl;
                glm::vec3 f3 = (glm::dot(torque, h) / glm::length(rr)) * nr;
                glm::vec3 f1 = -((d02*f2 + d03*f3) / glm::length(x1-x0));
            
                res[div+p2] += f2;
                res[div+p3] += f3;
                res[div+p1] += f1;
                res[div+p0] -= (f1 + f2 + f3);
                
            }
        }


        glm::vec3 dx = x1-x0;
        glm::vec3 v0 = state[div + p0];
        glm::vec3 v1 = state[div + p1];
        
        glm::vec3 Fs = struts[i].k * (glm::length(dx) - struts[i].lo) * glm::normalize(dx);
        glm::vec3 Fd = struts[i].d * glm::dot(v1 - v0, glm::normalize(dx)) * glm::normalize(dx);

        glm::vec3 f = Fs + Fd;

        res[div+p0] += f;
        res[div+p1] -= f;
        
    }

    for (int i = res.size()/2; i < res.size(); ++i) {
        res[i] /= mass;
    }

    return res;
}

void SoftBody::update(float dt, const glm::vec3& a) {
    prev_S = S;

    auto k1 = calculate_forces(S);

    auto temp = integrate(S, dt*0.5f, k1);
    auto k2 = calculate_forces(temp);

    temp = integrate(S, dt*0.5f, k2);
    auto k3 = calculate_forces(temp);

    temp = integrate(S, dt, k3);
    auto k4 = calculate_forces(temp);

    S = runge_kutta(S, dt, k1, k2, k3, k4);


    //normBuf = recalculate_normals(S);
}

void SoftBody::loadMesh(const std::string &meshName) {
    // Load geometry
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		std::cerr << errStr << std::endl;
	} else if (shapes.size() == 0) {
		std::cout<< "no shapes to extract \n";
		return;
	} else {
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
					if(!attrib.normals.empty()) {
						norBuf.push_back(attrib.normals[3*idx.normal_index+0]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+1]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+2]);
					}
					if(!attrib.texcoords.empty()) {
						texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
						texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
					}
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void SoftBody::init(){
    int particle_count = S.size() / 2;
    // Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, particle_count*3*sizeof(float), &S[0], GL_DYNAMIC_DRAW);
	
	
	// Send the index array to gpu
    glGenBuffers(1, &indBufID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SoftBody::draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P) {
	
	glm::mat4 iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));

	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
	glUniform3f(prog->getUniform("ka"), material->ka.x, material->ka.y, material->ka.z);
	glUniform3f(prog->getUniform("kd"), material->kd.x, material->kd.y, material->kd.z);
	glUniform3f(prog->getUniform("ks"), material->ks.x, material->ks.y, material->ks.z);
	glUniform1f(prog->getUniform("s"), material->s );
    glUniform1f(prog->getUniform("a"), 0.9f );

    int div = S.size()/2;

    int pos_id = prog->getAttribute("aPos");
    glEnableVertexAttribArray(pos_id);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, div*3*sizeof(float), &S[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(pos_id, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(prog->getAttribute("aPos"));
}

void SoftBody::extract_struts(){
    // make member func

    for (int i = 0; i < posBuf.size(); i+=9) {
        face f;
        strut s12,s23,s31;
        glm::vec3 v1,v2,v3, n1,n2,n3;

        v1 = glm::vec3(posBuf[i+0], posBuf[i+1], posBuf[i+2]);
        v2 = glm::vec3(posBuf[i+3], posBuf[i+4], posBuf[i+5]);
        v3 = glm::vec3(posBuf[i+6], posBuf[i+7], posBuf[i+8]);

        if (norBuf.size() >= i+8) {
            n1 = glm::vec3(norBuf[i+0], norBuf[i+1], norBuf[i+2]);
            n2 = glm::vec3(norBuf[i+3], norBuf[i+4], norBuf[i+5]);
            n3 = glm::vec3(norBuf[i+6], norBuf[i+7], norBuf[i+8]);
        }

        int r1,r2,r3;

        bool v1_unique = unique_particle(v1, S, r1);
        indices.push_back(r1);

        bool v2_unique = unique_particle(v2, S, r2);
        indices.push_back(r2);

        bool v3_unique = unique_particle(v3, S, r3);
        indices.push_back(r3);

        int face_idx = faces.size();

        int s12_r;
        bool strut12_unique = unique_strut(r1,r2, struts, s12_r);
        if (strut12_unique) {
            s12.lo = glm::length(v1-v2);
            s12.k = k * (L / s12.lo);
            s12.d = d * (L / s12.lo);
            s12.tk = tk;
            s12.td = td;
            s12.v1 = r1;
            s12.v2 = r2;
            s12.f2 = -1;
            struts.push_back(s12);
            struts[s12_r].f1 = face_idx;
        } else {
            struts[s12_r].f2 = face_idx;
        }

        int s23_r;
        bool strut23_unique = unique_strut(r2, r3, struts, s23_r);
        if (strut23_unique){
            s23.lo = glm::length(v2-v3);
            s23.k = k * (L / s23.lo);
            s23.d = d * (L / s23.lo);
            s23.tk = tk;
            s23.td = td;
            s23.v1 = r2;
            s23.v2 = r3;
            s23.f2 = -1;
            struts.push_back(s23);
            struts[s23_r].f1 = face_idx;
        } else {
            struts[s23_r].f2 = face_idx;
        }

        int s31_r;
        bool strut31_unique = unique_strut(r3, r1, struts, s31_r);
        if (strut31_unique) {
            s31.lo = glm::length(v3-v1);
            s31.k = k * (L / s31.lo);
            s31.d = d * (L / s31.lo);
            s31.tk = tk;
            s31.td = td;
            s31.v1 = r3;
            s31.v2 = r1;
            s31.f2 = -1;
            struts.push_back(s31);
            struts[s31_r].f1 = face_idx;
        } else {
            struts[s31_r].f2 = face_idx;
        }

        f.s1 = s12_r;
        f.s2 = s23_r;
        f.s3 = s31_r;

        float a12 = angle_v(v1 - v2, v3 - v2);
        float a23 = angle_v(v1 - v3, v2 - v3);
        float a31 = angle_v(v2 - v1, v3 - v1);

        f.a12 = a12;
        f.a23 = a23;
        f.a31 = a31;

        faces.push_back(f);
    }

    int div = S.size();
    for (int i = 0; i < div; ++i) {
        S.push_back(velocity);
    }

    prev_S = S;

    for (int gid = 0; gid < struts.size(); ++gid) {
        strut s = struts[gid];
        if (s.f1 == -1 || s.f2 == -1) continue;

        face face_left = faces[s.f1]; 
        face face_right = faces[s.f2]; 

        int p0 = s.v1;
        int p1 = s.v2;

        strut _s = face_left.s1 == gid ? struts[face_left.s2] : struts[face_left.s1];
        int p2 = _s.v1 == p0 || _s.v1 == p1 ? _s.v2 : _s.v1;
        _s = face_right.s1 == gid ? struts[face_right.s2] : struts[face_right.s1];
        int p3 = _s.v1 == p0 || _s.v1 == p1 ? _s.v2 : _s.v1;

        glm::vec3 x0 = S[p0];
        glm::vec3 x1 = S[p1];
        glm::vec3 x2 = S[p2];
        glm::vec3 x3 = S[p3];

        glm::vec3 nl = glm::normalize(glm::cross(x1-x0, x2-x0));
        glm::vec3 nr = glm::normalize(glm::cross(x3-x0, x1- x0));

        glm::vec3 h = glm::normalize(x1-x0);

        float theta = std::atan2((glm::dot( glm::cross(nl, nr), h) ), (glm::dot(nl, nr)));
        struts[gid].to = theta;
    }

}

void SoftBody::initial_conditions(InitialConditions& start, std::shared_ptr<Material> _material) {
    material = _material;
}