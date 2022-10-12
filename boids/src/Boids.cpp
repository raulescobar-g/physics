#include "Boids.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "GLSL.h"
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
    int amount_of_triangles;
};
struct aabb {
    glm::vec4 minimum, maximum;
};
struct cell {
    int idx;
};

Boids::Boids(){};

void extract_triangles(std::shared_ptr<Object> obj, std::vector<triangle>& triangles, std::vector<aabb>& boxes, std::vector<data>& amount_of_triangles) {
    std::vector<float> posBuf = obj->shape->getPosBuf();
    int sum = 0;

    glm::vec4 minimum = glm::vec4(10000.0f, 10000.0f, 10000.0f, 1.0f);
    glm::vec4 maximum = glm::vec4(-10000.0f, -10000.0f, -10000.0f, 1.0f);

    glm::mat4 MV = glm::translate(glm::mat4(1.0f), obj->position);
    MV *= glm::scale(glm::mat4(1.0f), obj->scale);
    if (glm::length(obj->rotation) > 0.001f) MV *= glm::rotate(glm::mat4(1.0f), glm::length(obj->rotation), glm::normalize(obj->rotation));
            
    
    for (int i = 0; i < posBuf.size(); i += 9){
        triangle tri;
        tri.v1 = MV * glm::vec4(posBuf[i], posBuf[i+1], posBuf[i+2], 1.0f);
        tri.v2 = MV * glm::vec4(posBuf[i+3], posBuf[i+4], posBuf[i+5], 1.0f);
        tri.v3 = MV * glm::vec4(posBuf[i+6], posBuf[i+7], posBuf[i+8], 1.0f);
            
        minimum.x = glm::min(minimum.x, tri.v1.x);
        minimum.y = glm::min(minimum.y, tri.v1.y);
        minimum.z = glm::min(minimum.z, tri.v1.z);

        maximum.x = glm::max(maximum.x, tri.v1.x);
        maximum.y = glm::max(maximum.y, tri.v1.y);
        maximum.z = glm::max(maximum.z, tri.v1.z);

        minimum.x = glm::min(minimum.x, tri.v2.x);
        minimum.y = glm::min(minimum.y, tri.v2.y);
        minimum.z = glm::min(minimum.z, tri.v2.z);

        maximum.x = glm::max(maximum.x, tri.v2.x);
        maximum.y = glm::max(maximum.y, tri.v2.y);
        maximum.z = glm::max(maximum.z, tri.v2.z);

        minimum.x = glm::min(minimum.x, tri.v3.x);
        minimum.y = glm::min(minimum.y, tri.v3.y);
        minimum.z = glm::min(minimum.z, tri.v3.z);

        maximum.x = glm::max(maximum.x, tri.v3.x);
        maximum.y = glm::max(maximum.y, tri.v3.y);
        maximum.z = glm::max(maximum.z, tri.v3.z);
        

        triangles.push_back(tri);
        sum += 1;
    }

    struct aabb box;
    minimum.w = 0.0f;
    maximum.w = 0.0f;
    box.minimum = minimum - glm::vec4(0.0001f, 0.0001f, 0.0001f, 0.0f);
    box.maximum = maximum + glm::vec4(0.0001f, 0.0001, 0.0001f, 0.0f);
    boxes.push_back(box);

    data temp;
    temp.amount_of_triangles = sum;
    amount_of_triangles.push_back(temp);
}

int Boids::get_poly_count() {
    return triangle_count;
}

void Boids::load_boid_mesh() {
    boid_mesh = std::make_shared<Shape>();
    boid_mesh->loadMesh("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\fishy.obj");
    boid_mesh->fitToUnitBox();
    boid_mesh->init();

    predator_mesh = std::make_shared<Shape>();
    predator_mesh->loadMesh("C:\\Users\\raul3\\Programming\\physics\\boids\\resources\\SharkBoi.obj");
    predator_mesh->fitToUnitBox();
    predator_mesh->init();
}

