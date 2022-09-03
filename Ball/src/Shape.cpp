#include "Shape.h"
#include <algorithm>
#include <iostream>

#include "GLSL.h"
#include "Program.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;

Shape::Shape() :
	posBufID(0),
	norBufID(0),
	texBufID(0),
	indBufID(0)
{
}

Shape::~Shape()
{
}

void Shape::loadMesh(const string &meshName)
{
	// Load geometry
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
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

void Shape::createMesh(std::string type, int parameter) {
	
	int res = parameter;
	float rad = (2.0f * glm::pi<float>()) / (float) (res-2);
	float len = 10.0f;

	if (type == "gen_sphere"){

		for (int i = 0; i < res/2 ; ++i) {
			for (int j = 0; j < res ; ++j) {

				float theta = i * rad;
				float phi = j * rad;

				posBuf.push_back(sin(theta) * sin(phi));
				posBuf.push_back(cos(theta));
				posBuf.push_back(sin(theta) * cos(phi) );
				

				norBuf.push_back(sin(theta) * sin(phi));
				norBuf.push_back(cos(theta));
				norBuf.push_back(sin(theta) * cos(phi));

				// texBuf.push_back(2.0f*j / (res *0.05f));
				// texBuf.push_back(1.0f - (float) i /  (res * 0.05f));
			}
		}

		for (int x = 0; x < res-1; ++x){
			for (int y = 0; y < (res/2)-1; ++y) {

				int idx = y * res + x;
				indBuf.push_back(idx + 1 + res);
				indBuf.push_back(idx + 1);
				indBuf.push_back(idx);
				
				indBuf.push_back(idx + res);
				indBuf.push_back(idx + 1 + res);
				indBuf.push_back(idx);
			}
		}
	} else if (type == "vase") {

		for (int i = 0; i < res ; ++i) {
			for (int j = 0; j < res ; ++j) {

				float theta = i * rad;
				float x = (j / (float) res) * len;

				posBuf.push_back(x);
				posBuf.push_back(theta);
				posBuf.push_back(0.0f);

				norBuf.push_back(0.0f);
				norBuf.push_back(0.0f);
				norBuf.push_back(0.0f);

				// texBuf.push_back(2.0f*j / (res *0.5f));
				// texBuf.push_back(1.0f - (float) i /  (res * 0.5f));
			}
		}

		for (int x = 0; x < res-1; ++x){
			for (int y = 0; y < (res)-1; ++y) {

				int idx = y * res + x;
				indBuf.push_back(idx);
				indBuf.push_back(idx + 1);
				indBuf.push_back(idx + 1 + res);
				
				
				indBuf.push_back(idx);
				indBuf.push_back(idx + 1 + res);
				indBuf.push_back(idx + res);
			}
		}
	} else {
		assert(false);
	}
	
}

void Shape::fitToUnitBox()
{
	// Scale the vertex positions so that they fit within [-1, +1] in all three dimensions.
	glm::vec3 vmin(posBuf[0], posBuf[1], posBuf[2]);
	glm::vec3 vmax(posBuf[0], posBuf[1], posBuf[2]);
	for(int i = 0; i < (int)posBuf.size(); i += 3) {
		glm::vec3 v(posBuf[i], posBuf[i+1], posBuf[i+2]);
		vmin.x = min(vmin.x, v.x);
		vmin.y = min(vmin.y, v.y);
		vmin.z = min(vmin.z, v.z);
		vmax.x = max(vmax.x, v.x);
		vmax.y = max(vmax.y, v.y);
		vmax.z = max(vmax.z, v.z);
	}

	glm::vec3 center = 0.5f*(vmin + vmax);
	glm::vec3 diff = vmax - vmin;
	float diffmax = diff.x;
	diffmax = max(diffmax, diff.y);
	diffmax = max(diffmax, diff.z);
	float scale = 1.0f / diffmax;
	for(int i = 0; i < (int)posBuf.size(); i += 3) {
		posBuf[i  ] = (posBuf[i  ] - center.x) * scale;
		posBuf[i+1] = (posBuf[i+1] - center.y) * scale;
		posBuf[i+2] = (posBuf[i+2] - center.z) * scale;
	}
}


void Shape::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(!norBuf.empty()) {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(!texBuf.empty()) {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	if (!indBuf.empty()) {
		glGenBuffers(1, &indBufID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	GLSL::checkError(GET_FILE_LINE);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	// Bind position buffer
	int h_pos = prog->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	int h_nor = prog->getAttribute("aNor");
	if(h_nor != -1 && norBufID != 0) {
		glEnableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}
	
	// Bind texcoords buffer
	int h_tex = prog->getAttribute("aTex");
	if(h_tex != -1 && texBufID != 0) {
		glEnableVertexAttribArray(h_tex);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (!indBuf.empty()) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID); }
	
	// Draw
	int count = posBuf.size()/3; // number of indices to be rendered

	indBuf.empty() ? glDrawArrays(GL_TRIANGLES, 0, count) : glDrawElements(GL_TRIANGLES, (int)indBuf.size(), GL_UNSIGNED_INT, (void *)0);
	
	if (!indBuf.empty()) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

	// Disable and unbind
	if(h_tex != -1) {
		glDisableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		glDisableVertexAttribArray(h_nor);
	}
	glDisableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLSL::checkError(GET_FILE_LINE);
}