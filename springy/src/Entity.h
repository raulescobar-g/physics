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

struct Material {
	glm::vec3 ka = glm::vec3(0.5f);
	glm::vec3 kd = glm::vec3(0.5f);
	glm::vec3 ks = glm::vec3(0.5f);
	float s = 10.0f;
};
struct InitialConditions {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);
};

struct box {
	glm::vec3 vmin, vmax;
};

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
		
		std::shared_ptr<MatrixStack> get_transform() {
			auto MV = std::shared_ptr<MatrixStack>();
			MV->translate(position);
			MV->scale(scale);
			if (glm::length(rotation) > 0.001f) MV->rotate(glm::length(rotation), rotation);
			return MV;
		}
		
	protected:
		virtual void loadMesh(const std::string &meshName);
		void setShape(std::vector<float> pos, std::vector<float> nor, std::vector<float> tex, std::vector<unsigned int> ind);
		void fitToUnitBox();
		virtual void init();

		std::vector<float> posBuf;
		std::vector<float> norBuf;
		std::vector<float> texBuf;
		std::vector<unsigned int> indBuf;

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
