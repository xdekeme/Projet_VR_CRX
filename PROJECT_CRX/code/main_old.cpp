#include<iostream>
#include <cstdlib> //Rand
#include <ctime> //Rand also

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#define STB_IMAGE_IMPLEMENTATION
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals |  aiProcess_JoinIdenticalVertices )
#include "stb_image.h"

#include <map>

#include "../headers/camera.h"
#include "../headers/spaceship.h"
#include "../headers/shader.h"
#include "../headers/object.h"
#include "../headers/object_bumping.h"
#include "../headers/colladainterface.h"
#include "../headers/colladaobject.h"
#include "../headers/colladaanimation.h"
#include "../headers/collision.h"
#include "../headers/particle.h"


//GLOBAL VARIABLES INIT
//---Context---
const int width = 700;
const int height = 700; 
float lastX = (float)width / 2.0;
float lastY = (float)height / 2.0;
bool firstMouse = true;
//---Camera---
glm::vec3 init_Pos_camera = glm::vec3(13.0, 5.0, 13.0);
Camera camera(init_Pos_camera);
float distanceFromCamera = 5.0f;
float offsetBelowCamera = -2.0f;
//---Game---
glm::vec3 init_Pos_spaceship = glm::vec3(13.0, 5.0, 13.0);
glm::vec3 goldPosition;
uint score = 0;
//---Physcial world---
btDynamicsWorld* dynamicsWorld;
btRigidBody* CollisionObject1;
btTransform CollisionObject1Transfo;
btRigidBody* CollisionObject2;
btTransform CollisionObject2Transfo;
std::vector<Object> meteorites;
std::vector<btRigidBody*> meteoriteBodies;
//---Explosion---
bool explosion = false;
float explosionIntensity = 1.0f; 
float explosionTime = 0.0f;
//---Collada---
std::vector<ColGeom> geom_vec;
//---Particles---
GLuint particleBuffer;
//---Meteorites---
bool meteorite_collision = false;
bool setup_meteorite_check = false;
int numberOfMeteorites = 30; 
float rotationSpeedMeteorite{glm::radians(45.0f)}; 
float rotationAngleMeteorite = glm::radians(90.0f);
//---Nebuleuse
int numParticlesNebuleuse = 320;
GLuint vaoNebuleuse;
//--Shadows---
glm::vec3 lightPos(-0.0f, 0.0f, 15.0f);
glm::mat4 lightProjection;
glm::mat4 lightView;
glm::mat4 lightSpaceMatrix;
float near_plane = 1.0f; 
float far_plane = 27.5f;
const unsigned int SHADOW_WIDTH = 1024; 
const unsigned int SHADOW_HEIGHT = 1024;
unsigned int depthMapFBO;
unsigned int depthMap;
float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };


//DEFINE ALL FUNCTIONS
Object createObject(char const * path, Shader shader);
ColladaObject createObjectCollada(char const * path, std::vector<ColGeom>* geom_vec, Shader shader);
void InitMeteorite(char const * meteorite_path, Shader shader);
void DrawMeteorite(Shader shader, unsigned int texture, float deltaTime);
void setup_meteorite();
void updateMeteorite(btRigidBody* CollisionObject1, Object collision1, btTransform CollisionObject1Transfo, unsigned int texture, Shader shader);
void RenderingShader(Shader shader);
void processInput(GLFWwindow* window, Object* spaceship);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void loadCubemapFace(const char * file, const GLenum& targetCube);
unsigned int loadTexture(const char *path);
void init_shadow();
void init_NebuleuseCloud();
void populateBuffer_Nebuleuse();
glm::mat4 get_ortho_matrix_spaceship();
void drawNebuleuse();
void check_get_gold(const glm::vec3 &cameraPosition, Object &gold);
glm::vec3 generateNewGoldPosition(); 
void check_max_GPU();


