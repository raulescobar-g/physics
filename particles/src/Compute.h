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



class Compute {
    public:
    Compute(){};
    Compute(glm::vec3 s, std::vector<float> p, std::vector<float> v, std::vector<float> c, std::vector<float> l): size(s) {
        GLint rc;
        pid = 0;
        cShaderName = "../resources/particle_comp.glsl";
        verbose = true;
        pid = glCreateProgram();

        GLuint CS = glCreateShader(GL_COMPUTE_SHADER);
		const char *cshader = GLSL::textFileRead(cShaderName.c_str());
		glShaderSource(CS, 1, &cshader, NULL);

		// Compile vertex shader
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
        glBufferData( GL_SHADER_STORAGE_BUFFER, p.size() * sizeof(GLfloat), NULL, GL_STATIC_DRAW );
        GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT ; // the invalidate makes a big difference when re-writing

        struct pos *points = (struct pos *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct pos), bufMask );
        for( int i = 0; i < NUM_PARTICLES; i++ )
        {
            points[ i ].x = Ranf( XMIN, XMAX );
            points[ i ].y = Ranf( YMIN, YMAX );
            points[ i ].z = Ranf( ZMIN, ZMAX );
            points[ i ].w = 1.;
        }
        glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
        glGenBuffers( 1, &velSSbo);
        glBindBuffer( GL_SHADER_STORAGE_BUFFER, velSSbo );
        glBufferData( GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct vel), NULL, GL_STATIC_DRAW );
        struct vel *vels = (struct vel *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct vel), bufMask );
        for( int i = 0; i < NUM_PARTICLES; i++ )
        {
            vels[ i ].vx = Ranf( VXMIN, VXMAX );
            vels[ i ].vy = Ranf( VYMIN, VYMAX );
            vels[ i ].vz = Ranf( VZMIN, VZMAX );
            vels[ i ].vw = 0.;
        }
        glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );



        GLSL::checkError(GET_FILE_LINE);
    }

    void compute_me() {
        glUseProgram(pid);
        glDispatchCompute(ceil(size.x / 8.0f), ceil(size.y / 4.0f), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glUseProgram(0);
    }
    
    std::string cShaderName;
    glm::vec3 size;
    GLuint pid;
    GLuint posSSbo;
    GLuint velSSbo
    GLuint colSSbo;
	std::map<std::string,GLint> attributes;
	std::map<std::string,GLint> uniforms;
    bool verbose;
};

#endif