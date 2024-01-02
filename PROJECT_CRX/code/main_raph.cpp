#include<iostream>

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <map>

#include "../headers/camera.h"
#include "../headers/shader.h"
#include "../headers/object.h"
#include "../headers/object_bumping.h"
#include "../headers/colladainterface.h"
#include "../headers/colladaobject.h"
#include "../headers/collision.h"
#include "../headers/particle.h"


//Definition of the constant and the functions
const int width = 700;
const int height = 700;  
Camera camera(glm::vec3(13.0, 5.0, 13.0));
float lastX = (float)width / 2.0;
float lastY = (float)height / 2.0;
bool firstMouse = true;
bool sphere1_draw = false;  
std::vector<ColGeom> geom_vec;
std::vector<Animation> anim_vec; 
bool explosion = false;
bool meteorite = false;
float explosionIntensity = 1.0f; 
float explosionTime = 0.0f;


void RenderingShader(Shader shader);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void loadCubemapFace(const char * file, const GLenum& targetCube);
unsigned int loadTexture(const char *path);



#ifndef NDEBUG
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	//DEBUG Lines have been removed to add clarity (add those if needed)
}
#endif




int main(int argc, char* argv[])
{
	//Boilerplate
	//Create the OpenGL context 
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif


	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "PROJECT - CRX", nullptr, nullptr);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);

	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	glEnable(GL_DEPTH_TEST);

	