//DEFINE STRUCTURE
struct ParticleNebuleuse {
    glm::vec4 position;
    glm::vec4 color;
};


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
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif


	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "PROJECT - CRX", nullptr, nullptr);
	const char* errorMessage;
	if (!window) {
		int errorCode = glfwGetError(&errorMessage);
		if (errorCode != GLFW_NO_ERROR) {
			std::cerr << "GLFW Error (" << errorCode << "): " << errorMessage << std::endl;
		}
	}
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
	//check_max_GPU(); //To check what is the max capacity of the GPU

	init_shadow();

	//Init random
	std::srand(std::time(0));

	//Creation of the physical world
	dynamicsWorld = Collision::createPhysicalWorld();

	//Shader with geometry compilation
	char fileVert[] = PATH_TO_SOURCES "/vertSrc.txt";
	char fileFrag[] = PATH_TO_SOURCES "/fragSrc.txt";
	char fileGeom[] = PATH_TO_SOURCES "/geomSrc.txt"; 
	Shader shader(fileVert, fileFrag, fileGeom);

	//Shader for particles compilation
	char fileFragPart[] = PATH_TO_SOURCES "/fragSrcParticles.txt";
	Shader shaderParticle = Shader(fileVert, fileFragPart, fileGeom);

	//Shader for Rocket compilation
	char fileFragRocket[] = PATH_TO_SOURCES "/fragSrc_Rocket.txt";
	Shader shaderRocket = Shader(fileVert, fileFragRocket, fileGeom);

	//Shader for SpaceShip compilation
	char fileVertSpaceShip[] = PATH_TO_SOURCES "/vertSrc_SpaceShip.txt";
	Shader shaderSpaceShip = Shader(fileVertSpaceShip, fileFrag, fileGeom);

	//Shader with bump mapping compilation
	char fileVertBump[] = PATH_TO_SOURCES "/vertSrc_BumpMapping.txt";
	char fileFragBump[] = PATH_TO_SOURCES "/fragSrc_BumpMapping.txt";
	Shader BumpShader = Shader(fileVertBump, fileFragBump);

	//Shader Animation
	char fileVertAnimation[] = PATH_TO_SOURCES "/vertSrc_Animation.txt";
	char fileFragAnimation[] = PATH_TO_SOURCES "/fragSrc_Animation.txt";
	Shader AnimationShader = Shader(fileVertAnimation, fileFragAnimation);

	//Shader Nebuleuse (compute shader) compilation
	char fileVertNebuleuse[] = PATH_TO_SOURCES "/vertSrc_Nebuleuse.txt";
	char fileFragNebuleuse[] = PATH_TO_SOURCES "/fragSrc_Nebuleuse.txt";
	char ComputeShaderpath[] = PATH_TO_SOURCES "/ComputeShader.txt";
	Shader NebuleuseShader = Shader(fileVertNebuleuse, fileFragNebuleuse);
	//Shader ComputeShaderNebuleuse = Shader(ComputeShaderpath); //A decommenter !!!!!

	//Shader shadows
	char fileVertShadow[] = PATH_TO_SOURCES "/vertSrc_Shadow.txt";
	char fileFragShadow[] = PATH_TO_SOURCES "/fragSrc_Shadow.txt";
	Shader simpleDepthShader = Shader(fileVertShadow, fileFragShadow);

	//Shader Cubemap compilation
	char fileVertCubeMap[] = PATH_TO_SOURCES "/vertSrcCubeMap.txt";
	char fileFragCubeMap[] = PATH_TO_SOURCES "/fragSrcCubeMap.txt";
	Shader cubeMapShader = Shader(fileVertCubeMap, fileFragCubeMap);


	//OBJECT FILE PATH
	char sphere_path[] = PATH_TO_OBJECTS "/sphere_coarse.obj";
	char meteorite_path[] = PATH_TO_OBJECTS "/meteorite.obj";
	char toyrocket_path[] = PATH_TO_OBJECTS "/Toy_Rocket.obj";
	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	char pathcollada_model[] = PATH_TO_OBJECTS "/model2.dae"; //Cowboy running
	char pathcollada[] = PATH_TO_OBJECTS "/sphere_raph.dae";

	//CREATION OBJECT
	Object earth = createObject(sphere_path, shader);
	Object sun = createObject(sphere_path, shader); 
	Object sun_shadow = createObject(sphere_path, simpleDepthShader);
	Object moon = createObject(sphere_path, shader);
	Object gold = createObject(sphere_path, shader);
	Object particule = createObject(sphere_path, shaderParticle);
	Object rocket = createObject(toyrocket_path, shaderRocket);
	Object rocket_shadow = createObject(toyrocket_path, simpleDepthShader);
	Object spaceship = createObject(toyrocket_path, shader);
	Object cubeMap = createObject(pathCube, cubeMapShader);
	Object collision1 = createObject(sphere_path, shader);
	Object collision2 = createObject(sphere_path, shader);
	ColladaObject sphere = createObjectCollada(pathcollada, &geom_vec, shader);
	InitMeteorite(meteorite_path, shader); 
	BumpingObject sun_bumping(sphere_path);
	sun_bumping.makeBumpingObject(BumpShader);

	//SET UP ANIMATION
	std::vector<ColladaAnimation::Vertex> vertices = {}; 
	std::vector<uint> indices = {};   
	std::vector<glm::mat4> currentPose; 
	uint boneCount;
	ColladaAnimation::Animation animation;
	ColladaAnimation::Bone skeleton; 
	uint vao;
	ColladaAnimation colladaAnim;
	glm::mat4 globalInverseTransform = colladaAnim.init_Animation(pathcollada_model, vertices, indices, boneCount, animation, skeleton, vao);

	//TEXTURE FILE PATH
	std::string pathEarthTexture = PATH_TO_TEXTURE "/earth.jpeg";
	std::string pathSunTexture = PATH_TO_TEXTURE "/sunv3.jpg";
	std::string pathSunTexture_normal = PATH_TO_TEXTURE "/sun_normal.jpg";
	std::string pathMoonTexture = PATH_TO_TEXTURE "/moonv3.jpeg";
	std::string pathMeteoriteTexture = PATH_TO_TEXTURE "/meteorite.jpeg";
	std::string pathFootballTexture = PATH_TO_TEXTURE "/football.jpg";
	std::string pathGoldTexture = PATH_TO_TEXTURE "/gold.jpg";
	std::string pathAnimationTexture = PATH_TO_TEXTURE "/animation.png";

	//LOAD TEXTURES
	unsigned int EarthTexture = loadTexture(pathEarthTexture.c_str());
	unsigned int SunTexture = loadTexture(pathSunTexture.c_str());
	unsigned int SunTexture_normal = loadTexture(pathSunTexture_normal.c_str());
	unsigned int MoonTexture = loadTexture(pathMoonTexture.c_str());
	unsigned int MeteoriteTexture = loadTexture(pathMeteoriteTexture.c_str());
	unsigned int FootballTexture = loadTexture(pathFootballTexture.c_str());
	unsigned int GoldTexture = loadTexture(pathGoldTexture.c_str());
	unsigned int AnimationTexture = colladaAnim.createTexture(pathAnimationTexture.c_str());
	

	//DEFINE MODEL
	//---Collada object (football)---
	sphere.model = glm::translate(sphere.model, glm::vec3(0.0f, 2.5f, 2.5f));
	sphere.model = glm::scale(sphere.model, glm::vec3(0.5f, 0.5f, 0.5f));
	//---Gold object (for the game)---
	goldPosition = generateNewGoldPosition();
	gold.model = glm::translate(gold.model, goldPosition);
	gold.model = glm::scale(gold.model, glm::vec3(0.25f, 0.25f, 0.25f));
	//---Sun planet---
	sun.model = glm::scale(sun.model, glm::vec3(2.0,2.0,2.0));
	sun_bumping.model = glm::scale(sun_bumping.model, glm::vec3(2.0,2.0,2.0));
	//---Particle system around the sun---
	Particle particleSystem;
	particule.model = glm::scale(particule.model, glm::vec3(.1,.1,.1));
	particule.model = glm::translate(particule.model, glm::vec3(0.0, 5.0, 0.0));
	//---Animation of the cowboy---
	glm::mat4 modelMatrixAnimation(1.0f);
	glm::mat4 identityAnimation(1.0);
	currentPose.resize(boneCount, identityAnimation);
	float angleCollada = glm::radians(-180.0f);
	glm::vec3 axisCollada = glm::vec3(1.0f, 0.0f, 0.0f);
	modelMatrixAnimation = glm::rotate(glm::mat4(1.0f), angleCollada, axisCollada);
	modelMatrixAnimation = glm::translate(modelMatrixAnimation, glm::vec3(0.0f, -2.0f, 0.0f));
	modelMatrixAnimation = glm::scale(modelMatrixAnimation, glm::vec3(0.5f, 0.5f, 0.5f));
	//---Nebuleuse---
	glm::mat4 modelMatrixNebuleuse(1.0f);
	modelMatrixNebuleuse = glm::translate(modelMatrixNebuleuse, glm::vec3(10.0f, -5.0f, 5.0f));
	modelMatrixNebuleuse = glm::scale(modelMatrixNebuleuse, glm::vec3(2.0f, 2.0f, 2.0f));
	init_NebuleuseCloud();
	populateBuffer_Nebuleuse();
	//---Rocket (reflexive object moving around the sun)---
	float rotAngle = glm::radians(0.0f);
	rocket.model = glm::translate(rocket.model, glm::vec3(0.0, 0.0, 3.0));
	rocket.model = glm::scale(rocket.model, glm::vec3(.5,.5,.5));
	rocket.model = glm::rotate(rocket.model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	rocket.model = glm::rotate(rocket.model, glm::radians(180.0f), glm::vec3(1, 0, 0));	
	//---Rocket Shadow---
	rocket_shadow.model = rocket.model; 
	//---Ortho matrice to block the spaceship on the camera---
	glm::mat4 projection = get_ortho_matrix_spaceship();
	//---General parameters for planets rotation
    glm::vec3 rotationPoint(0.0f, 1.0f, 0.0f);
    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
    float rotationAngle = glm::radians(90.0f);
	float rotationAngleZ = glm::radians(0.0f);
	float rotationAngleMoon = glm::radians(0.0f);
	glm::vec3 light_pos = glm::vec3(0.0, 0.0, 5.0);
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	
	//FPS + Score print function
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

			std::cout << "\033[A\033[2K"; 
			std::cout << "\033[A\033[2K"; 

			std::cout << "FPS: " << std::fixed << std::setprecision(2) << fpsCount << std::endl;
			std::cout << "Score: " << score << std::endl;
		}
	};


	//Rendering
	RenderingShader(shader);
	RenderingShader(BumpShader);
	RenderingShader(shaderSpaceShip);
	
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
		//General functions to call
		processInput(window, &spaceship);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();
		double now = glfwGetTime();

		//Light parameters
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

		// Enable depth testing
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glDepthRange(0.0f, 1.0f);

		//Camera parameters
		view = camera.GetViewMatrix();
		perspective = camera.GetProjectionMatrix(camera.Zoom);
		
		//Time parameters
		float deltaTime = now - lastFrame;
    	lastFrame = now;
		time += 1;
		auto delta = light_pos + glm::vec3(0.0,0.0,2 * std::sin(now));

		//Set parameters for the explosion
		if (explosion){
			explosionTime += deltaTime;
			explosionIntensity += 0.1;
		}

		//Physical world set up
		dynamicsWorld->stepSimulation(deltaTime);

		//Set the parameters to allow the earth to rotate around the sun
		float rotationSpeed{glm::radians(45.0f)}; // 30 degrees per second
		rotationAngle += rotationSpeed * deltaTime;
		glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f); 
		glm::quat rotationQuaternion = glm::angleAxis(rotationAngle, glm::normalize(rotationAxis));
		glm::mat4 modelMatrix = glm::mat4(rotationQuaternion);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, -15.0));
		glm::mat4 inverseModel = glm::transpose( glm::inverse(modelMatrix));

		//Shadow shader
		simpleDepthShader.use();
        simpleDepthShader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		simpleDepthShader.setMatrix4("model", sun.model);
		sun_shadow.draw();
		simpleDepthShader.setMatrix4("model", rocket.model);		
		rocket_shadow.draw();	
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//Basic shader
		shader.use();
		shader.setFloat("explosionIntensity", explosionIntensity);
		shader.setFloat("time", explosionTime);
		shader.setMatrix4("M", modelMatrix);
		shader.setMatrix4("itM", inverseModel);
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);
		shader.setVector3f("light.light_pos", delta);
		shader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
		shader.setInteger("MyTexture", 1);
		shader.setInteger("shadowMap", 2);
		
		//Draw earth
		glDepthFunc(GL_LEQUAL);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, EarthTexture);
		earth.draw();

		//Draw normal sun 
		glm::mat4 inverseModel2 = glm::transpose(glm::inverse(sun.model));
		shader.setMatrix4("M", sun.model);
		shader.setMatrix4("itM", inverseModel2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SunTexture);
		glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
		//sun.draw();

		//Draw moon 
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

		//Set meteorite
		if (meteorite_collision){
			if(!setup_meteorite_check){
				setup_meteorite();
				setup_meteorite_check = true;
			}
			updateMeteorite(CollisionObject1, collision1, CollisionObject1Transfo, MeteoriteTexture, shader);
			updateMeteorite(CollisionObject2, collision2, CollisionObject2Transfo, MeteoriteTexture, shader);
		}
		DrawMeteorite(shader, MeteoriteTexture, deltaTime);

		//Draw collada object (Sphere)
		glm::mat4 inverseModelCollada = glm::transpose(glm::inverse(sphere.model));
		shader.setMatrix4("M", sphere.model);
		shader.setMatrix4("itM", inverseModelCollada);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FootballTexture);
		sphere.drawCollada(&geom_vec);
		
		//Draw gold
		glm::mat4 inverseModelGold = glm::transpose(glm::inverse(gold.model));
		shader.setMatrix4("M", gold.model);
		shader.setMatrix4("itM", inverseModelGold);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GoldTexture);
		gold.draw();
		check_get_gold(camera.Position, gold);

		//Draw bumping sun
		BumpShader.use();
		BumpShader.setMatrix4("M", sun_bumping.model);
		BumpShader.setMatrix4("V", view);
		BumpShader.setMatrix4("P", perspective);
		BumpShader.setVector3f("u_view_pos", camera.Position);
		BumpShader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
		BumpShader.setVector3f("light.light_pos", delta);
		BumpShader.setInteger("diffuseMap", 1);
		BumpShader.setInteger("normalMap", 2);
		BumpShader.setInteger("shadowMap", 3);		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SunTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, SunTexture_normal);
		glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMap);
		sun_bumping.draw();

		//Particles 
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
		particleSystem.updateParticle(deltaTime);
    	particleSystem.drawParticles(shaderParticle, particule, angle);

		//Rocket 
		shaderRocket.use();
		glm::mat4 inverseRocket = glm::transpose( glm::inverse(rocket.model));
		rocket.model = glm::rotate(rocket.model, glm::radians(2.0f), glm::vec3(0, 0, 1));
		shaderRocket.setMatrix4("M", rocket.model);
		shaderRocket.setMatrix4("itM", inverseRocket);
		shaderRocket.setMatrix4("V", view);
		shaderRocket.setMatrix4("P", perspective);
		shaderRocket.setVector3f("u_view_pos", camera.Position);
		shaderRocket.setFloat("explosionIntensity", explosionIntensity);
		shaderRocket.setFloat("time", explosionTime);
		rocket.draw();

		//Spaceship
		shaderSpaceShip.use();
		glm::vec3 SpaceShipPosition = camera.Position + camera.Front * distanceFromCamera;
		SpaceShipPosition.y += offsetBelowCamera;
		spaceship.model = glm::translate(glm::mat4(1.0f), SpaceShipPosition);
		spaceship.model = glm::scale(spaceship.model, glm::vec3(.75,.75,.75));
		spaceship.model = glm::rotate(spaceship.model, glm::radians(270.0f), glm::vec3(1, 0, 0));
		glm::mat4 inverseModelSpaceShip = glm::transpose(glm::inverse(spaceship.model));
		shaderSpaceShip.setMatrix4("M", spaceship.model);
		shaderSpaceShip.setMatrix4("itM", inverseModelSpaceShip);
		shaderSpaceShip.setMatrix4("V", view);
		shaderSpaceShip.setMatrix4("projection", projection);
		shaderSpaceShip.setVector3f("u_view_pos", camera.Position);
		shaderSpaceShip.setVector3f("light.light_pos", delta);
		shaderSpaceShip.setFloat("explosionIntensity", explosionIntensity);
		shaderSpaceShip.setFloat("time", explosionTime);
		shaderSpaceShip.setInteger("MyTexture", 1);
		glDepthFunc(GL_LEQUAL);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, MoonTexture);
		spaceship.draw();

		//Animation
		AnimationShader.use();
		AnimationShader.setMatrix4("M", modelMatrixAnimation);
		AnimationShader.setMatrix4("V", view);
		AnimationShader.setMatrix4("P", perspective);
		colladaAnim.getPose(animation, skeleton, now, currentPose, identityAnimation, globalInverseTransform);
		AnimationShader.setMatrix4_count("bone_transforms", currentPose[0], boneCount);
		AnimationShader.setInteger("diff_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, AnimationTexture);
		colladaAnim.drawAnimation(vao, indices);

		// //Nebuleuse
		// ComputeShaderNebuleuse.use(); 
		// ComputeShaderNebuleuse.setFloat("time", now);
		// ComputeShaderNebuleuse.setInteger("numParticles", numParticlesNebuleuse);
		// glDispatchCompute(32, 1, 1);
		// glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		// NebuleuseShader.use();
		// NebuleuseShader.setMatrix4("M", modelMatrixNebuleuse);
		// NebuleuseShader.setMatrix4("V", view);
		// NebuleuseShader.setMatrix4("P", perspective);
		// drawNebuleuse();

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



//CREATE OBJECTS
Object createObject(char const * path, Shader shader){
	Object object(path);
	object.makeObject(shader);
	return object;
}

ColladaObject createObjectCollada(char const * path, std::vector<ColGeom>* geom_vec, Shader shader){
	ColladaObject colladaobject; //COLLADA Object
	ColladaInterface::readColladaFile(path, geom_vec);
	colladaobject.makeObjectCollada(shader, geom_vec);
	return colladaobject;
}

void InitMeteorite(char const * meteorite_path, Shader shader){
	for (int i = 0; i < numberOfMeteorites; i++) {
		Object meteorite(meteorite_path); 
		meteorite.makeObject(shader);

		float angle = i * (2.0f * 3.1416 / numberOfMeteorites);
		glm::vec3 startPosition = glm::vec3(5 * cos(angle), 0, 5 * sin(angle));

		btRigidBody* meteoriteBody = Collision::createRigidBody(dynamicsWorld, 0.5f, 1, btVector3(startPosition.x, startPosition.y, startPosition.z));
		meteoriteBodies.push_back(meteoriteBody);

		meteorites.push_back(meteorite);
	}
}


void DrawMeteorite(Shader shader, unsigned int texture, float deltaTime){

    for (int i = 0; i < numberOfMeteorites; i++) {
        Object& meteorite = meteorites[i];
        btRigidBody* meteoriteBody = meteoriteBodies[i];
        glm::mat4 model = glm::mat4(1.0f);

        btTransform meteoriteTransfo;
        meteoriteBody->getMotionState()->getWorldTransform(meteoriteTransfo);
        glm::vec3 position = glm::vec3(meteoriteTransfo.getOrigin().getX(), meteoriteTransfo.getOrigin().getY(), meteoriteTransfo.getOrigin().getZ());
        
        model = glm::translate(model, position); 
        model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
        glm::mat4 inverseModel = glm::transpose(glm::inverse(model));

        shader.setMatrix4("M", model);
        shader.setMatrix4("itM", inverseModel);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture);
        meteorite.draw();
    }
}



