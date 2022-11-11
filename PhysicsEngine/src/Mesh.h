#pragma once
#ifndef MESH_H
#define MESH_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector> 
#include <memory>
#include <string>

class Program;

#define BASE_DIR "C:\\Users\\raul3\\Programming\\physics\\PhysicsEngine\\resources\\"

class Mesh {
    public:
        Mesh();
        Mesh(std::string);
        void loadMesh(const std::string &meshName);
        void fitToUnitBox();
        void init();
        void draw(const Program& prog) const;

        std::vector<glm::vec3> operator *(float scale);

        std::vector<glm::vec3> get_vertices();
        
    private:
        std::string base_dir = BASE_DIR;

        std::vector<glm::vec3> posBuf;
        std::vector<glm::vec3> norBuf;
        std::vector<glm::vec2> texBuf;
        std::vector<unsigned int> indBuf;

        unsigned posBufID;
        unsigned norBufID;
        unsigned texBufID;
        unsigned indBufID;
};

#endif