#pragma  once
#ifndef COMPUTE_H
#define COMPUTE_H

#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLEW_STATIC
#include <GL/glew.h>
#include "GLSL.h"
#include <random>
#include <vector>
#include <memory>
#include <map>

struct pos {
    float x, y, z, size; // positions
};
struct vel {
    float x, y, z, mass; // velocities
};
struct color {
    float r, g, b, lifetime; // colors
};

class Compute {
    public:
    Compute(){};
    Compute(int max_amount){

         std::default_random_engine engine((unsigned) 1);

        std::normal_distribution<float> position_random(0.0f, 20.0f);
        std::normal_distribution<float> velocity_random(0.0f, 30.0f);
        std::normal_distribution<float> color_random(0.5f, 0.1f);
        std::normal_distribution<float> lifetime_random(10.0f, 1.0f);
        std::normal_distribution<float> mass_random(5.0f, 1.0f);
        std::normal_distribution<float> size_random(1.0f, 0.5f);

        GLint rc;
        pid = 0;
        cShaderName = "../resources/particle_comp.glsl";
        verbose = true;
        pid = glCreateProgram();

        GLuint CS = glCreateShader(GL_COMPUTE_SHADER);
		const char *cshader = GLSL::textFileRead(cShaderName.c_str());
		glShaderSource(CS, 1, &cshader, NULL);
		glCompileShader(CS);
		glGetShaderiv(CS, GL_COMPILE_STATUS, &rc);
		if(!rc) {
			if(verbose) {
				GLSL::printShaderInfoLog(CS);
				std::cout << "Error compiling compute shader " << cShaderName << std::endl;
			}
		}
		glAttachShader(pid, CS);

        glLinkProgram(pid);
        glGetProgramiv(pid, GL_LINK_STATUS, &rc);
        if(!rc) {
            if(verbose) {
                GLSL::printProgramInfoLog(pid);
                std::cout << "Error linking shaders "<< std::endl;
            }
        }

        glGenBuffers( 1, &posSSbo);
        glBindBuffer( GL_SHADER_STORAGE_BUFFER, posSSbo );
        glBufferData( GL_SHADER_STORAGE_BUFFER, max_amount * sizeof(struct pos), NULL, GL_STATIC_DRAW );
        GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT ; // the invalidate makes a big difference when re-writing

        struct pos *points = (struct pos *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, max_amount * sizeof(struct pos), bufMask );
        for( int i = 0; i < max_amount; i++ )
        {
            points[ i ].x = position_random(engine);
            points[ i ].y = 30.0f + position_random(engine);
            points[ i ].z = position_random(engine);
            points[ i ].size = size_random(engine);
        }
        glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

        glGenBuffers( 1, &velSSbo);
        glBindBuffer( GL_SHADER_STORAGE_BUFFER, velSSbo );
        glBufferData( GL_SHADER_STORAGE_BUFFER, max_amount * sizeof(struct vel), NULL, GL_STATIC_DRAW );
        struct vel *vels = (struct vel *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, max_amount * sizeof(struct vel), bufMask );
        for( int i = 0; i < max_amount; i++ )
        {
            vels[ i ].x = velocity_random(engine);
            vels[ i ].y = velocity_random(engine);
            vels[ i ].z = velocity_random(engine);
            vels[ i ].mass = mass_random(engine);
        }
        glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

        glGenBuffers( 1, &colSSbo);
        glBindBuffer( GL_SHADER_STORAGE_BUFFER, colSSbo );
        glBufferData( GL_SHADER_STORAGE_BUFFER, max_amount * sizeof(struct color), NULL, GL_STATIC_DRAW );
        struct color *colors = (struct color *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, max_amount * sizeof(struct color), bufMask );
        for( int i = 0; i < max_amount; i++ )
        {
            colors[ i ].r = color_random(engine);
            colors[ i ].g = color_random(engine);
            colors[ i ].b = color_random(engine);
            colors[ i ].lifetime = lifetime_random(engine);
        }
        glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );



        GLSL::checkError(GET_FILE_LINE);
    }

    GLuint getPosId(){ return posSSbo; };
    GLuint getColId() { return colSSbo; };

    void compute_me() {
        glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 4, posSSbo );
        glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 5, velSSbo );
        glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 6, colSSbo );

        glUseProgram(pid);

        glDispatchCompute( 1024, 1, 1 );
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glUseProgram(0);
    }
    
    std::string cShaderName;
    GLuint pid;
    GLuint posSSbo, velSSbo, colSSbo;
	std::map<std::string,GLint> attributes;
	std::map<std::string,GLint> uniforms;
    bool verbose;

    std::default_random_engine engine;

    std::normal_distribution<float> position_random;
    std::normal_distribution<float> velocity_random;
    std::normal_distribution<float> color_random;
    std::normal_distribution<float> lifetime_random;
    std::normal_distribution<float> mass_random;
    std::normal_distribution<float> size_random;
};

#endif