//SETUP METEORITE
void setup_meteorite(){
	CollisionObject1 = Collision::createRigidBody(dynamicsWorld, 1.0f, 1000, btVector3(5,0,0));
	CollisionObject1->setLinearVelocity(btVector3(1,0,0));
	CollisionObject2 = Collision::createRigidBody(dynamicsWorld, 1.0f, 1000, btVector3(15,0,0));
	CollisionObject2->setLinearVelocity(btVector3(-1,0,0));
}

//UPDATE METEORITE
void updateMeteorite(btRigidBody* CollisionObj, Object coll, btTransform CollisionObjTrans, unsigned int texture, Shader shader){
	CollisionObj->getMotionState()->getWorldTransform(CollisionObjTrans);
	coll.model = glm::translate(glm::mat4(1.0f), glm::vec3(CollisionObjTrans.getOrigin().getX(), CollisionObjTrans.getOrigin().getY(), CollisionObjTrans.getOrigin().getZ()));
	glm::mat4 inverseCollision = glm::transpose( glm::inverse(coll.model));
	shader.setMatrix4("M", coll.model);
	shader.setMatrix4("itM", inverseCollision);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture);
	coll.draw();
}


//GENERAL FUNCTIONS
void RenderingShader(Shader shader){
    float ambient = 0.2;
    float diffuse = 0.8;
    float specular = 0.5;
    glm::vec3 materialColour = glm::vec3(0.5f, 0.6f, 0.8f);

    shader.use();
    shader.setFloat("shininess", 32.0f);
    shader.setVector3f("materialColour", materialColour);
    shader.setFloat("light.ambient_strength", ambient);
    shader.setFloat("light.diffuse_strength", diffuse);
    shader.setFloat("light.specular_strength", specular);
    shader.setFloat("light.constant", 1.0f);
    shader.setFloat("light.linear", 0.09f);
    shader.setFloat("light.quadratic", 0.032f);
}



