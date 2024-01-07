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
#include "../headers/shader.h"
#include "../headers/object.h"
#include "../headers/object_bumping.h"
#include "../headers/colladainterface.h"
#include "../headers/colladaobject.h"
#include "../headers/colladaanimation.h"
#include "../headers/collision.h"
#include "../headers/particle.h"

//DEFINE STRUCTURE
struct ParticleNebuleuse {
    glm::vec4 position;
    glm::vec4 color;
};

struct LaunchedSphere {
    Object sphereObject;
    btRigidBody* sphereRigidBody;
	btTransform sphereRigidBodyTransfo;
    float timeAlive;

    LaunchedSphere(Object obj, btRigidBody* body, btTransform bodytransfo) : sphereObject(obj), sphereRigidBody(body), sphereRigidBodyTransfo(bodytransfo), timeAlive(0.0f)  {}
};


//GLOBAL VARIABLES INIT
//---Context---
const int width = 1080;
const int height = 1080; 
float lastX = (float)width / 2.0;
float lastY = (float)height / 2.0;
bool firstMouse = true;
float world_scale = 10.0;
//---Camera---
glm::vec3 init_Pos_camera = glm::vec3(0.0, 0.0, 60.0);
Camera camera(init_Pos_camera);
float distanceFromCamera = 5.0f;
float offsetBelowCamera = -2.0f;
float offsetRight = 0.1f;
float offsetUp = 0.1f;
bool ignoreMouse = false;
//---Game---
glm::vec3 goldPosition;
uint score = 0;
std::vector<LaunchedSphere> launchedSpheres;
int count_launched_ball = 0;
//---Physcial world---
btDynamicsWorld* dynamicsWorld;
btRigidBody* CollisionObject1;
btTransform CollisionObject1Transfo;
btRigidBody* CollisionObject2;
btTransform CollisionObject2Transfo;
std::vector<Object> meteorites;
std::vector<btRigidBody*> meteoriteBodies;
btRigidBody* spaceshipRigidBody;
btTransform SpaceshipObjectTransfo;
float weight = 1.0;
//---Explosion---
bool explosion = false;
float explosionIntensity = 10.0f; 
float explosionTime = 0.0f;
//---Collada---
std::vector<ColGeom> geom_vec;
//---Particles---
GLuint particleBuffer;
//---Meteorites---
bool meteorite_collision = false;
bool setup_meteorite_check = false;
int numberOfMeteorites = 15; 
float rotationSpeedMeteorite{glm::radians(45.0f)}; 
float rotationAngleMeteorite = glm::radians(90.0f);
//---Nebuleuse
int numParticlesNebuleuse = 1000;
GLuint vaoNebuleuse;
//--Shadows---
glm::vec3 lightPos(-5.0f, 0.0f, 50.0f);
glm::vec3 lightPos2(5.0f, 0.0f, 50.0f);
glm::mat4 lightProjection;
glm::mat4 lightView;
glm::mat4 lightView2;
glm::mat4 lightSpaceMatrix;
glm::mat4 lightSpaceMatrix2;
float near_plane = 1; 
float far_plane = 80;
const unsigned int SHADOW_WIDTH = 1024 * world_scale; 
const unsigned int SHADOW_HEIGHT = 1024 * world_scale;
unsigned int depthMapFBO1, depthMap1;
unsigned int depthMapFBO2, depthMap2;
float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
//--Spaceship---
glm::vec3 spaceshipFront= glm::vec3(0, 0, -1); 
glm::vec3 currentSpaceShipPosition;
bool lock_vision = true;
//--Teleportation--
bool earth_pos = false;
//--Cubemap--
GLuint cubeMapTexture;
GLuint cubeMapTextureGO;
//--EarthSpeed--
float speed_offset = 0;



