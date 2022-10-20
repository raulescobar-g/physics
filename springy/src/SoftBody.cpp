#include "Softbody.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "tiny_obj_loader.h"
#include <iostream>
#include <unordered_map>

SoftBody::SoftBody(const std::string& meshName){
    loadMesh(meshName);
    fitToUnitBox();
    extract_struts();
    init();
}

SoftBody::SoftBody(std::vector<float>& p, std::vector<float>& n, std::vector<float>& t, std::vector<unsigned int>& i){
    setShape(p, n, t, i);
    fitToUnitBox();
    extract_struts();
    init();
}

void SoftBody::set_programs(std::shared_ptr<Program> f, std::shared_ptr<Program>  s, std::shared_ptr<Program> i){
    face_compute = f;
    integration_compute = i;
    strut_compute = s;
}

void SoftBody::update(float dt, const glm::vec3& a) {
    
    face_compute->bind();
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, posBufID );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, partSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, faceSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, strutSSbo );
	glUniform3f(face_compute->getUniform("wind"), 0.1f, 0.0f, 0.0f);
    //glDispatchCompute( face_count, 1, 1 );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);   
    face_compute->unbind();

    strut_compute->bind();
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, posBufID );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, partSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, faceSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, strutSSbo );
    glDispatchCompute( strut_count, 1, 1 );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);   
    strut_compute->unbind();

    // for ( int i = 0;  i < 9; ++i){
    //      std::cout<<particles[i].force.x<<", "<<particles[i].force.y<<", "<<particles[i].force.z<<", "<<particles[i].force.w<<std::endl;
    // }
    // std::cout<<std::endl;


    integration_compute->bind();
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, posBufID );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, partSSbo );
    glUniform1f(integration_compute->getUniform("dt"), dt);
    glUniform3f(integration_compute->getUniform("gravity"), 0.0f, -0.0f, 0.0f);
    glDispatchCompute( particle_count, 1, 1 );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);  
    integration_compute->unbind();

    // for (int i = 0; i < 9*3; i+=3) {
    //     std::cout<<positions[i]<<", "<<positions[i+1]<<", "<<positions[i+2]<<std::endl;
    // }
    // //std::cout<<std::endl;
    for (int i = 0; i < 9; ++i) {
        std::cout<<particles[i].velocity.x<<", "<<particles[i].velocity.y<<", "<<particles[i].velocity.z<<std::endl;
    }
    std::cout<<std::endl;
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
    GLint bufMask = GL_MAP_WRITE_BIT |  GL_MAP_READ_BIT |  GL_MAP_PERSISTENT_BIT |  GL_MAP_COHERENT_BIT;

    glGenBuffers( 1, &posBufID);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, posBufID );
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, temp_positions.size()*sizeof(float), &temp_positions[0], bufMask | GL_DYNAMIC_STORAGE_BIT);
    positions = (float *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, temp_positions.size()*sizeof(float), bufMask );

    glGenBuffers( 1, &norBufID);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, norBufID );
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, temp_normals.size()*sizeof(float), &temp_normals[0], bufMask | GL_DYNAMIC_STORAGE_BIT);

    glGenBuffers( 1, &partSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, partSSbo );
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, temp_particles.size()*sizeof(particle), &temp_particles[0], bufMask | GL_DYNAMIC_STORAGE_BIT);
    particles = (particle *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, temp_particles.size()*sizeof(particle), bufMask );

    glGenBuffers( 1, &faceSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, faceSSbo);
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, temp_faces.size()*sizeof(face), &temp_faces[0], bufMask | GL_DYNAMIC_STORAGE_BIT);

    glGenBuffers( 1, &strutSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, strutSSbo);
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, temp_struts.size()*sizeof(strut), &temp_struts[0], bufMask | GL_DYNAMIC_STORAGE_BIT);

    if(!texBuf.empty()) {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, texBufID);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], bufMask | GL_DYNAMIC_STORAGE_BIT);
	}
    if (!indBuf.empty()) {
		glGenBuffers(1, &indBufID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, indBufID);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], bufMask | GL_DYNAMIC_STORAGE_BIT);
	}

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    temp_positions.clear();
    temp_normals.clear();
    temp_particles.clear();
    temp_faces.clear();
    temp_struts.clear();
    GLSL::checkError(GET_FILE_LINE); 
}

