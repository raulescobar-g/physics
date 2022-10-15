#pragma once
#ifndef CLOTH_H
#define CLOTH_H

#include <string>
#include <vector>
#include <memory>

#include "Shape.h"

class Program;

class Cloth : public Shape {
	public:
        Cloth();
        ~Cloth();
		void init();
		void draw(const std::shared_ptr<Program> prog) const;
	
};

#endif