//DEFINE ALL FUNCTIONS
Object createObject(char const * path, Shader shader);
ColladaObject createObjectCollada(char const * path, std::vector<ColGeom>* geom_vec, Shader shader);
void InitMeteorite(char const * meteorite_path, Shader shader, float weight);
void DrawMeteorite(Shader shader, unsigned int texture, float deltaTime);
void setup_meteorite();
void updateMeteorite(btRigidBody* CollisionObject1, Object collision1, btTransform CollisionObject1Transfo, unsigned int texture, Shader shader);
void RenderingShader(Shader shader);
void processInput(GLFWwindow* window, Object* spaceship);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void loadCubemapFace(const char * file, const GLenum& targetCube);
unsigned int loadTexture(const char *path);
void init_shadow(unsigned int& depthMapFBO, unsigned int& depthMap);
void add_shadows(unsigned int depthMapFBO, Shader simpleDepthShader, glm::mat4 modelRocket, Object rocket_shadow, Object sun, Object sun_shadow);
void init_NebuleuseCloud();
void populateBuffer_Nebuleuse();
void drawNebuleuse();
void check_get_gold(Object &gold);
glm::vec3 generateNewGoldPosition(); 
void check_max_GPU();
void setup_spaceship();
void updateSpaceship(btRigidBody* spaceshipRigidBody, Object& spaceship, Shader& shader);
void check_collision_sun_spaceship(btRigidBody* spaceshipRigidBody, btRigidBody* CollisionObject3);
void create_launch_ball(Object launch_ball);
void updateLaunchedSpheres(Object launch_ball, float deltaTime, Shader shader, unsigned int texture);
void load_space_cubemap();
void load_GO_cubemap();
void loadCubeMapTexture(const std::string& basePath, GLuint textureID);



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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0); //A CHANGER EN 3!
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
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

	init_shadow(depthMapFBO1, depthMap1);
	init_shadow(depthMapFBO2, depthMap2);

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
	char coin_path[] = PATH_TO_OBJECTS "/Coin.obj";
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
	Object gold = createObject(coin_path, shader);
	Object particule = createObject(sphere_path, shaderParticle);
	Object rocket = createObject(toyrocket_path, shaderRocket);
	Object rocket_shadow = createObject(toyrocket_path, simpleDepthShader);
	Object spaceship = createObject(toyrocket_path, shader);
	Object cubeMap = createObject(pathCube, cubeMapShader);
	Object launch_ball = createObject(sphere_path, shader); 
	Object collision1 = createObject(sphere_path, shader);
	Object collision2 = createObject(sphere_path, shader);
	ColladaObject sphere = createObjectCollada(pathcollada, &geom_vec, shader);
	InitMeteorite(meteorite_path, shader,weight); 
	setup_spaceship();

	//Sun rigidBody
	BumpingObject sun_bumping(sphere_path);
	sun_bumping.makeBumpingObject(BumpShader);
	btRigidBody* CollisionObject3 = Collision::createRigidBody(dynamicsWorld, 2 * world_scale, 0, btVector3(0, 0, 0)); //Creation of the rigid body
	btTransform CollisionObject3Transfo;
	CollisionObject3->getMotionState()->getWorldTransform(CollisionObject3Transfo);
	CollisionObject3->setLinearVelocity(btVector3(0, 0, 0));
	sun_bumping.model = glm::translate(glm::mat4(1.0f), glm::vec3(CollisionObject3Transfo.getOrigin().getX(), CollisionObject3Transfo.getOrigin().getY(), CollisionObject3Transfo.getOrigin().getZ()));

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
	std::string pathCoinTexture = PATH_TO_TEXTURE "/coin_mario.png";
	std::string pathMilitaireTexture = PATH_TO_TEXTURE "/militaire.jpg";
	std::string pathSpaceshipTexture = PATH_TO_TEXTURE "/spaceship.jpg";
	std::string pathAnimationTexture = PATH_TO_TEXTURE "/animation.png";

	//LOAD TEXTURES
	unsigned int EarthTexture = loadTexture(pathEarthTexture.c_str());
	unsigned int SunTexture = loadTexture(pathSunTexture.c_str());
	unsigned int SunTexture_normal = loadTexture(pathSunTexture_normal.c_str());
	unsigned int MoonTexture = loadTexture(pathMoonTexture.c_str());
	unsigned int MeteoriteTexture = loadTexture(pathMeteoriteTexture.c_str());
	unsigned int FootballTexture = loadTexture(pathFootballTexture.c_str());
	unsigned int GoldTexture = loadTexture(pathGoldTexture.c_str());
	unsigned int CoinTexture = loadTexture(pathCoinTexture.c_str());
	unsigned int MilitaireTexture = loadTexture(pathMilitaireTexture.c_str());
	unsigned int SpaceshipTexture = loadTexture(pathSpaceshipTexture.c_str());
	unsigned int AnimationTexture = colladaAnim.createTexture(pathAnimationTexture.c_str());
	

	//DEFINE MODEL
	//---Collada object (football)---
	sphere.model = glm::translate(sphere.model, glm::vec3(0.0f, 2.5 * world_scale, 2.5 * world_scale));
	sphere.model = glm::scale(sphere.model, glm::vec3(0.5 * world_scale, 0.5 * world_scale, 0.5 * world_scale));
	//---Gold object (for the game)---
	goldPosition = generateNewGoldPosition();
	gold.model = glm::translate(gold.model, goldPosition);
	gold.model = glm::scale(gold.model, glm::vec3(0.25 * world_scale, 0.25 * world_scale, 0.25 * world_scale));
	//---Sun planet---
	sun.model = glm::scale(sun.model, glm::vec3(2 * world_scale, 2 * world_scale, 2 * world_scale));
	sun_bumping.model = glm::scale(sun_bumping.model, glm::vec3(2 * world_scale, 2 * world_scale, 2 * world_scale));
	//---Particle system around the sun---
	Particle particleSystem;
	particule.model = glm::scale(particule.model, glm::vec3(0.1 * world_scale, 0.1 * world_scale, 0.1 * world_scale));
	particule.model = glm::translate(particule.model, glm::vec3(0.0, 5.0 * world_scale, 0.0));
	//---Animation of the cowboy---
	glm::mat4 modelMatrixAnimation(1.0f);
	glm::mat4 identityAnimation(1.0);
	currentPose.resize(boneCount, identityAnimation);
	float angleCollada = glm::radians(-180.0f);
	glm::vec3 axisCollada = glm::vec3(1.0f, 0.0f, 0.0f);
	modelMatrixAnimation = glm::rotate(glm::mat4(1.0f), angleCollada, axisCollada);
	modelMatrixAnimation = glm::translate(modelMatrixAnimation, glm::vec3(0.0f, -2 * world_scale, 0.0f));
	modelMatrixAnimation = glm::scale(modelMatrixAnimation, glm::vec3(0.5 * world_scale, 0.5 * world_scale, 0.5 * world_scale));
	//---Nebuleuse---
	glm::mat4 modelMatrixNebuleuse(1.0f);
	modelMatrixNebuleuse = glm::translate(modelMatrixNebuleuse, glm::vec3(0.0f, 2.5 * world_scale, 2.5 * world_scale));
	modelMatrixNebuleuse = glm::scale(modelMatrixNebuleuse, glm::vec3(0.8 * world_scale, 0.8 * world_scale, 0.8 * world_scale));
	init_NebuleuseCloud();
	populateBuffer_Nebuleuse();
	//---Rocket (reflexive object moving around the sun)---
	float rotAngle = glm::radians(0.0f);
	rocket.model = glm::translate(rocket.model, glm::vec3(0.0, 0.0, 3 * world_scale));
	rocket.model = glm::scale(rocket.model, glm::vec3(0.5 * world_scale, 0.5 * world_scale, 0.5 * world_scale));
	rocket.model = glm::rotate(rocket.model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	rocket.model = glm::rotate(rocket.model, glm::radians(180.0f), glm::vec3(1, 0, 0));	
	glm::mat4 modelRocket;
	modelRocket = glm::scale(rocket.model, glm::vec3(.1,.1,.1));
	glm::vec3 rocketPosition = rocket.model*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);	
	rocket_shadow.model = rocket.model; 
	//---General parameters for planets rotation
    glm::vec3 rotationPoint(0.0f, 1.0f, 0.0f);
    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
    float rotationAngle = glm::radians(90.0f);
	float rotationAngleZ = glm::radians(0.0f);
	float rotationAngleMoon = glm::radians(0.0f);
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(0.5 * world_scale, 0.5 * world_scale, 0.5 * world_scale));
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
	
	//Create the cubemap texture	
	glGenTextures(1, &cubeMapTexture);
	loadCubeMapTexture(PATH_TO_TEXTURE "/cubemaps/space/", cubeMapTexture);

	glGenTextures(1, &cubeMapTextureGO);
	loadCubeMapTexture(PATH_TO_TEXTURE "/cubemaps/GO/", cubeMapTextureGO);
	


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
		lightProjection = glm::ortho(-100.0f , 100.0f , -100.0f , 100.0f , near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
		lightView2 = glm::lookAt(lightPos2, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix2 = lightProjection * lightView2;

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

		//Set parameters for the explosion
		if (explosion){
			explosionTime += deltaTime;
			explosionIntensity += 0.1;
		}

		//Physical world set up
		dynamicsWorld->stepSimulation(deltaTime);

		//If collision between Sun and spaceship
		check_collision_sun_spaceship(spaceshipRigidBody, CollisionObject3);

		//Shadow shader
		simpleDepthShader.use();
        simpleDepthShader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
		add_shadows(depthMapFBO1, simpleDepthShader, modelRocket, rocket_shadow, sun, sun_shadow);
		simpleDepthShader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix2);
		add_shadows(depthMapFBO2, simpleDepthShader, modelRocket, rocket_shadow, sun, sun_shadow);

		//Basic shader
		shader.use();
		shader.setFloat("explosionIntensity", explosionIntensity);
		shader.setFloat("time", explosionTime);
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);
		shader.setVector3f("light.light_pos", lightPos);
		shader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
		shader.setInteger("MyTexture", 1);
		shader.setInteger("shadowMap", 2);

		//Update launched ball
		updateLaunchedSpheres(launch_ball, deltaTime, shader, MilitaireTexture);
		
		//Draw earth
		//Set the parameters to allow the earth to rotate around the sun
		float rotationSpeed{glm::radians(10.0f + speed_offset)}; // 30 degrees per second
		rotationAngle += rotationSpeed * deltaTime;
		glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f); 
		glm::quat rotationQuaternion = glm::angleAxis(rotationAngle, glm::normalize(rotationAxis));
		glm::mat4 modelMatrix = glm::mat4(rotationQuaternion);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, -15 * world_scale));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1 * world_scale, 1 * world_scale, 1 * world_scale));
		glm::mat4 inverseModel = glm::transpose( glm::inverse(modelMatrix));
		shader.setMatrix4("M", modelMatrix);
		shader.setMatrix4("itM", inverseModel);
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
        glBindTexture(GL_TEXTURE_2D, depthMap1);
		//sun.draw();

		//Draw moon 
		float earthOrbitRadius = 3 * world_scale;
		glm::vec3 earthPosition = modelMatrix*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 moonPosition = glm::vec3(
        earthPosition.x + earthOrbitRadius * std::cos(now),
        earthPosition.y,
        earthPosition.z + earthOrbitRadius * std::sin(now)
    	);
		glm::mat4 modelMatrixMoon = glm::translate(glm::mat4(1.0f), moonPosition);
		modelMatrixMoon = glm::scale(modelMatrixMoon, glm::vec3(0.6 * world_scale, 0.6 * world_scale, 0.6 * world_scale));
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
		gold.model = glm::rotate(gold.model, glm::radians(0.5f), glm::vec3(0, goldPosition.y, 0));
		glm::mat4 inverseModelGold = glm::transpose(glm::inverse(gold.model));
		shader.setMatrix4("M", gold.model);
		shader.setMatrix4("itM", inverseModelGold);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GoldTexture);
		//glBindTexture(GL_TEXTURE_2D, CoinTexture); // A DECOMMENTER POUR RAPHAEL!!!!!!!!!!!!!!!!!!
		gold.draw();
		check_get_gold(gold);

		//Spaceship
		updateSpaceship(spaceshipRigidBody, spaceship, shader);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SpaceshipTexture);
		spaceship.draw();

		//Draw bumping sun
		BumpShader.use();
		CollisionObject3->getMotionState()->getWorldTransform(CollisionObject3Transfo);
		sun_bumping.model = glm::translate(sun_bumping.model, glm::vec3(CollisionObject3Transfo.getOrigin().getX(), CollisionObject3Transfo.getOrigin().getY(), CollisionObject3Transfo.getOrigin().getZ()));
		BumpShader.setMatrix4("M", sun_bumping.model);
		BumpShader.setMatrix4("V", view);
		BumpShader.setMatrix4("P", perspective);
		BumpShader.setVector3f("u_view_pos", camera.Position);
		BumpShader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
		BumpShader.setMatrix4("lightSpaceMatrix2", lightSpaceMatrix2);
		BumpShader.setVector3f("light.light_pos", lightPos);
		BumpShader.setInteger("diffuseMap", 1);
		BumpShader.setInteger("normalMap", 2);
		BumpShader.setInteger("shadowMap", 3);	
		BumpShader.setInteger("shadowMap2", 4);		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SunTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, SunTexture_normal);
		glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMap1);
		glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, depthMap2);
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
		rocket.model = glm::rotate(rocket.model, glm::radians(2.0f), glm::vec3(0, 0, 1));
		modelRocket = glm::mat4(rotationQuaternion);
		modelRocket = glm::scale(glm::translate(glm::rotate(modelRocket, glm::radians(90.0f), glm::vec3(0, 0, 1)), glm::vec3(0.0, 0.0, -40.0)), glm::vec3(.5*world_scale,.5*world_scale,.5*world_scale));
		glm::mat4 inverseRocket = glm::transpose( glm::inverse(modelRocket));
		shaderRocket.setMatrix4("M", modelRocket);
		shaderRocket.setMatrix4("itM", inverseRocket);
		shaderRocket.setMatrix4("V", view);
		shaderRocket.setMatrix4("P", perspective);
		shaderRocket.setVector3f("u_view_pos", camera.Position);
		shaderRocket.setFloat("explosionIntensity", explosionIntensity);
		shaderRocket.setFloat("time", explosionTime);
		rocket.draw();

		//Animation
		AnimationShader.use();
		AnimationShader.setMatrix4("M", modelMatrixAnimation);
		AnimationShader.setMatrix4("V", view);
		AnimationShader.setMatrix4("P", perspective);
		colladaAnim.getPose(animation, skeleton, now, currentPose, identityAnimation, globalInverseTransform);
		AnimationShader.setMatrix4_count("bone_transforms", currentPose[0], boneCount);
		AnimationShader.setInteger("diff_texture", 0);
		AnimationShader.setVector3f("light_pos", lightPos);
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
		if (!explosion){
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
			cubeMapShader.setInteger("cubemapSampler", 0);
		} else {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureGO);
			cubeMapShader.setInteger("cubemapSampler", 1);
		}
		cubeMap.draw();
		
		//Change camera view
		if (earth_pos){
			camera.move_earth(glm::vec3(earthPosition.x, earthPosition.y + 3*world_scale, earthPosition.z));
		}

		fps(now);
		glDepthFunc(GL_LESS);
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