#ifndef NDEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	//Creation of the physical world
	btDynamicsWorld* dynamicsWorld = Collision::createPhysicalWorld();

	//Shader with geometry compilation
	char fileVert[] = PATH_TO_SOURCES "/vertSrc.txt";
	char fileFrag[] = PATH_TO_SOURCES "/fragSrc.txt";
	char fileGeom[] = PATH_TO_SOURCES "/geomSrc.txt";
	Shader shader(fileVert, fileFrag, fileGeom);

	//Shader with geometry compilation
	char fileFragPart[] = PATH_TO_SOURCES "/fragSrcParticles.txt";
	Shader shaderParticle = Shader(fileVert, fileFragPart, fileGeom);

	//Shader with bump mapping compilation
	char fileVertBump[] = PATH_TO_SOURCES "/vertSrc_BumpMapping.txt";
	char fileFragBump[] = PATH_TO_SOURCES "/fragSrc_BumpMapping.txt";
	Shader BumpShader = Shader(fileVertBump, fileFragBump);

	//Shader Cubemap compilation
	char fileVertCubeMap[] = PATH_TO_SOURCES "/vertSrcCubeMap.txt";
	char fileFragCubeMap[] = PATH_TO_SOURCES "/fragSrcCubeMap.txt";
	Shader cubeMapShader = Shader(fileVertCubeMap, fileFragCubeMap);

	//SIMPLE OBJ FILE
	char path[] = PATH_TO_OBJECTS "/sphere_coarse.obj";
	char path2[] = PATH_TO_OBJECTS "/meteorite.obj";

	Object sphere1(path); //Earth
	//sphere1.makeObject(shader);
	sphere1.makeObject(shader);
	std::string pathEarthTexture = PATH_TO_TEXTURE "/earth.jpeg";
	unsigned int EarthTexture = loadTexture(pathEarthTexture.c_str());

	Object sphere2(path); //Sun normal + bumping
	BumpingObject sun_bumping(path);
	//sphere2.makeObject(shader);
	//sun_bumping.makeBumpingObject(BumpShader);
	//std::string pathSunTexture = PATH_TO_TEXTURE "/sunv3.jpg";
	//std::string pathSunTexture_normal = PATH_TO_TEXTURE "/sun_normal.jpg";
	//unsigned int SunTexture = loadTexture(pathSunTexture.c_str());
	//unsigned int SunTexture_normal = loadTexture(pathSunTexture_normal.c_str());
	sphere2.model = glm::scale(sphere2.model, glm::vec3(2.0,2.0,2.0));
	sun_bumping.model = glm::scale(sun_bumping.model, glm::vec3(2.0,2.0,2.0));

	Object moon(path); //Moon
	std::string pathMoonTexture = PATH_TO_TEXTURE "/moonv3.jpeg";
	unsigned int MoonTexture = loadTexture(pathMoonTexture.c_str());
	moon.makeObject(shader);

	////collision Earth
	//Object Earth(path); //Object Earth
	//std::string pathEarthTexture = PATH_TO_TEXTURE "/earth.jpeg";
	//unsigned int EarthTexture = loadTexture(pathEarthTexture.c_str());
	//Earth.makeObject(shader);
	//btRigidBody* CollisionObjectEarth = Collision::createRigidBody(dynamicsWorld, 1.0f, 1, btVector3(0, 0, 0)); //Creation of the rigid body
	//btTransform CollisionObjectEarthTransfo;
	//CollisionObjectEarth->getMotionState()->getWorldTransform(CollisionObjectEarthTransfo);
	////CollisionObjectEarth->setLinearVelocity(btVector3(0, 0, 0));
	//Earth.model = glm::translate(glm::mat4(1.0f), glm::vec3(CollisionObjectEarthTransfo.getOrigin().getX(), CollisionObjectEarthTransfo.getOrigin().getY(), CollisionObjectEarthTransfo.getOrigin().getZ()));

	//Object collision1(path2); //Object Collision1
	//std::string pathMeteoriteTexture = PATH_TO_TEXTURE "/meteorite.jpeg";
	//unsigned int MeteoriteTexture = loadTexture(pathMeteoriteTexture.c_str());
	//collision1.makeObject(shader);
	//btRigidBody* CollisionObject1 = Collision::createRigidBody(dynamicsWorld, 1.0f, 1, btVector3(5,0,0)); //Creation of the rigid body
	//btTransform CollisionObject1Transfo;
	//CollisionObject1->getMotionState()->getWorldTransform(CollisionObject1Transfo);
	//CollisionObject1->setLinearVelocity(btVector3(1, 0, 0));
	//collision1.model = glm::translate(glm::mat4(1.0f), glm::vec3(CollisionObject1Transfo.getOrigin().getX(), CollisionObject1Transfo.getOrigin().getY(), CollisionObject1Transfo.getOrigin().getZ()));
	
	std::string pathMeteoriteTexture = PATH_TO_TEXTURE "/meteorite.jpeg";
	unsigned int MeteoriteTexture = loadTexture(pathMeteoriteTexture.c_str());

	int numberOfMeteorites = 30;  // Pour 30 météorites
	std::vector<Object> meteorites;
	std::vector<btRigidBody*> meteoriteBodies;

	for (int i = 0; i < numberOfMeteorites; i++) {
		Object meteorite(path2);  // Remplacez path2 par le chemin approprié
		meteorite.makeObject(shader);

		// Répartir les météorites uniformément autour de l'ellipse
		float angle = i * (2.0f * 3.1416 / numberOfMeteorites);
		glm::vec3 startPosition = glm::vec3(5 * cos(angle), 0, 5 * sin(angle));

		btRigidBody* meteoriteBody = Collision::createRigidBody(dynamicsWorld, 0.5f, 1, btVector3(startPosition.x, startPosition.y, startPosition.z));
		meteoriteBodies.push_back(meteoriteBody);

		// Calculer la direction tangentielle à l'ellipse pour la vitesse
		glm::vec3 tangentDirection = glm::normalize(glm::vec3(-5 * sin(angle), 0, 5 * cos(angle)));
		float speed = 0.4f; // Vitesse constante
		btVector3 velocity = speed * btVector3(tangentDirection.x, tangentDirection.y, tangentDirection.z);

		meteoriteBody->setLinearVelocity(velocity);
		meteorites.push_back(meteorite);
	}



	Object collision2(path); //Object Collision2
	collision2.makeObject(shader);
	btRigidBody* CollisionObject2 = Collision::createRigidBody(dynamicsWorld, 5.0f, 54, btVector3(30,0,0)); //Creation of the rigid body
	btTransform CollisionObject2Transfo;
	CollisionObject2->getMotionState()->getWorldTransform(CollisionObject2Transfo);
	CollisionObject2->setLinearVelocity(btVector3(-2, 0, 0));
	collision2.model = glm::translate(glm::mat4(1.0f), glm::vec3(CollisionObject2Transfo.getOrigin().getX(), CollisionObject2Transfo.getOrigin().getY(), CollisionObject2Transfo.getOrigin().getZ()));

	//NOTE: Il y a un probleme sur l'impact, rayon reeel et rayon lors de l'impact.

	BumpingObject collision3(path); //Object SUN
	std::string pathSunTexture = PATH_TO_TEXTURE "/sunv3.jpg";
	std::string pathSunTexture_normal = PATH_TO_TEXTURE "/sun_normal.jpg";
	unsigned int SunTexture = loadTexture(pathSunTexture.c_str());
	unsigned int SunTexture_normal = loadTexture(pathSunTexture_normal.c_str());
	collision3.makeBumpingObject(BumpShader);
	btRigidBody* CollisionObject3 = Collision::createRigidBody(dynamicsWorld, 2.0f, 99999999999, btVector3(0, 0, 0)); //Creation of the rigid body
	btTransform CollisionObject3Transfo;
	CollisionObject3->getMotionState()->getWorldTransform(CollisionObject3Transfo);
	CollisionObject3->setLinearVelocity(btVector3(0, 0, 0));
	collision3.model = glm::translate(glm::mat4(1.0f), glm::vec3(CollisionObject3Transfo.getOrigin().getX(), CollisionObject3Transfo.getOrigin().getY(), CollisionObject3Transfo.getOrigin().getZ()));


	Object particule(path); //Particles
	std::string pathParticuleTexture = PATH_TO_TEXTURE "/sunv3.jpg";
	unsigned int ParticuleTexture = loadTexture(pathParticuleTexture.c_str());
	particule.makeObject(shaderParticle);
	particule.model = glm::scale(particule.model, glm::vec3(.1,.1,.1));
	particule.model = glm::translate(particule.model, glm::vec3(0.0, 5.0, 0.0));
	Particle particleSystem; //Initialisation of the particleSystem

	//COLLADA Object
	ColladaObject ColladaObject;
	//char pathcollada[] = PATH_TO_OBJECTS "/sphere.dae";
	//ColladaInterface::readColladaFile(pathcollada, &geom_vec, &anim_vec);
	ColladaObject.makeObjectCollada(shader, &geom_vec);

	// Set up the rotation parameters
    glm::vec3 rotationPoint(0.0f, 1.0f, 0.0f);
    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
    float rotationAngle = glm::radians(90.0f);
	float rotationAngleZ = glm::radians(0.0f);
	float rotationAngleMoon = glm::radians(0.0f);

	//Load the cube model and make the model
	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	Object cubeMap(pathCube);
	cubeMap.makeObject(cubeMapShader);
	

	double prev = 0;
	int deltaFrame = 0;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {
			prev = now;
			const double fpsCount = (double)deltaFrame / deltaTime;
			deltaFrame = 0;
			std::cout << "\r FPS: " << fpsCount;
			std::cout.flush();
		}
	};


	glm::vec3 light_pos = glm::vec3(0.0, 0.0, 5.0);
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));

	//glm::mat4 inverseModel = glm::transpose( glm::inverse(model));

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	//Rendering
	RenderingShader(shader);
	RenderingShader(BumpShader);
	
	//Create the cubemap texture
	GLuint cubeMapTexture;
	glGenTextures(1, &cubeMapTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	//This is the image you will use as your skybox
	std::string pathToCubeMapSpaceCyril = PATH_TO_TEXTURE "/cubemaps/space/";

	std::map<std::string, GLenum> facesToLoad = { 
		{pathToCubeMapSpaceCyril + "1.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_X},
		{pathToCubeMapSpaceCyril + "2.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
		{pathToCubeMapSpaceCyril + "3.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
		{pathToCubeMapSpaceCyril + "4.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
		{pathToCubeMapSpaceCyril + "5.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
		{pathToCubeMapSpaceCyril + "6.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
	};

	for (std::pair<std::string, GLenum> pair : facesToLoad) {
		loadCubemapFace(pair.first.c_str(), pair.second);
	}


	//glfwSwapInterval(1);
	int time = 0;
	float angle = PI/2;
	double lastFrame = 0;
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		view = camera.GetViewMatrix();
		perspective = camera.GetProjectionMatrix(camera.Zoom);
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float deltaTime = now - lastFrame;
    	lastFrame = now;
		time += 1;

		// Enable depth testing
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glDepthRange(0.0f, 1.0f);
		
		
		//Set the parameters to allow the earth to rotate around the sun
		float rotationSpeed{glm::radians(45.0f)}; // 30 degrees per second
		rotationAngle += rotationSpeed * deltaTime; // Adjust rotation speed as needed
		glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f); // Arbitrary rotation axis, adjust as needed
		glm::quat rotationQuaternion = glm::angleAxis(rotationAngle, glm::normalize(rotationAxis));
		glm::mat4 modelMatrix = glm::mat4(rotationQuaternion);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, -15.0));
		glm::mat4 inverseModel = glm::transpose( glm::inverse(modelMatrix));

		auto delta = light_pos + glm::vec3(0.0,0.0,2 * std::sin(now));

		//Set parameters for the explosion
		if (explosion){
			explosionTime += deltaTime;
			explosionIntensity += 0.1;
		}
		
		dynamicsWorld->stepSimulation(deltaTime);
		
		shader.use();
		shader.setFloat("explosionIntensity", explosionIntensity);
		shader.setFloat("time", explosionTime);
		shader.setMatrix4("M", modelMatrix);
		shader.setMatrix4("itM", inverseModel);
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);
		shader.setVector3f("light.light_pos", delta);
		shader.setInteger("MyTexture", 1);

		BumpShader.use();
		BumpShader.setMatrix4("M", modelMatrix);
		BumpShader.setMatrix4("itM", inverseModel);
		BumpShader.setMatrix4("V", view);
		BumpShader.setMatrix4("P", perspective);
		BumpShader.setVector3f("u_view_pos", camera.Position);
		BumpShader.setVector3f("light.light_pos", delta);
		BumpShader.setInteger("diffuseMap", 1);
		BumpShader.setInteger("normalMap", 2);
		
		shader.use();
		glDepthFunc(GL_LEQUAL);
		//ColladaObject.drawCollada(&geom_vec);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, EarthTexture);
		sphere1.draw();

		/*CollisionObjectEarth->getMotionState()->getWorldTransform(CollisionObjectEarthTransfo);
		Earth.model = glm::translate(modelMatrix, glm::vec3(CollisionObjectEarthTransfo.getOrigin().getX(), CollisionObjectEarthTransfo.getOrigin().getY(), CollisionObjectEarthTransfo.getOrigin().getZ()));
		glm::mat4 inverseCollisionEarth = glm::transpose(glm::inverse(Earth.model));
		shader.setMatrix4("M", Earth.model);
		shader.setMatrix4("itM", inverseCollisionEarth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, EarthTexture);
		Earth.draw();*/
		
		// send sphere 2 
		glm::mat4 inverseModel2 = glm::transpose( glm::inverse(sphere2.model));
		shader.setMatrix4("M", sphere2.model);
		shader.setMatrix4("itM", inverseModel2);
		glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, SunTexture);
		//sphere2.draw();

		//CollisionObject1->getMotionState()->getWorldTransform(CollisionObject1Transfo); //Meteorite1
		//collision1.model = glm::translate(glm::mat4(1.0f), glm::vec3(CollisionObject1Transfo.getOrigin().getX(), CollisionObject1Transfo.getOrigin().getY(), CollisionObject1Transfo.getOrigin().getZ()));
		//glm::mat4 inverseCollision = glm::transpose( glm::inverse(collision1.model));
		//shader.setMatrix4("M", collision1.model);
		//shader.setMatrix4("itM", inverseCollision);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, MeteoriteTexture);
		//collision1.draw();

		for (int i = 0; i < numberOfMeteorites; i++) {
				Object& meteorite = meteorites[i];
				btRigidBody* meteoriteBody = meteoriteBodies[i];
				glm::mat4 model = glm::mat4(1.0f);

				// Obtenir la transformation actuelle du corps rigide
				btTransform meteoriteTransfo;
				meteoriteBodies[i]->getMotionState()->getWorldTransform(meteoriteTransfo);
				glm::vec3 position = glm::vec3(meteoriteTransfo.getOrigin().getX(), meteoriteTransfo.getOrigin().getY(), meteoriteTransfo.getOrigin().getZ());
				meteoriteBody->getMotionState()->getWorldTransform(meteoriteTransfo);

				model = glm::translate(model, position);
				model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5)); // Mise à l'échelle
				glm::mat4 inverseModel = glm::transpose(glm::inverse(model));

				shader.setMatrix4("M", model);
				shader.setMatrix4("itM", inverseModel);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, MeteoriteTexture);
				meteorite.draw();
			


			CollisionObject2->getMotionState()->getWorldTransform(CollisionObject2Transfo); //Meteorite2
			collision2.model = glm::translate(glm::mat4(1.0f), glm::vec3(CollisionObject2Transfo.getOrigin().getX(), CollisionObject2Transfo.getOrigin().getY(), CollisionObject2Transfo.getOrigin().getZ()));
			collision2.model = glm::scale(collision2.model, glm::vec3(5, 5, 5)); // Mise à l'échelle
			glm::mat4 inverseCollision2 = glm::transpose( glm::inverse(collision2.model));
			shader.setMatrix4("M", collision2.model);
			shader.setMatrix4("itM", inverseCollision2);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, MeteoriteTexture);
			collision2.draw();
		}

		BumpShader.use(); //SUN
		CollisionObject3->getMotionState()->getWorldTransform(CollisionObject3Transfo);
		collision3.model = glm::translate(sun_bumping.model, glm::vec3(CollisionObject3Transfo.getOrigin().getX(), CollisionObject3Transfo.getOrigin().getY(), CollisionObject3Transfo.getOrigin().getZ()));
		glm::mat4 inverseCollision3 = glm::transpose(glm::inverse(collision3.model));
		BumpShader.setMatrix4("M", collision3.model);
		BumpShader.setMatrix4("itM", inverseCollision3);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SunTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, SunTexture_normal);
		collision3.draw();
		

		//Copy of the sun to see the difference with the bump mapping 
		/*BumpShader.use();
		BumpShader.setMatrix4("M", sun_bumping.model);
		BumpShader.setMatrix4("itM", inverseModel2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SunTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, SunTexture_normal);
		sun_bumping.draw();*/


		// moon 
		shader.use();
		float earthOrbitRadius = 5.0;
		glm::vec3 earthPosition = modelMatrix*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		glm::vec3 moonPosition = glm::vec3(
        earthPosition.x + earthOrbitRadius * std::cos(now),
        earthPosition.y,
        earthPosition.z + earthOrbitRadius * std::sin(now)
    	);

		glm::mat4 modelMatrixMoon = glm::translate(glm::mat4(1.0f), moonPosition);
		modelMatrixMoon = glm::scale(modelMatrixMoon, glm::vec3(0.8,0.8,0.8));

		glm::mat4 inverseModelMoon = glm::transpose( glm::inverse(modelMatrixMoon));

		shader.setMatrix4("M", modelMatrixMoon);
		shader.setMatrix4("itM", inverseModelMoon);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, MoonTexture);
		moon.draw();

		////Collision detection between 1 and 3 
		//int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
		//for (int i = 0; i < numManifolds; i++) {
		//	btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		//	const btCollisionObject* obA = contactManifold->getBody0();
		//	const btCollisionObject* obB = contactManifold->getBody1();

		//	if ((obA == CollisionObject1 && obB == CollisionObject3) || (obA == CollisionObject3 && obB == CollisionObject1)) {
		//		explosion = false;
		//		break; 
		//	}
		//}

		// Particles 
		shaderParticle.use();
		shaderParticle.setMatrix4("itM", inverseModel);
		shaderParticle.setMatrix4("V", view);
		shaderParticle.setMatrix4("P", perspective);
		shaderParticle.setFloat("explosionIntensity", explosionIntensity);
		shaderParticle.setFloat("time", explosionTime);

		if (time == 60) {
			angle = particleSystem.randomFloat(0.0f, 2*PI);
			time = 0;
		}
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ParticuleTexture);
		particleSystem.updateParticle(deltaTime);
    	particleSystem.drawParticles(shaderParticle, particule, angle);


		//Send the cubemap 
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		cubeMapShader.setInteger("cubemapTexture", 0);

		cubeMap.draw();
		glDepthFunc(GL_LESS);

		fps(now);
		glfwSwapBuffers(window);
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();
	ColladaInterface::freeGeometries(&geom_vec);
	Collision::freePhysicalWorld(dynamicsWorld);

	return 0;
}





