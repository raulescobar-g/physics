#include "Particles.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "GLSL.h"
#include "MatrixStack.h"

#include <iostream>


struct position {
    float x, y, z, size;
};
struct velocity {
    float x, y, z, mass;
};
struct color {
    float r, g, b, lifetime;
};
struct triangle { 
    glm::vec4 v1,v2,v3;
};

struct transform {
    glm::mat4 T;
};

struct data {
    uint amount_of_triangles;
};

struct point_attractor {
    float mass, x, y, z; 
};
// another struct to send bounding box and transform instead of sending for each one


Particles::Particles(){};

void extract_triangles(std::shared_ptr<Object> obj, std::vector<triangle>& triangles, std::vector<data>& amount_of_triangles) {
    std::vector<float> posBuf = obj->shape->getPosBuf();
    int sum = 0;
    
    for (int i = 0; i < posBuf.size(); i += 9){
        triangle tri;
        tri.v1 = glm::vec4(posBuf[i], posBuf[i+1], posBuf[i+2], 1.0f);
        tri.v2 = glm::vec4(posBuf[i+3], posBuf[i+4], posBuf[i+5], 1.0f);
        tri.v3 = glm::vec4(posBuf[i+6], posBuf[i+7], posBuf[i+8], 1.0f);
        triangles.push_back(tri);
        sum += 1;
    }
    data temp;
    temp.amount_of_triangles = sum;
    amount_of_triangles.push_back(temp);
}

int Particles::get_poly_count() {
    return triangle_count;
}

void Particles::load_particle_mesh() {
    GLfloat temp[12] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    };

    for (int i = 0; i < 12; ++i) {
        g_vertex_buffer_data[i] = temp[i];
    }
}

void Particles::buffer_world_geometry(const std::vector<std::shared_ptr<Object> >& objects) {
    // compute stuff
    std::vector<triangle> triangles;
    std::vector<data> amount_of_triangles;
    for (auto obj : objects) {
        extract_triangles(obj, triangles, amount_of_triangles);
    }
    glGenBuffers( 1, &objSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, objSSbo );
    glBufferData( GL_SHADER_STORAGE_BUFFER, triangles.size() * sizeof(struct triangle), &triangles[0], GL_STATIC_DRAW );

    std::vector<transform> transforms;
    for (auto obj : objects) {
        glm::mat4 MV = glm::translate(glm::mat4(1.0f), obj->pos);
        MV *= glm::scale(glm::mat4(1.0f), glm::vec3(obj->scale, obj->scale, obj->scale));
        if (glm::length(obj->rotation) > 0.001f) MV *= glm::rotate(glm::mat4(1.0f), glm::length(obj->rotation), glm::normalize(obj->rotation));
        
        transform t;
        t.T = MV;
        transforms.push_back(t);
    }
    glGenBuffers( 1, &transSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, transSSbo );
    glBufferData( GL_SHADER_STORAGE_BUFFER, transforms.size() * sizeof(struct transform), &transforms[0], GL_STATIC_DRAW );


    triangle_count = triangles.size();
    glGenBuffers( 1, &dataSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, dataSSbo );
    glBufferData( GL_SHADER_STORAGE_BUFFER, amount_of_triangles.size() * sizeof(struct data), &amount_of_triangles[0], GL_STATIC_DRAW );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GLSL::checkError(GET_FILE_LINE);    
}