void InitMeteorite(char const * meteorite_path, Shader shader, float weight){
	std::random_device rd; 
	std::mt19937 gen(rd()); 
	std::uniform_real_distribution<> dis(-10 * world_scale, 10 * world_scale); // Distribution entre -5.0 et 5.0 pour un cube de 10x10x10

	for (int i = 0; i < numberOfMeteorites; i++) {
		Object meteorite(meteorite_path);
		meteorite.makeObject(shader);

		glm::vec3 startPosition = glm::vec3(dis(gen), dis(gen), dis(gen));

		btRigidBody* meteoriteBody = Collision::createRigidBody(dynamicsWorld, 0.5 * world_scale, weight, btVector3(startPosition.x, startPosition.y, startPosition.z));
		meteoriteBodies.push_back(meteoriteBody);

		meteoriteBody->setLinearVelocity(btVector3(0, 0, 0));
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
        model = glm::scale(model, glm::vec3(0.5 * world_scale, 0.5 * world_scale, 0.5 * world_scale));
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
	CollisionObject2 = Collision::createRigidBody(dynamicsWorld, 1 * world_scale, 1000, btVector3(15,0,0));
	CollisionObject2->setLinearVelocity(btVector3(-2,0,0));
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


//SPACESHIP
void setup_spaceship() {
	glm::vec3 initialSpaceShipPosition = camera.Position + camera.Front * distanceFromCamera;
	initialSpaceShipPosition.y += offsetBelowCamera;

	spaceshipRigidBody = Collision::createRigidBody(dynamicsWorld, 1 * world_scale, 99999, btVector3(initialSpaceShipPosition.x, initialSpaceShipPosition.y, initialSpaceShipPosition.z)); // Masse = 0 pour un objet immobile
	spaceshipRigidBody->setLinearVelocity(btVector3(5, 5, 5));
}


void updateSpaceship(btRigidBody* spaceshipRigidBody, Object& spaceship, Shader& shader) {
    currentSpaceShipPosition = camera.Position + camera.Front * distanceFromCamera;
    currentSpaceShipPosition.y += offsetBelowCamera;

    btTransform spaceshipTransform;
    spaceshipTransform.setIdentity();
    spaceshipTransform.setOrigin(btVector3(currentSpaceShipPosition.x, currentSpaceShipPosition.y, currentSpaceShipPosition.z));
    spaceshipRigidBody->setWorldTransform(spaceshipTransform);
    spaceshipRigidBody->getMotionState()->setWorldTransform(spaceshipTransform);

    spaceship.model = glm::translate(glm::mat4(1.0f), currentSpaceShipPosition);
	if(lock_vision){
		glm::quat rotationQuat = glm::rotation(spaceshipFront, glm::normalize(camera.Front));
		spaceship.model *= glm::mat4_cast(rotationQuat);
	}
	spaceship.model = glm::rotate(spaceship.model, glm::radians(270.0f), glm::vec3(1, 0, 0));

    glm::mat4 inverseSpaceship = glm::transpose(glm::inverse(spaceship.model));
    shader.setMatrix4("M", spaceship.model);
    shader.setMatrix4("itM", inverseSpaceship);
}


void check_collision_sun_spaceship(btRigidBody* spaceshipRigidBody, btRigidBody* CollisionObject3){
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++) {
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* objA = contactManifold->getBody0();
		const btCollisionObject* objB = contactManifold->getBody1();

		if ((objA == CollisionObject3 && objB == spaceshipRigidBody) || (objA == spaceshipRigidBody && objB == CollisionObject3)) {
			explosion = true;
			break;
		}
	}
}




//GENERAL FUNCTIONS
void RenderingShader(Shader shader){
    float ambient = 0.5;		//0.2/0.8/0.5
    float diffuse = 3.0 ;
    float specular = 0.5;
    glm::vec3 materialColour = glm::vec3(0.5, 0.6, 0.8);

    shader.use();
    shader.setFloat("shininess", 32.0);
    shader.setVector3f("materialColour", materialColour);
    shader.setFloat("light.ambient_strength", ambient);
    shader.setFloat("light.diffuse_strength", diffuse);
    shader.setFloat("light.specular_strength", specular);
    shader.setFloat("light.constant", 1.0);		// 1.0/0.14/0.07
    shader.setFloat("light.linear", 0.005);
    shader.setFloat("light.quadratic", 0.0006);
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
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS){
		lock_vision = false;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
		lock_vision = true;
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
		meteorite_collision = true;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS){
		earth_pos = true; 
		ignoreMouse = true;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS){
		earth_pos = false; 
		ignoreMouse = false;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS){
		speed_offset += 1.0; 
	}

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS){
		speed_offset -= 1.0; 
	}

	
	
}

