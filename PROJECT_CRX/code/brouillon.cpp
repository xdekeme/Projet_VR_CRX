void initializeVAOsVBOs(std::vector<ColGeom>& geom_vec, Shader& shader) {
    vaos = new GLuint[geom_vec.size()];
    vbos = new GLuint[geom_vec.size() * 3]; 

    glGenVertexArrays(geom_vec.size(), vaos);
    glGenBuffers(geom_vec.size() * 3, vbos); 

	shader.use();

    for (size_t i = 0; i < geom_vec.size(); ++i) {
        glBindVertexArray(vaos[i]);

        int vboidx = 3 * i; 

        // Position
        glBindBuffer(GL_ARRAY_BUFFER, vbos[vboidx]);
        auto posData = geom_vec[i].map["POSITION"];
        glBufferData(GL_ARRAY_BUFFER, posData.size * sizeof(float), posData.data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Normales
        glBindBuffer(GL_ARRAY_BUFFER, vbos[vboidx + 1]);
        auto normalData = geom_vec[i].map["NORMAL"]; 
        glBufferData(GL_ARRAY_BUFFER, normalData.size * sizeof(float), normalData.data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Indices (EBO)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[vboidx + 2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, geom_vec[i].index_count * sizeof(unsigned short), geom_vec[i].indices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);
}





// Respond to paint events
void display(GLFWwindow* window) {

  	for (int i = 0; i < num_objects; i++) {
    glBindVertexArray(vaos[i]);
    glDrawElements(geom_vec[i].primitive, geom_vec[i].index_count, 
                   GL_UNSIGNED_SHORT, geom_vec[i].indices);
	}	
}

// Respond to reshape events
void reshape(int w, int h) {
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

// Deallocate memory
void deallocate() {

  // Deallocate mesh data
  ColladaInterface::freeGeometries(&geom_vec);

  // Deallocate OpenGL objects
  glDeleteBuffers(num_objects, vbos);
  glDeleteBuffers(2 * num_objects, vaos);
  glDeleteBuffers(1, &ubo);
  delete(vbos);
  delete(vaos);
}






//COLLADA ANIMATION
// struct Bone {
// 	std::string name;
// 	int parentIndex;
// 	glm::mat4 localMatrix;
// 	glm::mat4 invModelMatrix;
// };


// std::vector<Bone> bones;
    // std::unordered_map<std::string, int> boneNameToIndexMap;

    // void LoadBonesFromCollada(const std::string& filename) {
    //     Assimp::Importer importer;
    //     const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate);
        
    //     if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    //         throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));
    //     }
        
    //     ProcessNode(scene->mRootNode, -1, scene);
    // }

	// void ReplaceNamesWithParentIndices() { //To get rid of the name "Scene", "Camera" and "Cube" that were present in my file
    //     std::vector<Bone> relevantBones;

    //     for (Bone& bone : bones) {
    //         if (bone.name != "Scene" && bone.name != "Camera" && bone.name != "Cube") {
    //             auto it = boneNameToIndexMap.find(bone.name);
    //             if (it != boneNameToIndexMap.end()) {
    //                 bone.parentIndex = it->second;
    //             }
    //             relevantBones.push_back(bone); 
    //         }
    //     }
    //     bones = relevantBones;
    // }

	// void PrintBonesAndIndices() const { //Help me visualize if it was correct or not
    //     std::cout << "Bone Indexes:" << std::endl;
    //     for (size_t i = 0; i < bones.size(); ++i) {
    //         std::cout << "Bone Name: " << bones[i].name 
    //                   << ", Index: " << i 
    //                   << ", Parent Index: " << bones[i].parentIndex << std::endl;
    //     }
    // }

	// void CalculateAnimationPose(
	// 	std::vector<Bone>& bones,
	// 	std::vector<glm::mat4>& inMatrices,
	// 	std::vector<glm::mat4>& outMatrices
	// )
	// {
	// 	std::vector<glm::mat4> localTransform(bones.size());
	// 	std::vector<glm::mat4> modelTransform(bones.size());

	// 	for (int i = 0; i < bones.size(); i++)
	// 	{
	// 		localTransform[i] = bones[i].localMatrix * inMatrices[i];
	// 	}

	// 	modelTransform[0] = localTransform[0];

	// 	for (int i = 1; i < bones.size(); i++)
	// 	{
	// 		int parent = bones[i].parentIndex;
	// 		modelTransform[i] = modelTransform[parent] * localTransform[i];
	// 	}

	// 	for (int i = 0; i < bones.size(); i++)
	// 	{
	// 		outMatrices[i] = modelTransform[i] * bones[i].invModelMatrix;
	// 	}
	// }


	//void ProcessBones(const aiScene* scene) {
	// 	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
	// 		const aiMesh* mesh = scene->mMeshes[i];
	// 		for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
	// 			const aiBone* bone = mesh->mBones[j];
	// 			if (boneNameToIndexMap.find(bone->mName.C_Str()) != boneNameToIndexMap.end()) {
	// 				int boneIndex = boneNameToIndexMap[bone->mName.C_Str()];
	// 				bones[boneIndex].invModelMatrix = ConvertMatrix(bone->mOffsetMatrix);
	// 			}
	// 		}
	// 	}
	// }

    // void ProcessNode(aiNode* node, int parentIndex, const aiScene* scene) {
    //     Bone bone;
    //     bone.name = node->mName.C_Str();
    //     bone.parentIndex = parentIndex;
    //     bone.localMatrix = ConvertMatrix(node->mTransformation);

    //     int currentIndex = bones.size();
    //     bones.push_back(bone);
    //     boneNameToIndexMap[bone.name] = currentIndex;

    //     for (unsigned int i = 0; i < node->mNumChildren; i++) {
    //         ProcessNode(node->mChildren[i], currentIndex, scene);
    //     }
	// 	ReplaceNamesWithParentIndices();
	// 	ProcessBones(scene);
    // }

    // glm::mat4 ConvertMatrix(const aiMatrix4x4& from) {
    //     glm::mat4 to;
    //     // Assimp matrix is row-major in memory, so we transpose it to match glm's column-major format
    //     to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    //     to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    //     to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    //     to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    //     return to;
    // }





//SPACESHIP
#ifndef SPACESHIP_H
#define SPACESHIP_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Spaceship {
public:
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::vec3 RotationAxis;
    float RotationAngle;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    float MovementSpeed;
    float MouseSensitivity;

    Spaceship(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
              glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
              glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f),
              float rotationAngle = 0.0f,
              float movementSpeed = 2.5f,
              float mouseSensitivity = 0.1f)
        : Position(position), Scale(scale), RotationAxis(rotationAxis), RotationAngle(rotationAngle), MovementSpeed(movementSpeed),  MouseSensitivity(mouseSensitivity) {
        Front = glm::vec3(0.0f, 0.0f, -1.0f);
        Up = glm::vec3(0.0f, 1.0f, 0.0f);
        Right = glm::normalize(glm::cross(Front, Up));
    }

    void MoveForward(float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        Position += Front * velocity;
    }

    void MoveBackward(float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        Position -= Front * velocity;
    }

    void MoveRight(float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        Position += Right * velocity;
    }

    void MoveLeft(float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        Position -= Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset) {
        xoffset *= MouseSensitivity;

        // Ajuster l'angle de rotation autour de l'axe Y
        RotationAngle += xoffset;

        // Mettre à jour la direction du vaisseau
        UpdateDirection();
    }

private:
    void UpdateDirection() {
        // Calculer la nouvelle direction du vaisseau
        Front.x = cos(glm::radians(RotationAngle));
        Front.z = sin(glm::radians(RotationAngle));
        Front = glm::normalize(Front);

        // Calculer Right en fonction de la nouvelle direction Front
        Right = glm::normalize(glm::cross(Front, Up));
    }
};

#endif // SPACESHIP_H