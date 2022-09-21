#include "Particles.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "GLSL.h"
#include <iostream>

Particles::Particles(int amount=10) : amount(amount), max_amount(amount) {
    std::default_random_engine engine((unsigned) 1);

    std::normal_distribution<float> position_random(0.0f, 20.0f);
    std::normal_distribution<float> velocity_random(0.0f, 5.0f);
    std::normal_distribution<float> color_random(150.0f, 20.0f);
    std::normal_distribution<float> lifetime_random(10.0f, 1.0f);
    std::normal_distribution<float> mass_random(5.0f, 1.0f);
    std::normal_distribution<float> size_random(1.0f, 0.5f);

    positions.resize(amount * 4);
    velocities.resize(amount * 3);
    colors.resize(amount * 4);
    lifetimes.resize(amount);
    masses.resize(amount);
    sizes.resize(amount);

    for (int i = 0; i < amount; ++i) {
        for (int j = 0; j < 3; ++j){
            velocities[3*i+j] = velocity_random(engine);
        }

        lifetimes[i] = lifetime_random(engine);
        masses[i] = mass_random(engine);
        sizes[i] = size_random(engine);
    }
    for (int i = 0; i < amount*4; i+=4) {
        positions[i] = position_random(engine);
        positions[i+1] = position_random(engine);
        positions[i+2] = position_random(engine);
        positions[i+3] = size_random(engine);
        std::cout<<positions[i]<<std::endl;

        colors[i] = (unsigned char) color_random(engine);
        colors[i+1] = (unsigned char) color_random(engine);
        colors[i+2] = (unsigned char) color_random(engine);
        colors[i+3] = (unsigned char) 255.0f;
    }
};

void Particles::init() {

    vaoId = 1;
	glGenVertexArrays(0, &vaoId);
	glBindVertexArray(vaoId);

    GLfloat g_vertex_buffer_data[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    };

    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    glGenBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, amount * 4 * sizeof(GLfloat), &positions[0], GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, amount * 4 * sizeof(GLubyte), &colors[0], GL_STREAM_DRAW);

    GLSL::checkError(GET_FILE_LINE);
}

Particles::~Particles() {};

void Particles::update(float _dt){
    return;
};

void Particles::draw(const std::shared_ptr<Program> prog){

    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    //glBufferData(GL_ARRAY_BUFFER, max_amount * 4 * sizeof(GLfloat), &positions[0], GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(float) * 4, &positions[0]);

    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    //glBufferData(GL_ARRAY_BUFFER, max_amount * 4 * sizeof(GLubyte), &colors[0], GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(unsigned char) * 4, &colors[0]);


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
    // 2nd attribute buffer : positions of particles' centers
    glEnableVertexAttribArray(positionsID);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glVertexAttribPointer(
    positionsID, // attribute. No particular reason for 1, but must match the layout in the shader.
    4, // size : x + y + z + size => 4
    GL_FLOAT, // type
    GL_FALSE, // normalized?
    0, // stride
    (void*)0 // array buffer offset
    );

    int colorsID = prog->getAttribute("color");
    // 3rd attribute buffer : particles' colors
    glEnableVertexAttribArray(colorsID);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glVertexAttribPointer(
    colorsID, // attribute. No particular reason for 1, but must match the layout in the shader.
    4, // size : r + g + b + a => 4
    GL_UNSIGNED_BYTE, // type
    GL_TRUE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
    0, // stride
    (void*)0 // array buffer offset
    );

    glVertexAttribDivisor(verticesID, 0); // particles vertices : always reuse the same 4 vertices -> 0
    glVertexAttribDivisor(positionsID, 1); // positions : one per quad (its center) -> 1
    glVertexAttribDivisor(colorsID, 1); // color : one per quad -> 1

    // Draw the particules !
    // This draws many times a small triangle_strip (which looks like a quad).
    // This is equivalent to :
    // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
    // but faster.
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, amount);

    glDisableVertexAttribArray(verticesID);
    glDisableVertexAttribArray(positionsID);
    glDisableVertexAttribArray(colorsID);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
};