//PROCESS INPUT KEYBOARD
void processInput(GLFWwindow* window, Object* spaceship) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		camera.ProcessKeyboardMovement(LEFT, 0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		camera.ProcessKeyboardMovement(RIGHT, 0.1);
	}
		
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		camera.ProcessKeyboardMovement(FORWARD, 0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		camera.ProcessKeyboardMovement(BACKWARD, 0.1);
	}
		

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
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
		meteorite_collision = true;
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
	//spaceship_class.ProcessMouseMovement(xoffset, yoffset);
	//spaceship->model = glm::translate(glm::mat4(1.0f), spaceship_class.Position);
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


//ORTHO MATRIX
glm::mat4 get_ortho_matrix_spaceship(){
	float aspectRatio = static_cast<float>(width) / height;
	float viewHeight = 10.0f;
	float viewWidth = viewHeight * aspectRatio;
	float left = -viewWidth / 2.0f;
	float right = viewWidth / 2.0f;
	float bottom = -viewHeight / 2.0f;
	float top = viewHeight / 2.0f;
	float near = 0.1f;
	float far = 100.0f;
	return glm::ortho(left, right, bottom, top, near, far);
}

//INIT SHADOWS
void init_shadow(){
	glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//NEBULEUSE FUNCTIONS TO DRAW
void init_NebuleuseCloud() {
	std::vector<ParticleNebuleuse> particles(numParticlesNebuleuse);

	glGenBuffers(1, &particleBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleNebuleuse) * numParticlesNebuleuse, particles.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);
}

