#include "Cloth.h"
#include <iostream>

Cloth::Cloth(int n, const char* file){
    int vertCount = (n+1) * (n+1);
    float step = 2.0f / (float) n;
    float uvStep = 1.0f / (float) n;

    glm::vec3 offset = glm::vec3(-1.0f, 0.0f, -1.0f);

    anchor1 = vertCount-1;
    anchor2 = n;

    for (int x = 0; x <= n; ++x) {
        for (int z = 0; z <= n; ++z) {
            glm::vec3 pos = glm::vec3(x * step, z * step, 0.0f) + offset;
            glm::vec3 vel = glm::vec3(0.0f, 0.0f, 0.0f);
            S.push_back(pos * scale + anchor_translation);
            uvs.push_back(glm::vec2(x*uvStep, (n-z) * uvStep));
        }
    }

    for (int x = 0; x <= n; ++x) {
        for (int z = 0; z <= n; ++z) {
            glm::vec3 vel(0.0f);
            S.push_back(vel);
        }
    }


    for (unsigned int v = 0; v < vertCount - n - 2; v++) {
        if ((v + 1) % (n + 1) == 0) {
			v++;
		}

		indices.push_back(v);
		indices.push_back(v + 1);
		indices.push_back(v + n + 1);
 
		indices.push_back(v + 1);
		indices.push_back(v + n + 2);
		indices.push_back(v + n + 1);
    }

    for (int i = 0; i < indices.size(); i+=3) {
        face f;
        int face_idx = faces.size();

        strut s12, s23, s31;
        int s12_i, s23_i, s31_i;

        int p1 = indices[i];
        int p2 = indices[i+1];
        int p3 = indices[i+2];

        glm::vec3 v1 = S[p1];
        glm::vec3 v2 = S[p2];
        glm::vec3 v3 = S[p3];

        bool s12_unique = unique_strut(p1,p2,struts, s12_i); // import
        if (s12_unique) {
            s12.lo = glm::length(v1-v2);
            s12.k = k * (L / s12.lo);
            s12.d = d * (L / s12.lo);

            s12.v1 = p1;
            s12.v2 = p2;
            s12.f2 = -1;
            struts.push_back(s12);
            struts[s12_i].f1 = face_idx;
        } else {
            struts[s12_i].f2 = face_idx;
        }

        bool s23_unique = unique_strut(p2,p3,struts, s23_i);
        if (s23_unique) {
            s23.lo = glm::length(v2-v3);
            s23.k = k * (L / s23.lo);
            s23.d = d * (L / s23.lo);

            s23.v1 = p2;
            s23.v2 = p3;
            s23.f2 = -1;
            struts.push_back(s23);
            struts[s23_i].f1 = face_idx;
        } else {
            struts[s23_i].f2 = face_idx;
        }

        bool s31_unique = unique_strut(p3,p1,struts, s31_i);
        if (s31_unique) {
            s31.lo = glm::length(v3-v1);
            s31.k = k * (L / s31.lo);
            s31.d = d * (L / s31.lo);

            s31.v1 = p3;
            s31.v2 = p1;
            s31.f2 = -1;
            struts.push_back(s31);
            struts[s31_i].f1 = face_idx;
        } else {
            struts[s31_i].f2 = face_idx;
        }

        f.s1 = s12_i;
        f.s2 = s23_i;
        f.s3 = s31_i; 

        float a12 = angle_v(v1 - v2, v3 - v2);
        float a23 = angle_v(v1 - v3, v2 - v3);
        float a31 = angle_v(v2 - v1, v3 - v1);

        f.a12 = a12;
        f.a23 = a23;
        f.a31 = a31;
        faces.push_back(f);
    }

    texture = std::make_shared<Texture>();
	texture->setFilename(file);
	texture->init();
	texture->setUnit(0);
	texture->setWrapModes(GL_REPEAT, GL_REPEAT);

    prev_S = S;
    init();
}

void Cloth::init() {
    int particle_count = S.size() / 2;
    // Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, particle_count*3*sizeof(float), &S[0], GL_DYNAMIC_DRAW);
	
	// Send the texture array to the GPU
    glGenBuffers(1, &texBufID);
    glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    glBufferData(GL_ARRAY_BUFFER, uvs.size()*2*sizeof(unsigned int), &uvs[0], GL_STATIC_DRAW);
	
	// Send the index array to gpu
    glGenBuffers(1, &indBufID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Cloth::draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P) {

    texture->bind(prog->getUniform("texture"));
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

    int pos_id = prog->getAttribute("aPos");
    glEnableVertexAttribArray(pos_id);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, S.size()*3*sizeof(float), &S[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(pos_id, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    int tex_id = prog->getAttribute("aTex");
    glEnableVertexAttribArray(tex_id);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glVertexAttribPointer(tex_id, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(prog->getAttribute("aTex"));
	glDisableVertexAttribArray(prog->getAttribute("aPos"));
    texture->unbind();
}

std::vector<glm::vec3> Cloth::calculate_forces(std::vector<glm::vec3> state) {
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
        glm::vec3 x0 = state[struts[i].v1];
        glm::vec3 x1 = state[struts[i].v2];

        glm::vec3 dx = x1-x0;
        glm::vec3 v0 = state[div + struts[i].v1];
        glm::vec3 v1 = state[div + struts[i].v2];
        
        glm::vec3 Fs = struts[i].k * (glm::length(dx) - struts[i].lo) * glm::normalize(dx);
        glm::vec3 Fd = struts[i].d * glm::dot(v1 - v0, glm::normalize(dx)) * glm::normalize(dx);

        glm::vec3 f = Fs + Fd;

        res[div+struts[i].v1] += f;
        res[div+struts[i].v2] -= f;
    }

    if (anchor1 > -1) res[anchor1] = glm::vec3(0.0f, 0.0f, 0.0f);
    if (anchor2 > -1) res[anchor2] = glm::vec3(0.0f, 0.0f, 0.0f);


    return res;
}

std::vector<glm::vec3> Cloth::integrate(std::vector<glm::vec3> s, float h, std::vector<glm::vec3> ds) {
    std::vector<glm::vec3> res;
    res.resize(s.size());

    for (int i = 0; i < s.size(); ++i) {
        res[i] = s[i] + h * ds[i];
    }
    return res;
}

void Cloth::update(float dt) {
    prev_S = S;

    auto k1 = calculate_forces(S);

    auto temp = integrate(S, dt*0.5f, k1);
    auto k2 = calculate_forces(temp);

    temp = integrate(S, dt*0.5f, k2);
    auto k3 = calculate_forces(temp);

    temp = integrate(S, dt, k3);
    auto k4 = calculate_forces(temp);

    S = runge_kutta(S, dt, k1, k2, k3, k4);
}

void Cloth::initial_conditions(glm::vec3 pos, float s) {
    scale = s;
    anchor_translation = pos;
}


void Cloth::calculate_collision_response(std::shared_ptr<StaticBody> obj, float dt) {
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