bool near(glm::vec3 v1, glm::vec3 v2){
    return glm::abs(v1.x - v2.x) < 0.001 && glm::abs(v1.y - v2.y) < 0.001 && glm::abs(v1.z - v2.z) < 0.001;
}

bool unique_particle(glm::vec3 vertex, std::vector<glm::vec3>& vec, int& result) {
    result = vec.size();
    for (int i = 0; i < vec.size(); ++i) {
        if (near(vertex, vec[i])){
            result = i;
            return false;
        }
    }
    vec.push_back(vertex);
    return true;
}

bool unique_strut(int r1, int r2, std::vector<strut>& strut_table, int& result){
    result = strut_table.size();
    for (int i = 0; i < strut_table.size(); ++i) {
        int _r1 = strut_table[i].v1;
        int _r2 = strut_table[i].v2;
        if ( (r1 == _r1 && r2 == _r2) || (r1 == _r2 && r2 == _r1)){
            result = i;
            return false;
        }
    }
    strut t;
    t.v1 = r1;
    t.v2 = r2;
    strut_table.push_back(t);
    return true;
}

float angle_v(glm::vec3 a, glm::vec3 b){
    return glm::acos( glm::clamp(glm::dot( glm::normalize(a), glm::normalize(b)),-0.99999f, 0.99999f) );
}

