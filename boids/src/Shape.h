#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include <vector>
#include <memory>

class Program;

class Shape
{
public:
	Shape();
	~Shape();
	void loadMesh(const std::string &meshName);
	void createSphere(int parameter);
	void fitToUnitBox();
	void init();
	void draw(const std::shared_ptr<Program> prog) const;

	std::vector<float> getPosBuf() const { 
		return posBuf; 
	}
	std::vector<float> getNorBuf() const {
		return norBuf;
	}
	unsigned getPosBufID() const {
		return posBufID;
	}
	unsigned getNorBufID() const {
		return norBufID;
	}
private:
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	std::vector<unsigned int> indBuf;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
	unsigned indBufID;
};

#endif
