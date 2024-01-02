#ifndef BUMPINGOBJECT_H
#define BUMPINGOBJECT_H

#include<iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>


struct BumpingVertex {
    glm::vec3 Position;
    glm::vec2 Texture;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

class BumpingObject {
public:
    std::vector<BumpingVertex> vertices;
	int numVertices;

	GLuint VBO, VAO;

	glm::mat4 model = glm::mat4(1.0);

    BumpingObject(const char* path) {
        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<glm::vec3> temp_vertices;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_normals;

        std::ifstream infile(path);
        std::string line;

        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::string header;
            iss >> header;

            if (header == "v") {
                glm::vec3 vertex;
                iss >> vertex.x >> vertex.y >> vertex.z;
                temp_vertices.push_back(vertex);
            } else if (header == "vt") {
                glm::vec2 uv;
                iss >> uv.x >> uv.y;
                temp_uvs.push_back(uv);
            } else if (header == "vn") {
                glm::vec3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                temp_normals.push_back(normal);
            } else if (header == "f") {
                std::string vertex1, vertex2, vertex3;
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

                iss >> vertex1 >> vertex2 >> vertex3;
                parseVertex(vertex1, vertexIndex[0], uvIndex[0], normalIndex[0]);
                parseVertex(vertex2, vertexIndex[1], uvIndex[1], normalIndex[1]);
                parseVertex(vertex3, vertexIndex[2], uvIndex[2], normalIndex[2]);

                for (int i = 0; i < 3; i++) {
                    vertexIndices.push_back(vertexIndex[i]);
                    uvIndices.push_back(uvIndex[i]);
                    normalIndices.push_back(normalIndex[i]);
                }
            }
        }

        for (unsigned int i = 0; i < vertexIndices.size(); i += 3) {
            BumpingVertex v0 = processVertex(vertexIndices[i], uvIndices[i], normalIndices[i], temp_vertices, temp_uvs, temp_normals);
            BumpingVertex v1 = processVertex(vertexIndices[i + 1], uvIndices[i + 1], normalIndices[i + 1], temp_vertices, temp_uvs, temp_normals);
            BumpingVertex v2 = processVertex(vertexIndices[i + 2], uvIndices[i + 2], normalIndices[i + 2], temp_vertices, temp_uvs, temp_normals);

            computeTangents(v0, v1, v2);

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);
        }

		numVertices = vertices.size();
    }

	void makeBumpingObject(Shader shader, bool texture = true) {
		std::cout << numVertices << std::endl;
		float* data = new float[14 * numVertices]; // 14 = 3(pos) + 2(tex) + 3(norm) + 3(tangent) + 3(bitangent)
		for (int i = 0; i < numVertices; i++) {
			BumpingVertex v = vertices.at(i);
			// Position
			data[i * 14] = v.Position.x;
			data[i * 14 + 1] = v.Position.y;
			data[i * 14 + 2] = v.Position.z;
			// Texture coordinates
			data[i * 14 + 3] = v.Texture.x;
			data[i * 14 + 4] = v.Texture.y;
			// Normals
			data[i * 14 + 5] = v.Normal.x;
			data[i * 14 + 6] = v.Normal.y;
			data[i * 14 + 7] = v.Normal.z;
			// Tangents
			data[i * 14 + 8] = v.Tangent.x;
			data[i * 14 + 9] = v.Tangent.y;
			data[i * 14 + 10] = v.Tangent.z;
			// Bitangents
			data[i * 14 + 11] = v.Bitangent.x;
			data[i * 14 + 12] = v.Bitangent.y;
			data[i * 14 + 13] = v.Bitangent.z;
		}

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		//define VBO and VAO as active buffer and active vertex array
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(BumpingVertex) * numVertices, data, GL_STATIC_DRAW);

		auto att_pos = glGetAttribLocation(shader.ID, "position");
		glEnableVertexAttribArray(att_pos);
		glVertexAttribPointer(att_pos, 3, GL_FLOAT, false, 14 * sizeof(float), (void*)0);

		
		if (texture) { 
			auto att_tex = glGetAttribLocation(shader.ID, "tex_coord");
			glEnableVertexAttribArray(att_tex);
			glVertexAttribPointer(att_tex, 2, GL_FLOAT, false, 14 * sizeof(float), (void*)(3 * sizeof(float)));
			
		}
		
		auto att_col = glGetAttribLocation(shader.ID, "normal");
		glEnableVertexAttribArray(att_col);
		glVertexAttribPointer(att_col, 3, GL_FLOAT, false, 14 * sizeof(float), (void*)(5 * sizeof(float)));

		// Tangents
		auto att_tangent = glGetAttribLocation(shader.ID, "aTangent");
		glEnableVertexAttribArray(att_tangent);
		glVertexAttribPointer(att_tangent, 3, GL_FLOAT, false, 14 * sizeof(float), (void*)(8 * sizeof(float)));

		// Bitangents
		auto att_bitangent = glGetAttribLocation(shader.ID, "aBitangent");
		glEnableVertexAttribArray(att_bitangent);
		glVertexAttribPointer(att_bitangent, 3, GL_FLOAT, false, 14 * sizeof(float), (void*)(11 * sizeof(float)));
		
		//desactive the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		delete[] data;

	}

	void draw() {
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
	}

private:
    void parseVertex(const std::string &vertexToken, unsigned int &vertexIndex, unsigned int &uvIndex, unsigned int &normalIndex) {
        std::istringstream tokenStream(vertexToken);
        std::string indexString;
        
        std::getline(tokenStream, indexString, '/');
        vertexIndex = std::stoi(indexString);

        std::getline(tokenStream, indexString, '/');
        uvIndex = std::stoi(indexString);

        std::getline(tokenStream, indexString);
        normalIndex = std::stoi(indexString);
    }

    BumpingVertex processVertex(unsigned int vertexIndex, unsigned int uvIndex, unsigned int normalIndex, const std::vector<glm::vec3> &temp_vertices, const std::vector<glm::vec2> &temp_uvs, const std::vector<glm::vec3> &temp_normals) {
        BumpingVertex vertex;

        vertex.Position = temp_vertices[vertexIndex - 1];
        vertex.Texture = temp_uvs[uvIndex - 1];
        vertex.Normal = temp_normals[normalIndex - 1];

        return vertex;
    }

    void computeTangents(BumpingVertex &v0, BumpingVertex &v1, BumpingVertex &v2) {
        glm::vec3 edge1 = v1.Position - v0.Position;
        glm::vec3 edge2 = v2.Position - v0.Position;
        glm::vec2 deltaUV1 = v1.Texture - v0.Texture;
        glm::vec2 deltaUV2 = v2.Texture - v0.Texture;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent, bitangent;

        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        v0.Tangent = glm::normalize(tangent);
        v0.Bitangent = glm::normalize(bitangent);
        v1.Tangent = glm::normalize(tangent);
        v1.Bitangent = glm::normalize(bitangent);
        v2.Tangent = glm::normalize(tangent);
        v2.Bitangent = glm::normalize(bitangent);
    }
};
#endif