#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <vector>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

#include "MatrixStack.h"
#include "MeshUtil.h"

struct box {
	glm::vec3 vmin, vmax;
};

class SoftBody;
class StaticBody;
class Program;

class Entity {
	public:
		Entity();
		Entity(const std::string& meshName);
		Entity(std::vector<float>&, std::vector<float>&, std::vector<float>&, std::vector<unsigned int>&);
		~Entity();

		virtual void initial_conditions(const InitialConditions& start, std::shared_ptr<Material> material);
		virtual void update(float dt, const glm::vec3& a);
		virtual void draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV) const;		

		std::vector<float>& get_posbuf() { return posBuf; }
		
		glm::mat4 get_transform() {
			glm::mat4 MV(1.0f);
			MV = glm::translate(MV, position);
			MV = glm::scale(MV, scale);
			if (glm::length(rotation) > 0.001f) MV = rotate(MV, glm::length(rotation), rotation);
			return MV;
		}

		std::vector<strut> get_struts() { return struts; }
		
	protected:
		virtual void loadMesh(const std::string &meshName);
		void setShape(std::vector<float> pos, std::vector<float> nor, std::vector<float> tex, std::vector<unsigned int> ind);
		void fitToUnitBox();
		virtual void init();
		void extract_struts();

		std::vector<float> posBuf;
		std::vector<float> norBuf;
		std::vector<float> texBuf;
		std::vector<unsigned int> indBuf;

		std::vector<strut> struts;

		unsigned posBufID;
		unsigned norBufID;
		unsigned texBufID;
		unsigned indBufID;

		std::shared_ptr<Material> material;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		box aabb;
};

#endif
