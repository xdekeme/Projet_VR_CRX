#ifndef PARTICLE_H
#define PARTICLE_H

#include<iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include <random>

#define PI 3.14159265

struct ParticleProperties {
    float x, y, z;       // Position
    float vx, vy, vz;    // Velocity
    float lifespan;      // Remaining lifespan
};

extern float world_scale; //World scale factor defined in main.cpp

class Particle
{
public:
	const int numParticles = 151;
	const float particleRadius = 0.08f * world_scale;
	const float maxParticleSpeed = 0.1f  * world_scale;
	const float maxParticleLifespan = 5.0f  * world_scale;
	std::vector<ParticleProperties> particles;	


	Particle() {
		particles.resize(numParticles);
        initParticle();
	}

	void initParticle() {
		for (int i = 0; i < numParticles; ++i) {
			particles[i].x = randomFloat(0.0f, .0010f * world_scale);
			particles[i].y = randomFloat(-0.0f, 0.5f * world_scale);
			particles[i].vx = randomFloat(-0.005f * world_scale, 0.005f * world_scale);
			particles[i].vy = randomFloat(.5f * world_scale, .7f * world_scale);

			particles[i].lifespan = 80.0f * world_scale;  // Reset lifespan

			particles[i].z = randomFloat(0.0f, .0010f * world_scale);
			particles[i].vz = randomFloat(-0.005f * world_scale, 0.005f * world_scale);
    	}
	}

	void updateParticle(float deltaTime) {
		for (int i = 0; i < numParticles; ++i) {
            particles[i].x += particles[i].vx;
			particles[i].vy -= 0.05 * world_scale;
			particles[i].y += particles[i].vy*deltaTime;
			particles[i].z += particles[i].vz;
			

            particles[i].lifespan -= 1.0f * world_scale;
            if (particles[i].lifespan <= 0.0f) {
				particles[i].x = randomFloat(0.0f, .0010f * world_scale);
                particles[i].y = randomFloat(-0.0f, 0.2f * world_scale);
                particles[i].vx = randomFloat(-0.002f * world_scale, 0.002f * world_scale);
                particles[i].vy = randomFloat(0.8f * world_scale, 1.0f * world_scale);

                particles[i].lifespan = 80.0f * world_scale;  // Reset lifespan

				particles[i].z = randomFloat(0.0f, .0010f * world_scale);
				particles[i].vz = randomFloat(-0.002f * world_scale, 0.002f * world_scale);
            }
        }
	}

	void drawParticles(Shader shaderParticle, Object particle, float angle){
		 for (int i = 0; i < numParticles; ++i) {
			float modifiedX = (particles[i].x * sin(angle) - particles[i].y * cos(angle)) - 2 * cos(angle);
    		float modifiedY = (particles[i].x * cos(angle) + particles[i].y * sin(angle)) + 2 * sin(angle);
			float modifiedZ = particles[i].z ;

			float cos_angle = cos(angle);
			float sin_angle = sin(angle);

			float rotation_matrix[3][3] = {
				{sin(angle), -cos(angle), 0},
				{cos(angle), sin(angle), 0},
				{0, 0, 1}
			};

			float original_x = particles[i].x;
			float original_y = particles[i].y;

			shaderParticle.setMatrix4("M",glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(modifiedX * world_scale, modifiedY * world_scale, modifiedZ * world_scale )), glm::vec3(0.03 * world_scale, 0.03 * world_scale, 0.03 * world_scale)));
			particle.draw();
		}
	}

	float randomFloat(float min, float max) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}
};
#endif