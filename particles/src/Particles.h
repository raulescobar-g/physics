#pragma  once
#ifndef PARTICLES_H
#define PARTICLES_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <vector>

class Particles {
    public: 
    Particles(int radius=100.0f,int amount=100){
        std::default_random_engine engine((unsigned) 1);

        std::normal_distribution<float> position_random(, );
        std::normal_distribution<float> velocity_random(, );
        std::normal_distribution<float> color_random(, );
        std::normal_distribution<float> lifetime_random(, );
        std::normal_distribution<float> mass_random(, );
        std::normal_distribution<float> size_random(, );

        positions.resize(amount);
        velocities.resize(amount);
        colors.resize(amount);
        lifetime.resize(amount);
        mass.resize(amount);
        size.resize(amount);

        for (int i = 0; i < amount; ++i) {
            positions[i] = ;
            velocities[i] = ;
            colors[i] = ;
            lifetimes[i] = ;
            masses[i] = ;
            sizes[i] = ;
        }
    };
    ~Particles(){};

    void init(){};
    void update(){};
    void draw(){};

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
    std::vector<glm::vec3> colors;
    std::vector<float> lifetimes;
    std::vector<float> masses;
    std::vector<float> sizes;

    std::normal_distribution<float> position_random;
    std::normal_distribution<float> velocity_random;
    std::normal_distribution<float> color_random;
    std::normal_distribution<float> lifetime_random;
    std::normal_distribution<float> mass_random;
    std::normal_distribution<float> size_random;

    std::default_random_engine engine;

    private: 
    GLuint pid;
	std::map<std::string,GLint> attributes;
	std::map<std::string,GLint> uniforms;
	bool verbose;
};

#endif