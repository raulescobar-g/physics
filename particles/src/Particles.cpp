#include "Particles.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "GLSL.h"
#include <iostream>



Particles::Particles(int amount=10) : amount(amount), max_amount(amount) {};

void Particles::init() {
    compute = Compute(1024 * 1024 );

    GLfloat g_vertex_buffer_data[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    };

    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

}

Particles::~Particles() {};

void Particles::update(float _dt){

    compute.compute_me();
};

void Particles::draw(const std::shared_ptr<Program> prog){

    int verticesID = prog->getAttribute("vertices");
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(verticesID);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glVertexAttribPointer(
    verticesID, // attribute. No particular reason for 0, but must match the layout in the shader.
    3, // size
    GL_FLOAT, // type
    GL_FALSE, // normalized?
    0, // stride
    (void*)0 // array buffer offset
    );

    int positionsID = prog->getAttribute("position");
    // // 2nd attribute buffer : positions of particles' centers
    glEnableVertexAttribArray(positionsID);
    glBindBuffer(GL_ARRAY_BUFFER, compute.getPosId());
    glVertexAttribPointer(
    positionsID, // attribute. No particular reason for 1, but must match the layout in the shader.
    4, // size : x + y + z + size => 4
    GL_FLOAT, // type
    GL_FALSE, // normalized?
    0, // stride
    (void*)0 // array buffer offset
    );
    glEnableClientState( GL_VERTEX_ARRAY );

    int colorsID = prog->getAttribute("color");
    // // 3rd attribute buffer : particles' colors
    glEnableVertexAttribArray(colorsID);
    glBindBuffer(GL_ARRAY_BUFFER, compute.getColId());
    glVertexAttribPointer(
    colorsID, // attribute. No particular reason for 1, but must match the layout in the shader.
    4, // size : r + g + b + a => 4
    GL_FLOAT, // type
    GL_TRUE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
    0, // stride
    (void*)0 // array buffer offset
    );
    glEnableClientState( GL_VERTEX_ARRAY );

    glVertexAttribDivisor(verticesID, 0); // particles vertices : always reuse the same 4 vertices -> 0
    glVertexAttribDivisor(positionsID, 1); // positions : one per quad (its center) -> 1
    glVertexAttribDivisor(colorsID, 1); // color : one per quad -> 1

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, amount);

    glDisableVertexAttribArray(verticesID);
    glDisableVertexAttribArray(positionsID);
    glDisableVertexAttribArray(colorsID);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
};