//Source from LearnOpenGl: https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/4.normal_mapping/normal_mapping.cpp
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (ignoreMouse){
		return;
	}
	
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        count_launched_ball += 1;
    }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
        // Code pour le clic gauche relâché
    }
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


//INIT SHADOWS
void init_shadow(unsigned int& depthMapFBO, unsigned int& depthMap){
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

void add_shadows(unsigned int depthMapFBO, Shader simpleDepthShader, glm::mat4 modelRocket, Object rocket_shadow, Object sun, Object sun_shadow){
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	simpleDepthShader.setMatrix4("model", modelRocket);		
	rocket_shadow.draw();
	simpleDepthShader.setMatrix4("model", sun.model);		
	sun_shadow.draw();		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
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
	glPointSize(2.5f);
	glBindVertexArray(vaoNebuleuse);
	glDrawArrays(GL_POINTS, 0, numParticlesNebuleuse);
	glBindVertexArray(0);
}


//SCORE & GOLD FUNCTIONS
glm::vec3 generateNewGoldPosition() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-10 * world_scale, 10 * world_scale);

	float x = dis(gen);
	float y = dis(gen);
	float z = dis(gen);

	return glm::vec3(x, y, z);
}

void check_get_gold(Object& gold) {
	float distanceThreshold = 2.5f * world_scale / 5;
	float distance = glm::distance(currentSpaceShipPosition, goldPosition);

	if (distance < distanceThreshold) {
		score++;
		goldPosition = generateNewGoldPosition();
		gold.model = glm::translate(glm::mat4(1.0f), goldPosition);
	}
}

