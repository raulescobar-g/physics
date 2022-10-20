#include "Entity.h"
#include <algorithm>
#include <iostream>

#include "GLSL.h"
#include "Program.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Entity::Entity() {}

Entity::Entity(const std::string &meshName) : posBufID(0), norBufID(0), texBufID(0), indBufID(0){
	loadMesh(meshName);
	fitToUnitBox();
	init();
}

Entity::Entity(std::vector<float>& pos, std::vector<float>& norm, std::vector<float>& tex, std::vector<unsigned int>& ind) : posBufID(0), norBufID(0), texBufID(0), indBufID(0){
	setShape(pos, norm, tex, ind);
	fitToUnitBox();
	init();
}

Entity::~Entity(){}

void Entity::loadMesh(const std::string &meshName){
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

void Entity::setShape(std::vector<float> pos, 
					std::vector<float> nor, 
					std::vector<float> tex, 
					std::vector<unsigned int> ind
					){
	posBuf = pos;
	norBuf = nor;
	texBuf = tex;
	indBuf = ind;
}

void Entity::fitToUnitBox(){
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

	vmin = glm::vec3(posBuf[0], posBuf[1], posBuf[2]);
	vmax = glm::vec3(posBuf[0], posBuf[1], posBuf[2]);
	for(int i = 0; i < (int)posBuf.size(); i += 3) {
		glm::vec3 v(posBuf[i], posBuf[i+1], posBuf[i+2]);
		vmin.x = std::min(vmin.x, v.x);
		vmin.y = std::min(vmin.y, v.y);
		vmin.z = std::min(vmin.z, v.z);
		vmax.x = std::max(vmax.x, v.x);
		vmax.y = std::max(vmax.y, v.y);
		vmax.z = std::max(vmax.z, v.z);
	}

	aabb.vmin = vmin;
	aabb.vmax = vmax;
}

void Entity::init(){
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	
	// Send the texture array to the GPU
	if(!texBuf.empty()) {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the index array to gpu
	if (!indBuf.empty()) {
		glGenBuffers(1, &indBufID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
		
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Entity::draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P) const {

	MV->translate(position);
	MV->scale(scale);
	if (glm::length(rotation) > 0.001f) MV->rotate(glm::length(rotation), rotation);
	
	glm::mat4 iMV = glm::transpose(glm::inverse(glm::mat4(MV->topMatrix())));

	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
	glUniform3f(prog->getUniform("ka"), material->ka.x, material->ka.y, material->ka.z);
	glUniform3f(prog->getUniform("kd"), material->kd.x, material->kd.y, material->kd.z);
	glUniform3f(prog->getUniform("ks"), material->ks.x, material->ks.y, material->ks.z);
	glUniform1f(prog->getUniform("s"), material->s );


	// Bind position buffer
	GLint h_pos = prog->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	// Bind normal buffer
	int h_nor = prog->getAttribute("aNor");
	glEnableVertexAttribArray(h_nor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	

	// Bind texcoords buffer
	int h_tex = prog->getAttribute("aTex");
	if(h_tex != -1 && texBufID != 0) {
		glEnableVertexAttribArray(h_tex);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (!indBuf.empty()) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID); 
		glDrawElements(GL_TRIANGLES, (int)indBuf.size(), GL_UNSIGNED_INT, (void *)0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
	} else {
		int count = posBuf.size()/3;
		glDrawArrays(GL_TRIANGLES, 0, count);
	}

	// Disable and unbind
	if(h_tex != -1) {
		glDisableVertexAttribArray(h_tex);
	}
	glDisableVertexAttribArray(h_nor);
	glDisableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Entity::update(float dt, const glm::vec3& a) {
	std::cout<<"using base class"<<std::endl;
	exit(-1);
}

void Entity::initial_conditions(const InitialConditions& start, std::shared_ptr<Material> obj_material) {
	position = start.position;
    rotation = start.rotation;
    scale = start.scale;
    material = obj_material;
}