void Particles::buffer_attractors(std::vector<glm::vec4> attractors) {

    std::vector<point_attractor> attractor_structs;

    for (auto a : attractors) {
        point_attractor a_temp;

        a_temp.mass = a.x;
        a_temp.x = a.y;
        a_temp.y = a.z;
        a_temp.z = a.w;

        attractor_structs.push_back(a_temp);
    }

    glGenBuffers( 1, &attrSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, attrSSbo );
    glBufferData( GL_SHADER_STORAGE_BUFFER, attractor_structs.size() * sizeof(point_attractor), &attractor_structs[0], GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GLSL::checkError(GET_FILE_LINE);   

}

glm::vec4 Particles::get_particle_count_data() {
    return glm::vec4(initial, current, target, max_amount);
}

void Particles::init(int max, int desired, int start, const std::shared_ptr<Program> compute_program) {
    max_amount = max;
    initial = start;
    target = desired;
    current = 0;

    engine = std::default_random_engine((unsigned) 1);
    unit_normal = std::normal_distribution<float>(0.0f, 1.0f);

    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

    // render stuff
    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // compute stuff
    glGenBuffers( 1, &posSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, posSSbo );
    glBufferData( GL_SHADER_STORAGE_BUFFER, max_amount * sizeof(struct position), NULL, GL_STATIC_DRAW );
    positions = (struct position *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, max_amount * sizeof(struct position), bufMask );
    for( int i = 0; i < initial; ++i )
    {
        positions[ i ].x = unit_normal(engine) * 3.0f;
        positions[ i ].y = 30.0f + unit_normal(engine) * 10.0f;
        positions[ i ].z = unit_normal(engine) * 3.0f;
        positions[ i ].size = std::abs(unit_normal(engine)) * 0.01 + 0.03;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

    glGenBuffers( 1, &velSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, velSSbo );
    glBufferData( GL_SHADER_STORAGE_BUFFER, max_amount * sizeof(struct velocity), NULL, GL_STATIC_DRAW );
    velocities = (struct velocity *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, max_amount * sizeof(struct velocity), bufMask );
    for( int i = 0; i < initial ; ++i )
    {
        velocities[ i ].x = unit_normal(engine) * 2.0f;
        velocities[ i ].y = unit_normal(engine) * 2.0f;
        velocities[ i ].z = unit_normal(engine) * 2.0f;
        velocities[ i ].mass = std::abs(unit_normal(engine)) * 5.0f;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

    glGenBuffers( 1, &colSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, colSSbo );
    glBufferData( GL_SHADER_STORAGE_BUFFER, max_amount * sizeof(struct color), NULL, GL_STATIC_DRAW );
    colors = (struct color *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, max_amount * sizeof(struct color), bufMask );
    for( int i = 0; i < initial; ++i )
    {
        colors[ i ].r = abs(unit_normal(engine) / 4.0f + 0.5f);
        colors[ i ].g = abs(unit_normal(engine) / 4.0f + 0.5f);
        colors[ i ].b = abs(unit_normal(engine) / 4.0f + 0.5f);
        colors[ i ].lifetime = unit_normal(engine) + 50.0f;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    current = initial;
    current_particle = current;
    GLSL::checkError(GET_FILE_LINE); 
}

Particles::~Particles() {};

// void Particles::detach_spawner() {}

void Particles::update(){
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 4, posSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 5, velSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 6, colSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 7, objSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 8, transSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 9, dataSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 10, attrSSbo );

    glDispatchCompute( 1024, 1, 1 );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
};

void Particles::draw(const std::shared_ptr<Program> particle_render_program, const std::shared_ptr<Program> compute_program) const {

    int verticesID = particle_render_program->getAttribute("vertices");
    glEnableVertexAttribArray(verticesID);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glVertexAttribPointer(verticesID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    int positionsID = particle_render_program->getAttribute("position");
    glEnableVertexAttribArray(positionsID);
    glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
    glVertexAttribPointer(positionsID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableClientState( GL_VERTEX_ARRAY ); // test remove

    int colorsID = particle_render_program->getAttribute("color");
    glEnableVertexAttribArray(colorsID);
    glBindBuffer(GL_ARRAY_BUFFER, colSSbo);
    glVertexAttribPointer(colorsID, 4, GL_FLOAT, GL_TRUE, 0, (void*)0);
    glEnableClientState( GL_VERTEX_ARRAY );

    glVertexAttribDivisor(verticesID, 0); 
    glVertexAttribDivisor(positionsID, 1);
    glVertexAttribDivisor(colorsID, 1); 

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, max_amount);

    glDisableVertexAttribArray(verticesID);
    glDisableVertexAttribArray(positionsID);
    glDisableVertexAttribArray(colorsID);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
};


// void set_watcher(int current_particle, int max_amount, GLuint& colSSbo ,Queue& queue) {

//         glBindBuffer( GL_SHADER_STORAGE_BUFFER, colSSbo );
//         colors = (struct color *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, max_amount * sizeof(struct color), GL_MAP_READ_BIT);

//         while () {

//         }
// }

// void set_reviver() {

// }
