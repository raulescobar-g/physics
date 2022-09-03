#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "MatrixStack.h"

#define FOVY_SPEED 0.1f
#define MAP_ZOOM 200.0f

Camera::Camera() :
	aspect(1.0f),
	fovy((float)(45.0*M_PI/180.0)),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.0, 0.0),
	translations(0.0f, 1.0f, -5.0f),
	pos(50.0f,1.0f,50.0f),
	yaw(glm::pi<float>()),
	pitch(0.0f)
	{}

Camera::~Camera(){}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const{
	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}

void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const{
	MV->multMatrix(glm::lookAt(pos, pos + glm::normalize(glm::vec3(sin(yaw)*cos(pitch), sin(pitch), cos(yaw)*cos(pitch))), glm::vec3(0.0,1.0,0.0) ));
}

void Camera::increment_fovy() {
	fovy += 0.1 * FOVY_SPEED;
	if (fovy > 114.0f * glm::pi<float>()/180.0f) fovy = 114.0f * glm::pi<float>()/180.0f;
}
	
void Camera::decrement_fovy() {
	fovy -= 0.1 * FOVY_SPEED;
	if (fovy < 4.0f * glm::pi<float>()/180.0f) fovy = 4.0f * glm::pi<float>()/180.0f;
}