void SoftBody::extract_struts(){
    // make member func
    std::vector<glm::vec3> vertex_map; 
    std::vector<strut> strut_map;

    indBuf.clear();
    for (int i = 0; i < posBuf.size(); i+=9) {
        particle p1,p2,p3;
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
        bool v1_unique = unique_particle(v1, vertex_map, r1);
        if (v1_unique) {
            temp_positions.push_back(v1.x); 
            temp_positions.push_back(v1.y);
            temp_positions.push_back(v1.z);

            temp_normals.push_back(n1.x);
            temp_normals.push_back(n1.y);
            temp_normals.push_back(n1.z);

            p1.velocity = glm::vec4(velocity, 0.0f);
            p1.force = glm::vec4(0.0f);
            temp_particles.push_back(p1);
        } 
        indBuf.push_back(r1);

        bool v2_unique = unique_particle(v2, vertex_map, r2);
        if (v2_unique) {
            temp_positions.push_back(v2.x);
            temp_positions.push_back(v2.y);
            temp_positions.push_back(v2.z);

            temp_normals.push_back(n2.x);
            temp_normals.push_back(n2.y);
            temp_normals.push_back(n2.z);

            p2.velocity = glm::vec4(velocity, 0.0f);
            p2.force = glm::vec4(0.0f);
            temp_particles.push_back(p2);
        } 
        indBuf.push_back(r2);

        bool v3_unique = unique_particle(v3, vertex_map, r3);
        if (v3_unique) {
            temp_positions.push_back(v3.x);
            temp_positions.push_back(v3.y);
            temp_positions.push_back(v3.z);

            temp_normals.push_back(n3.x);
            temp_normals.push_back(n3.y);
            temp_normals.push_back(n3.z);

            p3.velocity = glm::vec4(velocity, 0.0f);
            p3.force = glm::vec4(0.0f);
            temp_particles.push_back(p3);
        } 
        indBuf.push_back(r3);

        int face_idx = temp_faces.size();

        int s12_r;
        bool strut12_unique = unique_strut(r1,r2,strut_map, s12_r);
        if (strut12_unique) {
            s12.lo = glm::length(v1-v2);
            s12.k = k;
            s12.d = d;
            s12.tk = tk;
            s12.td = td;
            s12.v1 = r1;
            s12.v2 = r2;
            s12.f2 = -1;
            temp_struts.push_back(s12);
            temp_struts[s12_r].f1 = face_idx;
        } else {
            temp_struts[s12_r].f2 = face_idx;
        }

        int s23_r;
        bool strut23_unique = unique_strut(r2, r3, strut_map, s23_r);
        if (strut23_unique){
            s23.lo = glm::length(v2-v3);
            s23.k = k;
            s23.d = d;
            s23.tk = tk;
            s23.td = td;
            s23.v1 = r2;
            s23.v2 = r3;
            s23.f2 = -1;
            temp_struts.push_back(s23);
            temp_struts[s23_r].f1 = face_idx;
        } else {
            temp_struts[s23_r].f2 = face_idx;
        }

        int s31_r;
        bool strut31_unique = unique_strut(r3, r1, strut_map, s31_r);
        if (strut31_unique) {
            s31.lo = glm::length(v3-v1);
            s31.k = k;
            s31.d = d;
            s31.tk = tk;
            s31.td = td;
            s31.v1 = r3;
            s31.v2 = r1;
            s31.f2 = -1;
            temp_struts.push_back(s31);
            temp_struts[s31_r].f1 = face_idx;
        } else {
            temp_struts[s31_r].f2 = face_idx;
        }

        f.s1 = s12_r;
        f.s2 = s23_r;
        f.s3 = s31_r;

        glm::vec3 _v1(temp_positions[r1 + 0], temp_positions[r1 + 1], temp_positions[r1 + 2]);
        glm::vec3 _v2(temp_positions[r2 + 0], temp_positions[r2 + 1], temp_positions[r2 + 2]);
        glm::vec3 _v3(temp_positions[r3 + 0], temp_positions[r3 + 1], temp_positions[r3 + 2]);

        float a12 = angle_v(_v1 - _v2, _v3 - _v2);
        float a23 = angle_v(_v1 - _v3, _v2 - _v3);
        float a31 = angle_v(_v2 - _v1, _v3 - _v1);

        f.a12 = a12;
        f.a23 = a23;
        f.a31 = a31;

        temp_faces.push_back(f);
    }

    for (int gid = 0; gid < temp_struts.size(); ++gid) {
        strut s = temp_struts[gid];
        if (s.f1 == -1 || s.f2 == -1) continue;

        face face_left = temp_faces[s.f1]; 
        face face_right = temp_faces[s.f2]; 

        int p0 = s.v1;
        int p1 = s.v2;

        strut _s = face_left.s1 == gid ? temp_struts[face_left.s2] : temp_struts[face_left.s1];
        int p2 = _s.v1 == p0 || _s.v1 == p1 ? _s.v2 : _s.v1;
        _s = face_right.s1 == gid ? temp_struts[face_right.s2] : temp_struts[face_right.s1];
        int p3 = _s.v1 == p0 || _s.v1 == p1 ? _s.v2 : _s.v1;

        glm::vec3 x0 = glm::vec3(temp_positions[ p0*3 ], temp_positions[ p0*3 + 1 ], temp_positions[ p0*3 + 2 ]);
        glm::vec3 x1 = glm::vec3(temp_positions[ p1*3 ], temp_positions[ p1*3 + 1 ], temp_positions[ p1*3 + 2 ]);
        glm::vec3 x2 = glm::vec3(temp_positions[ p2*3 ], temp_positions[ p2*3 + 1 ], temp_positions[ p2*3 + 2 ]);
        glm::vec3 x3 = glm::vec3(temp_positions[ p3*3 ], temp_positions[ p3*3 + 1 ], temp_positions[ p3*3 + 2 ]);

        glm::vec3 nl = glm::normalize(glm::cross(x1-x0, x2-x0));
        glm::vec3 nr = glm::normalize(glm::cross(x3-x0, x1- x0));

        glm::vec3 h = glm::normalize(x1-x0);
        float d02 = glm::dot(x2-x0, h);
        float d03 = glm::dot(x3-x0, h);
        glm::vec3 rl = (x2-x0) - d02*h;
        glm::vec3 rr = (x3-x0) - d03*h;

        float sl = glm::dot(glm::vec3(temp_particles[p2].velocity), nl);
        float sr = glm::dot(glm::vec3(temp_particles[p3].velocity), nr);

        float theta = std::atan2((glm::dot( glm::cross(nl, nr), h) ), (glm::dot(nl, nr)));
        temp_struts[gid].to = theta;
    }

    for (auto&p : temp_particles){
        p.force.w = mass / (float) temp_particles.size();
    }

    vertex_map.clear();
    strut_map.clear();
    face_count = temp_faces.size();
    particle_count = temp_particles.size();
    strut_count = temp_struts.size();
}