void Boids::buffer_world_geometry(const std::vector<std::shared_ptr<Object> >& objects) {
    if (objects.size() > 0) {
        // compute stuff
        std::vector<triangle> triangles;
        std::vector<data> amount_of_triangles;
        std::vector<aabb> boxes;

        for (auto obj : objects) {
            extract_triangles(obj, triangles, boxes, amount_of_triangles);
        }

        glGenBuffers( 1, &objSSbo);
        glBindBuffer( GL_SHADER_STORAGE_BUFFER, objSSbo );
        glBufferData( GL_SHADER_STORAGE_BUFFER, triangles.size() * sizeof(struct triangle), &triangles[0], GL_STATIC_DRAW );

        glGenBuffers( 1, &aabbSSbo);
        glBindBuffer( GL_SHADER_STORAGE_BUFFER, aabbSSbo );
        glBufferData( GL_SHADER_STORAGE_BUFFER, boxes.size() * sizeof(struct aabb), &boxes[0], GL_STATIC_DRAW );

        triangle_count = triangles.size();
        glGenBuffers( 1, &dataSSbo);
        glBindBuffer( GL_SHADER_STORAGE_BUFFER, dataSSbo );
        glBufferData( GL_SHADER_STORAGE_BUFFER, amount_of_triangles.size() * sizeof(struct data), &amount_of_triangles[0], GL_STATIC_DRAW );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        GLSL::checkError(GET_FILE_LINE);   
    } else {
        triangle_count = 0;
    } 
}

glm::vec4 Boids::get_display_data() {

    int amount = max_amount;

    return glm::vec4(max_amount, 0, 0, 0);
}

void Boids::init(int max, int _predators, int _dups, int _dims, float space) {
    max_amount = max;
    predators = _predators;

    spacing = space;
    dims = _dims;
    dups = _dups; // how many should fit in a box 1/8 of total amount
    int volume = dims * dims * dims * dups; // here
    std::cout<<"volume: "<<volume<<", dims: "<<dims<<"\n";

    engine = std::default_random_engine((unsigned) 1);
    unit_normal = std::normal_distribution<float>(0.0f, 1.0f);

    GLint bufMask = GL_MAP_WRITE_BIT |  GL_MAP_READ_BIT |  GL_MAP_PERSISTENT_BIT |  GL_MAP_COHERENT_BIT;

    // compute stuff
    glGenBuffers( 1, &posSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, posSSbo );
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, (max_amount+predators) * sizeof(struct position), NULL, bufMask | GL_DYNAMIC_STORAGE_BIT);
    positions = (struct position *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, (max_amount+predators) * sizeof(struct position), bufMask );

    glGenBuffers( 1, &velSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, velSSbo );
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, (max_amount+predators) * sizeof(struct velocity), NULL, bufMask | GL_DYNAMIC_STORAGE_BIT );
    velocities = (struct velocity *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, (max_amount+predators) * sizeof(struct velocity), bufMask );

    glGenBuffers( 1, &colSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, colSSbo );
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, (max_amount+predators) * sizeof(struct color), NULL, bufMask | GL_DYNAMIC_STORAGE_BIT );
    colors = (struct color *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, (max_amount+predators) * sizeof(struct color), bufMask );

    glGenBuffers( 1, &gridSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, gridSSbo );
    glBufferStorage( GL_SHADER_STORAGE_BUFFER, volume * sizeof(struct cell), NULL, bufMask | GL_DYNAMIC_STORAGE_BIT);
    grid = (struct cell *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, volume * sizeof(struct cell), bufMask );

    spawn_boids();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GLSL::checkError(GET_FILE_LINE); 
}

int hash(float x, float y, float z, int dups, float spacing, int dims) {
    float ds = spacing / (float)dims; 
    float offset = spacing / 2.0f;
    //std::cout<<"x: "<<x<<", y: "<<y<<", z: "<<z<<std::endl;
    int xi = ((int) std::floor((x+offset) / ds)) * dups;
    int yi = ((int) std::floor((y+offset) / ds)) * dims * dups;
    int zi = ((int) std::floor((z+offset) / ds)) * dims * dims * dups;
    return xi + yi + zi;
}

void Boids::spawn_boids() {
    for( int i = 0; i < max_amount + predators; ++i ){   
        positions[ i ].x = glm::clamp(unit_normal(engine) * (spacing/4.0f), -2.0f * spacing/5.0f, 2.0f * spacing/5.0f);
        positions[ i ].y = glm::clamp(unit_normal(engine) * (spacing/4.0f), -2.0f * spacing/5.0f, 2.0f * spacing/5.0f);
        positions[ i ].z = glm::clamp(unit_normal(engine) * (spacing/4.0f), -2.0f * spacing/5.0f, 2.0f * spacing/5.0f);

        velocities[ i ].x = unit_normal(engine) * 1.0f;
        velocities[ i ].y = unit_normal(engine) * 1.0f;
        velocities[ i ].z = unit_normal(engine) * 1.0f;

        colors[ i ].r = std::abs(unit_normal(engine))/2;
        colors[ i ].g = std::abs(unit_normal(engine))/2;
        colors[ i ].b = std::abs(unit_normal(engine))/2;

        positions[ i ].size = i >= max_amount ? 10.0f : 0.5f + std::abs(unit_normal(engine));
        velocities[ i ].mass = 1.0f;
        colors[ i ].lifetime = i >= max_amount ? -1.0f : 1.0f; // lifetime signifies if predator or boid
    }


    for (int i = 0; i < dims * dims * dims * dups; ++i) {
        grid[i].idx = -1;
    }
    for( int i = 0; i < max_amount + predators; ++i ){
        int idx = hash(positions[i].x, positions[i].y, positions[i].z, dups, spacing, dims);
        int offset = 0;
        while (grid[idx + offset].idx != -1) {
            ++offset;
            if (offset >= dups) {
                std::cout<<"does not fit in here bro \n";
                exit(-1);
            }
        }
        grid[idx + offset].idx = i;
    }

}