void create_launch_ball(Object sphere_ball) {
	//Transform
	btTransform sphereRigidBodyTransfo;

	//Position & velocity
    glm::vec3 initialPosition = currentSpaceShipPosition + camera.Front * distanceFromCamera;
    btRigidBody* sphereRigidBody = Collision::createRigidBody(dynamicsWorld, 0.5f, 100.0, btVector3(initialPosition.x, initialPosition.y, initialPosition.z));

    glm::vec3 launchDirection = glm::normalize(camera.Front);
    float launchSpeed = 100.0f; 
	sphereRigidBody->getMotionState()->getWorldTransform(sphereRigidBodyTransfo);
    sphereRigidBody->setLinearVelocity(btVector3(launchDirection.x * launchSpeed, launchDirection.y * launchSpeed, launchDirection.z * launchSpeed));

	launchedSpheres.emplace_back(sphere_ball, sphereRigidBody, sphereRigidBodyTransfo);
}

void updateLaunchedSpheres(Object sphere_ball, float deltaTime, Shader shader, unsigned int texture) {
	if (count_launched_ball > launchedSpheres.size()){
		create_launch_ball(sphere_ball);
	} 

	auto it = launchedSpheres.begin();
    while (it != launchedSpheres.end()) {
        it->timeAlive += deltaTime;
        if (it->timeAlive > 5.0f) {
            it = launchedSpheres.erase(it);
			count_launched_ball -= 1;
        } else {
            it->sphereRigidBody->getMotionState()->getWorldTransform(it->sphereRigidBodyTransfo);
            it->sphereObject.model = glm::translate(glm::mat4(1.0f), glm::vec3(it->sphereRigidBodyTransfo.getOrigin().getX(), it->sphereRigidBodyTransfo.getOrigin().getY(), it->sphereRigidBodyTransfo.getOrigin().getZ()));
			it->sphereObject.model = glm::scale(it->sphereObject.model, glm::vec3(0.5f, 0.5f, 0.5f));
            glm::mat4 inverseBall = glm::transpose(glm::inverse(it->sphereObject.model));
            shader.setMatrix4("M", it->sphereObject.model);
            shader.setMatrix4("itM", inverseBall);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture);
            it->sphereObject.draw();

            ++it;
        }
    } 
}


void loadCubeMapTexture(const std::string& basePath, GLuint textureID) {
	// Bind the cube map texture
	glActiveTexture(GL_TEXTURE0 + textureID - 1); // GL_TEXTURE0 + n pour GL_TEXTUREn
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load the image for each face of the cube map
	std::map<std::string, GLenum> facesToLoad = {
		{basePath + "1.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_X},
		{basePath + "2.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
		{basePath + "3.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
		{basePath + "4.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
		{basePath + "5.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
		{basePath + "6.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
	};

	for (const auto& pair : facesToLoad) {
		loadCubemapFace(pair.first.c_str(), pair.second);
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





















