#pragma  once
#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class MatrixStack;

class Camera
{
public:
	enum {
		ROTATE = 0,
		TRANSLATE,
		SCALE
	};
	
	Camera();
	virtual ~Camera();
	void setAspect(float a) { aspect = a; };
	void applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const;
	void applyOrthoMatrix(std::shared_ptr<MatrixStack> P,float w, float h) const;
	void applyOrthoTopMatrix(std::shared_ptr<MatrixStack> P,float w, float h) const;
	void applyViewMatrix(std::shared_ptr<MatrixStack> MV) const;
	void applyTopViewMatrix(std::shared_ptr<MatrixStack> MV) const;

	void increment_fovy();
	void decrement_fovy();

	glm::vec3 pos;
	float yaw;
	float pitch;
	
private:
	float aspect;
	float fovy;
	float znear;
	float zfar;

	glm::vec2 rotations;
	glm::vec3 translations;


	
};

#endif