Boids::~Boids() {};

void Boids::update(){

    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 4, posSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 5, velSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 6, colSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 7, objSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 8, dataSSbo );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 9, aabbSSbo);
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 13, gridSSbo );
    
    
    glDispatchCompute( max_amount + predators, 1, 1 );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);   
};

void Boids::draw_boids(const std::shared_ptr<Program> particle_render_program) const {
    // Bind position buffer
	int h_pos = particle_render_program->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, boid_mesh->getPosBufID());
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	int h_nor = particle_render_program->getAttribute("aNor");
	if(h_nor != -1 && boid_mesh->getNorBufID() != 0) {
		glEnableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, boid_mesh->getNorBufID());
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

    int positionsID = particle_render_program->getAttribute("position");
    glEnableVertexAttribArray(positionsID);
    glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
    glVertexAttribPointer(positionsID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    int colorsID = particle_render_program->getAttribute("color");
    glEnableVertexAttribArray(colorsID);
    glBindBuffer(GL_ARRAY_BUFFER, colSSbo);
    glVertexAttribPointer(colorsID, 4, GL_FLOAT, GL_TRUE, 0, (void*)0);

    int velocityID = particle_render_program->getAttribute("velocity");
    glEnableVertexAttribArray(velocityID);
    glBindBuffer(GL_ARRAY_BUFFER, velSSbo);
    glVertexAttribPointer(velocityID, 4, GL_FLOAT, GL_TRUE, 0, (void*)0);

    glVertexAttribDivisor(h_pos, 0); 
    glVertexAttribDivisor(h_nor, 0); 
    glVertexAttribDivisor(positionsID, 1);
    glVertexAttribDivisor(colorsID, 1); 
    glVertexAttribDivisor(velocityID, 1); 
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, boid_mesh->getPosBuf().size() / 3, max_amount);

    glDisableVertexAttribArray(h_pos);
    glDisableVertexAttribArray(h_nor);
    glDisableVertexAttribArray(positionsID);
    glDisableVertexAttribArray(colorsID);
    glDisableVertexAttribArray(velocityID);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Boids::draw_predators(const std::shared_ptr<Program> particle_render_program) const {

	int h_pos = particle_render_program->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, predator_mesh->getPosBufID());
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	int h_nor = particle_render_program->getAttribute("aNor");
	if(h_nor != -1 && predator_mesh->getNorBufID() != 0) {
		glEnableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, predator_mesh->getNorBufID());
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

    int offset = max_amount * 4 * sizeof(float);

    int positionsID = particle_render_program->getAttribute("position");
    glEnableVertexAttribArray(positionsID);
    glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
    glVertexAttribPointer(positionsID, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*) offset);

    int colorsID = particle_render_program->getAttribute("color");
    glEnableVertexAttribArray(colorsID);
    glBindBuffer(GL_ARRAY_BUFFER, colSSbo);
    glVertexAttribPointer(colorsID, 4, GL_FLOAT, GL_TRUE, 0, (GLvoid*) offset);

    int velocityID = particle_render_program->getAttribute("velocity");
    glEnableVertexAttribArray(velocityID);
    glBindBuffer(GL_ARRAY_BUFFER, velSSbo);
    glVertexAttribPointer(velocityID, 4, GL_FLOAT, GL_TRUE, 0, (GLvoid*) offset);

    glVertexAttribDivisor(h_pos, 0); 
    glVertexAttribDivisor(h_nor, 0); 
    glVertexAttribDivisor(positionsID, 1);
    glVertexAttribDivisor(colorsID, 1); 
    glVertexAttribDivisor(velocityID, 1); 
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, predator_mesh->getPosBuf().size() / 3, predators);

    glDisableVertexAttribArray(h_pos);
    glDisableVertexAttribArray(h_nor);
    glDisableVertexAttribArray(positionsID);
    glDisableVertexAttribArray(colorsID);
    glDisableVertexAttribArray(velocityID);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
};
