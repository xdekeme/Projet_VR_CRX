#include<iostream>

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <map>

#include "../../camera.h"
#include "../../shader.h"
#include "../object.h"

#include <random>

#define PI 3.14159265

const int width = 700;
const int height = 700;


GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);

void loadCubemapFace(const char * file, const GLenum& targetCube);


void renderCube();
void renderQuad();


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

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif

// Camera camera(glm::vec3(13.0, 5.0, 13.0));
Camera camera(glm::vec3(0.0, 0.0, 7.0));







const int numParticles = 50;
const float particleRadius = 0.08f;
const float maxParticleSpeed = 0.1f;
const float maxParticleLifespan = 5.0f;

struct Particle {
    float x, y, z;       // Position
    float vx, vy, vz;     // Velocity
    float lifespan;   // Remaining lifespan

	
};

float randomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}



unsigned int planeVAO;


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}





int main(int argc, char* argv[])
{
	std::cout << "Welcome to exercice 9: " << std::endl;
	std::cout << "Implement relection on an object\n"
		"\n";


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
	GLFWwindow* window = glfwCreateWindow(width, height, "Exercise 09", nullptr, nullptr);
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

// #ifndef NDEBUG
// 	int flags;
// 	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
// 	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
// 	{
// 		glEnable(GL_DEBUG_OUTPUT);
// 		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
// 		glDebugMessageCallback(glDebugOutput, nullptr);
// 		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
// 	}
// #endif

	const std::string sourceV = "#version 330 core\n"
		"in vec3 position; \n"
		"in vec2 tex_coords; \n"
		"in vec3 normal; \n"

		"out vec3 v_frag_coord; \n"
		"out vec3 v_normal; \n"

		"uniform mat4 M; \n"
		"uniform mat4 itM; \n"
		"uniform mat4 V; \n"
		"uniform mat4 P; \n"


		" void main(){ \n"
		"vec4 frag_coord = M*vec4(position, 1.0); \n"
		"gl_Position = P*V*frag_coord; \n"
		"v_normal = vec3(itM * vec4(normal, 1.0)); \n"
		"v_frag_coord = frag_coord.xyz; \n"
		"\n" 
		"}\n";
	//reflectaion will be coded in the fragment shader
	const std::string sourceF = "#version 400 core\n"
		"out vec4 FragColor;\n"
		"precision mediump float; \n"

		"in vec3 v_frag_coord; \n"
		"in vec3 v_normal; \n"
		"uniform vec3 u_view_pos; \n"
		//Declare a uniform to have access to the cubemap texture
		
		"uniform samplerCube cubemapSampler; \n"
		

		"struct Light{\n" 
		"vec3 light_pos; \n"
		"float ambient_strength; \n"
		"float diffuse_strength; \n"
		"float specular_strength; \n"
		//attenuation factor
		"float constant;\n"
		"float linear;\n"
		"float quadratic;\n"

		"vec3 direction;\n"

		"};\n"
		"uniform Light light;"

		"uniform float shininess; \n"
		"uniform vec3 materialColour; \n"

		"float specularCalculation(vec3 N, vec3 L, vec3 V ){ \n"
			"vec3 R = reflect (-L,N);  \n " //reflect (-L,N) is  equivalent to //max (2 * dot(N,L) * N - L , 0.0) ;
			"float cosTheta = dot(R , V); \n"
			"float spec = pow(max(cosTheta,0.0), 32.0); \n"
			"return light.specular_strength * spec;\n"
		"}\n"

		"void main() { \n"
		//What do you need to compute the reflection vector R ?
		"vec3 N = normalize(v_normal);\n"
		"vec3 V = normalize(u_view_pos - v_frag_coord); \n"
		//Find the position you need to sample on the cubemap, tips : glsl has a reflect method
		"vec3 R = reflect(-V,N); \n"
		//use tue cubemp texture to color the fragment
		//"FragColor = texture(cubemapSampler,R); \n"
		
		"vec3 L = normalize(light.light_pos - v_frag_coord) ; \n"
		//"vec3 V = normalize(u_view_pos - v_frag_coord); \n"
		"float specular = specularCalculation( N, L, V); \n"
		"float diffuse = light.diffuse_strength * max(dot(N,L),0.0);\n"
		"float distance = length(light.light_pos - v_frag_coord);"
		"float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);"
		"float light = light.ambient_strength + attenuation * (diffuse + specular); \n"

		// "vec3 lightDir = normalize(-light.direction);  \n"
		// "float diffuse2 = light.diffuse_strength * max(dot(N,lightDir),0.0);\n"
		// "float light2 = light.ambient_strength + attenuation * (diffuse); \n"

		"FragColor = vec4(materialColour * vec3(light), 1.0); \n"



		"} \n";

	Shader shader(sourceV, sourceF);

	const std::string sourceVCubeMap = "#version 330 core\n"
		"in vec3 position; \n"
		"in vec2 tex_coords; \n"
		"in vec3 normal; \n"
		
		//only P and V are necessary
		"uniform mat4 V; \n"
		"uniform mat4 P; \n"

		"out vec3 texCoord_v; \n"

		" void main(){ \n"
		"texCoord_v = position;\n"
		//remove translation info from view matrix to only keep rotation
		"mat4 V_no_rot = mat4(mat3(V)) ;\n"
		"vec4 pos = P * V_no_rot * vec4(position, 1.0); \n"
		// the positions xyz are divided by w after the vertex shader
		// the z component is equal to the depth value
		// we want a z always equal to 1.0 here, so we set z = w!
		// Remember: z=1.0 is the MAXIMUM depth value ;)
		"gl_Position = pos.xyww;\n"
		"\n" 
		"}\n";

	const std::string sourceFCubeMap = 
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"precision mediump float; \n"
		"uniform samplerCube cubemapSampler; \n"
		"in vec3 texCoord_v; \n"
		"void main() { \n"
		"FragColor = texture(cubemapSampler,texCoord_v); \n"
		"} \n";


	Shader cubeMapShader = Shader(sourceVCubeMap, sourceFCubeMap);







	char path[] = PATH_TO_OBJECTS "/sphere_coarse.obj";

	Object sphere1(path);
	sphere1.makeObject(shader);
	
    // Set up the rotation parameters
    glm::vec3 rotationPoint(0.0f, 1.0f, 0.0f);
    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
    float rotationAngle = glm::radians(90.0f);
	float rotationAngleZ = glm::radians(0.0f);
	float rotationAngleMoon = glm::radians(0.0f);

    // Create a quaternion for the rotation
    //glm::quat rotationQuaternion = glm::angleAxis(rotationAngle, rotationAxis);

    // Apply rotation to the model matrix
    //sphere1.model = glm::rotate(sphere1.model, rotationAngle, rotationAxis);

    // Apply translation after rotation
    //sphere1.model = glm::translate(sphere1.model, -rotationPoint);
    //sphere1.model = glm::translate(sphere1.model, glm::vec3(0.0, 0.0, -10.0));
    //sphere1.model = glm::translate(sphere1.model, rotationPoint);



	Object sphere2(path);
	sphere2.makeObject(shader);
	sphere2.model = glm::scale(sphere2.model, glm::vec3(2.0,2.0,2.0));

	Object moon(path);
	moon.makeObject(shader);





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


	glm::vec3 light_pos = glm::vec3(0.0, 0.0, 20.0);
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));

	//glm::mat4 inverseModel = glm::transpose( glm::inverse(model));

	
	


	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	float ambient = 0.05;			//0.5/0.8/0.8
	float diffuse = 14.0;
	float specular = 0.0;

	glm::vec3 materialColour = glm::vec3(0.5f,0.6,0.8);

	//Rendering

	shader.use();
	shader.setFloat("shininess", 32.0f);
	shader.setVector3f("materialColour", materialColour);
	shader.setFloat("light.ambient_strength", ambient);
	shader.setFloat("light.diffuse_strength", diffuse);
	shader.setFloat("light.specular_strength", specular);
	shader.setFloat("light.constant", 1.0);
	shader.setFloat("light.linear", 0.14);
	shader.setFloat("light.quadratic", 0.07);


	shader.setVector3f("light.direction", 0.0f, 0.0f, -1.0f); 	


	

	GLuint cubeMapTexture;
	glGenTextures(1, &cubeMapTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//stbi_set_flip_vertically_on_load(true);

	std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/space/";

	std::map<std::string, GLenum> facesToLoad = { 
		{pathToCubeMap + "posx.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_X},
		{pathToCubeMap + "posy.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
		{pathToCubeMap + "posz.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
		{pathToCubeMap + "negx.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
		{pathToCubeMap + "negy.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
		{pathToCubeMap + "negz.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
	};
	//load the six faces
	for (std::pair<std::string, GLenum> pair : facesToLoad) {
		loadCubemapFace(pair.first.c_str(), pair.second);
	}



const std::string sourceFParticle = "#version 400 core\n"
		"out vec4 FragColor;\n"
		"precision mediump float; \n"

		"in vec3 v_frag_coord; \n"
		"in vec3 v_normal; \n"
		"uniform vec3 u_view_pos; \n"
		//Declare a uniform to have access to the cubemap texture
		
		"uniform samplerCube cubemapSampler; \n"
		

		"uniform vec3 materialColour; \n"


		"void main() { \n"

		"float distance = sqrt(v_frag_coord.x*v_frag_coord.x + v_frag_coord.y*v_frag_coord.y); \n"
		"vec3 yellowColor = vec3(1.0, 1.0, 0.0); \n"
		"vec3 redColor = vec3(1.0, 0.0, 0.0); \n"
		"vec3 color = mix(yellowColor, redColor, smoothstep(1.9, 2.15, distance)); \n"
	
    	"FragColor = vec4(color, 1.0); \n"


		"} \n";

	Shader shaderParticle = Shader(sourceV, sourceFParticle);







	char pathpart[] = PATH_TO_OBJECTS "/sphere_extremely_coarse.obj";
	Object particule(pathpart);
	particule.makeObject(shaderParticle);
	particule.model = glm::scale(particule.model, glm::vec3(.1,.1,.1));
	particule.model = glm::translate(particule.model, glm::vec3(0.0, 5.0, 0.0));



	const int numParticles = 151;  // Adjust the number of particles as needed
	Particle particles[numParticles];

    // Initialize particles with random values
    for (int i = 0; i < numParticles; ++i) {
			particles[i].x = randomFloat(0.0f, .0010f);
			particles[i].y = randomFloat(-0.0f, 0.5f);
			particles[i].vx = randomFloat(-0.005f, 0.005f);
			particles[i].vy = randomFloat(.5f, .7f);

			particles[i].lifespan = 80.0f;  // Reset lifespan

			particles[i].z = randomFloat(0.0f, .0010f);
			particles[i].vz = randomFloat(-0.005f, 0.005f);


    }


	





	const std::string RocketFragment = "#version 400 core\n"
	"out vec4 FragColor;\n"
	"precision mediump float; \n"

	"in vec3 v_frag_coord; \n"
	"in vec3 v_normal; \n"

	"uniform vec3 u_view_pos; \n"

	"uniform samplerCube cubemapSampler; \n"


	"void main() { \n"
	"vec3 N = normalize(v_normal);\n"
	"vec3 V = normalize(u_view_pos - v_frag_coord); \n"
	"vec3 R = reflect(-V,N); \n"
	
	"FragColor = texture(cubemapSampler,R) + vec4(R*0.3, 1.0); \n"
	"} \n";


	Shader ShaderRocket = Shader(sourceV, RocketFragment);
	//Shader ShaderRocket = Shader(sourceV, sourceF);



	char pathrocket[] = PATH_TO_OBJECTS "/Toy_Rocket.obj";
	Object rocket(pathrocket);
	rocket.makeObject(shader);	
	
	float rotAngle = glm::radians(0.0f);
	rocket.model = glm::translate(rocket.model, glm::vec3(0.0, 0.0, 0.0));
	rocket.model = glm::scale(rocket.model, glm::vec3(.5,.5,.5));
	//rocket.model = glm::translate(rocket.model, glm::vec3(0.0, 0.0, 7.0));
	

	rocket.model = glm::rotate(rocket.model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	rocket.model = glm::rotate(rocket.model, glm::radians(180.0f), glm::vec3(1, 0, 0));
	
	

		












	
	// // Framebuffer for Shadow Map
	// unsigned int shadowMapFBO;
	// glGenFramebuffers(1, &shadowMapFBO);

	// // Texture for Shadow Map FBO
	// unsigned int shadowMapWidth = 2048, shadowMapHeight = 2048;
	// unsigned int shadowMap;
	// glGenTextures(1, &shadowMap);
	// glBindTexture(GL_TEXTURE_2D, shadowMap);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// // Prevents darkness outside the frustrum
	// float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	// glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	// // Needed since we don't touch the color buffer
	// glDrawBuffer(GL_NONE);
	// glReadBuffer(GL_NONE);
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// glm::vec3 light_pos2 = glm::vec3(0.0f, 0.0f, -7.0f);
	// glm::mat4 orthgonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 75.0f);
	// glm::mat4 lightView = glm::lookAt(light_pos2, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

	// glm::mat4 lightProjection = orthgonalProjection * lightView;



	// const std::string ShadowMapV ="#version 330 core\n"
	// "layout (location = 0) in vec3 aPos;\n"

	// "uniform mat4 lightProjection;\n"
	// "uniform mat4 model;\n"

	// "void main(){\n"
	// "gl_Position = lightProjection * model * vec4(aPos, 1.0);\n"
	// "}\n";

	// const std::string ShadowMapF = "#version 330 core\n"

	// "void main()\n"
	// "{\n"
	// "}\n";



	// Shader ShadowMapShader = Shader(ShadowMapV, ShadowMapF);
	// ShadowMapShader.use();
	// glUniformMatrix4fv(glGetUniformLocation(ShadowMapShader.ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));




    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



	

	char pathShadowF[] = "/Users/cyrilvanleer/info-h502_202324/LAB03/exercices/project/ShadowFragment.fs";
	char pathShadowV[] = "/Users/cyrilvanleer/info-h502_202324/LAB03/exercices/project/ShadowVertex.vs";

	Shader simpleDepthShader = Shader(pathShadowV, pathShadowF);


	char pathShader2F[] = "/Users/cyrilvanleer/info-h502_202324/LAB03/exercices/project/Shader2Fragment.fs";
	char pathShader2V[] = "/Users/cyrilvanleer/info-h502_202324/LAB03/exercices/project/Shader2Vertex.vs";

	Shader shader2 = Shader(pathShader2V, pathShader2F);

	char pathShaderdebugF[] = "/Users/cyrilvanleer/info-h502_202324/LAB03/exercices/project/ShaderdebugF.fs";
	char pathShaderdebugV[] = "/Users/cyrilvanleer/info-h502_202324/LAB03/exercices/project/ShaderdebugV.vs";

	Shader shaderdebug = Shader(pathShaderdebugV, pathShaderdebugF);


	// shader configuration
    // --------------------
    shaderdebug.use();
    shaderdebug.setInteger("depthMap", 0);

	// lighting info
    // -------------
    glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
	//glm::vec3 lightPos(10.0f, 10.0f, 10.0f);










	
	int time = 0;
	//float angle = randomFloat(0.0f, 90.0f);
	float angle = PI/2;
	

	double lastFrame = 0;
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float deltaTime = now - lastFrame;
    	lastFrame = now;

		time += 1;


		// // 1. render depth of scene to texture (from light's perspective)
        // // --------------------------------------------------------------
        // glm::mat4 lightProjection, lightView;
        // glm::mat4 lightSpaceMatrix;
        // float near_plane = 1.0f, far_plane = 7.5f;
    	// lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        // lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        // lightSpaceMatrix = lightProjection * lightView;
        // // render scene from light's point of view
        // simpleDepthShader.use();
        // simpleDepthShader.setMatrix4("lightSpaceMatrix", lightSpaceMatrix);




		// glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        // glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        //     glClear(GL_DEPTH_BUFFER_BIT);
        //     glActiveTexture(GL_TEXTURE0);
        //     //renderScene(simpleDepthShader);
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//         // reset viewport
        // glViewport(0, 0, width, height);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // // render Depth map to quad for visual debugging
        // // ---------------------------------------------
        // shaderdebug.use();
        // shaderdebug.setFloat("near_plane", near_plane);
        // shaderdebug.setFloat("far_plane", far_plane);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, depthMap);
        // renderQuad();






		
		

		float rotationSpeed{glm::radians(45.0f)}; // 30 degrees per second
		// Update rotation parameters for the sphere
		rotationAngle += rotationSpeed * deltaTime; // Adjust rotation speed as needed
		glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f); // Arbitrary rotation axis, adjust as needed
		glm::quat rotationQuaternion = glm::angleAxis(rotationAngle, glm::normalize(rotationAxis));

		float rotationSpeedZ{glm::radians(10.0f)}; // 30 degrees per second
		rotationAngleZ += rotationSpeedZ * deltaTime; // Rotation around Z-axis
    	glm::quat rotationQuaternionZ = glm::angleAxis(rotationAngleZ, glm::vec3(1.0f, 0.0f, 0.0f));

		// Update sphere model matrix
		glm::mat4 modelMatrix = glm::mat4(rotationQuaternion);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, -15.0));

		glm::mat4 inverseModel = glm::transpose( glm::inverse(modelMatrix));
		
		
		shader.use();

		shader.setMatrix4("M", modelMatrix);
		shader.setMatrix4("itM", inverseModel);
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);

		
		//auto delta = light_pos + glm::vec3(0.0,0.0,2 * std::sin(now));

		auto delta = light_pos;

		shader.setVector3f("light.light_pos", delta);
		
		
		glDepthFunc(GL_LEQUAL);
		sphere1.draw();


		// send sphere 2 

		glm::mat4 inverseModel2 = glm::transpose( glm::inverse(sphere2.model));

		shader.setMatrix4("M", sphere2.model);
		shader.setMatrix4("itM", inverseModel2);
		sphere2.draw();


		ShaderRocket.use();

		//rocket.model = glm::translate(rocket.model, glm::vec3(0.0, , 0.0));
		

		glm::mat4 inverseRocket = glm::transpose( glm::inverse(rocket.model));

		float rotSpeed{glm::radians(405.0f)}; // 30 degrees per second
		
		rotAngle += rotSpeed * deltaTime; // Adjust r

		glm::mat4 rot = glm::rotate(rocket.model, rotAngle, glm::vec3(1, 0, 0));

		ShaderRocket.setMatrix4("M", glm::translate(rocket.model, glm::vec3(1.0* std::sin(4*now), 7.0* std::cos(now/4), 7.0* std::sin(now/4))));
		//ShaderRocket.setMatrix4("M",rocket.model);
		ShaderRocket.setMatrix4("itM", inverseRocket);
		ShaderRocket.setMatrix4("V", view);
		ShaderRocket.setMatrix4("P", perspective);
		ShaderRocket.setVector3f("u_view_pos", camera.Position);

		//shader.setMatrix4("M", rocket.model);
		rocket.draw();


		// moon 

		shader.use();
		float rotationSpeedMoon{glm::radians(30.0f)}; // 30 degrees per second
		float earthOrbitRadius = 5.0;
		rotationAngleMoon += rotationSpeedMoon * deltaTime; // Rotation around own axis
    	glm::quat rotationQuaternionMoon = glm::angleAxis(rotationAngleMoon, glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 earthPosition = modelMatrix*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    	glm::vec3 rotatedEarthPosition = rotationQuaternion * earthPosition;

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
		moon.draw();


		// Particles 

		shaderParticle.use();
		shaderParticle.setMatrix4("itM", inverseModel);
		shaderParticle.setMatrix4("V", view);
		shaderParticle.setMatrix4("P", perspective);
		shaderParticle.setVector3f("u_view_pos", camera.Position);


		if (time == 60) {
			angle = randomFloat(0.0f, 2*PI);
			time = 0;
		}

		
		for (int i = 0; i < numParticles; ++i) {
            particles[i].x += particles[i].vx;
            //particles[i].y += particles[i].vy*deltaTime -85*pow(deltaTime,2);
			particles[i].y += particles[i].vy*deltaTime -82*pow(deltaTime,2);
			particles[i].z += particles[i].vz;
			

            particles[i].lifespan -= 1.0f;
            if (particles[i].lifespan <= 0.0f) {
                // Reset particle or remove it from the simulation
                // particles[i].x = randomFloat(0.0f, .0010f);®
                // particles[i].y = randomFloat(-0.0f, 0.3f);
                // particles[i].vx = randomFloat(-0.005f, 0.005f);
                // particles[i].vy = randomFloat(.5f, .7f);

                // particles[i].lifespan = 80.0f;  // Reset lifespan


				particles[i].x = randomFloat(0.0f, .0010f);
                particles[i].y = randomFloat(-0.0f, 0.2f);
                particles[i].vx = randomFloat(-0.002f, 0.002f);
                particles[i].vy = randomFloat(1.0f, 1.3f);

                particles[i].lifespan = 80.0f;  // Reset lifespan

				particles[i].z = randomFloat(0.0f, .0010f);
				particles[i].vz = randomFloat(-0.002f, 0.002f);

            }
        }

        // Render particles
        for (int i = 0; i < numParticles; ++i) {
            

			//shader.setMatrix4("M",glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(particles[i].x  , particles[i].y +2, 0.0f)), glm::vec3(0.03, 0.03, 0.03)));


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
			
			float modx = rotation_matrix[0][0] * original_x + rotation_matrix[0][1] * original_y;
			float mody = rotation_matrix[1][0] * original_x + rotation_matrix[1][1] * original_y;
			float modz = rotation_matrix[2][0] * original_x + rotation_matrix[2][1] * original_y + rotation_matrix[2][2] * particles[i].z;

			modx = modx - 2 * cos(angle);
			mody = mody + 2 * sin(angle);


		


			//shaderParticle.setMatrix4("M",glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(modx, mody, modz )), glm::vec3(0.03, 0.03, 0.03)));


			shaderParticle.setMatrix4("M",glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(modifiedX, modifiedY, modifiedZ )), glm::vec3(0.03, 0.03, 0.03)));

			particule.draw();
		}


		//updateParticles(0.016f);  // Assume constant frame rate for simplicity
        //renderParticles(shaderProgram);


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

	return 0;
}

void loadCubemapFace(const char * path, const GLenum& targetFace)
{
	int imWidth, imHeight, imNrChannels;
	unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{

		glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(targetFace);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}
	stbi_image_free(data);
}


void processInput(GLFWwindow* window) {
	//3. Use the cameras class to change the parameters of the camera
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


}