//GENERAL FUNCTIONS
void RenderingShader(Shader shader){
	float ambient = 0.5;
	float diffuse = 0.5;
	float specular = 0.8;
	glm::vec3 materialColour = glm::vec3(0.5f,0.6,0.8);

	shader.use();
	shader.setFloat("shininess", 32.0f);
	shader.setVector3f("materialColour", materialColour);
	shader.setFloat("light.ambient_strength", ambient);
	shader.setFloat("light.diffuse_strength", diffuse);
	shader.setFloat("light.specular_strength", specular);
	shader.setFloat("light.constant", 1.0);
	shader.setFloat("light.linear", 0.14);
	shader.setFloat("light.quadratic", 0.07);
}


//PROCESS INPUT
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(LEFT, 0.1);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(RIGHT, 0.1);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(FORWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(BACKWARD, 0.1);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(-1, 0.0, 1);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, 1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, -1.0, 1);

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
		explosion = true;
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		meteorite = true;
	}
}

//Source from LearnOpenGl: https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/4.normal_mapping/normal_mapping.cpp
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}



//TEXTURE
//Source from LearnOpenGl: https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/4.normal_mapping/normal_mapping.cpp
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void loadCubemapFace(const char * path, const GLenum& targetFace)
{
	int imWidth, imHeight, imNrChannels;
	//Load the image using stbi_load
	unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{
		//Send the image to the the buffer
		glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(targetFace);
		
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
	}
	//Don't forget to free the memory
	stbi_image_free(data);
}