void populateBuffer_Nebuleuse() {
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleNebuleuse), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleNebuleuse), (void*)offsetof(ParticleNebuleuse, color));
	glBindVertexArray(0);

	vaoNebuleuse = vao; 
}

void drawNebuleuse() {
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	glPointSize(5.0f);
	glBindVertexArray(vaoNebuleuse);
	glDrawArrays(GL_POINTS, 0, numParticlesNebuleuse);
	glBindVertexArray(0);
}


//SCORE & GOLD FUNCTIONS
glm::vec3 generateNewGoldPosition() {
    float x = -10 + std::rand() % 21; //Number between -10 and 10
    float y = -10 + std::rand() % 21;
    float z = -10 + std::rand() % 21;

    return glm::vec3(x, y, z);
}

void check_get_gold(const glm::vec3 &cameraPosition, Object &gold) {
    float distanceThreshold = 5.0f; 
    float distance = glm::distance(cameraPosition, goldPosition);

    if (distance < distanceThreshold) {
        score++;
        goldPosition = generateNewGoldPosition();
		gold.model = glm::translate(glm::mat4(1.0f), goldPosition);
    }
}


//CHECK MAX CAPACITIES OF THE GPU
void check_max_GPU(){
	GLint maxWorkGroupCount[3];
    GLint maxWorkGroupSize[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupCount[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWorkGroupCount[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWorkGroupCount[2]);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxWorkGroupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxWorkGroupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxWorkGroupSize[2]);

    std::cout << "Max Compute Work Group Count: "
              << maxWorkGroupCount[0] << ", " 
              << maxWorkGroupCount[1] << ", " 
              << maxWorkGroupCount[2] << std::endl;

    std::cout << "Max Compute Work Group Size: "
              << maxWorkGroupSize[0] << ", " 
              << maxWorkGroupSize[1] << ", " 
              << maxWorkGroupSize[2] << std::endl;
}























