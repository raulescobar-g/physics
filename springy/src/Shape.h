#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include <vector>
#include <memory>

class Program;

class Shape{
	public:
		Shape();
		~Shape();
		void loadMesh(const std::string &meshName);
		void fitToUnitBox();
		virtual void init();
		virtual void draw(const std::shared_ptr<Program> prog) const;

		std::vector<float> getPosBuf() const;
		void setShape(std::vector<float> pos, std::vector<float> nor, std::vector<float> tex, std::vector<unsigned int> ind);
		
	protected:
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
