#ifndef COLLADAOBJECT_H
#define COLLADAOBJECT_H

#include<iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "colladainterface.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



struct ColladaVertex {
	glm::vec3 Position;
	glm::vec2 Texture;
	glm::vec3 Normal;
};


class ColladaObject
{
public:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;

	GLuint *vaos, *vbos;
	GLuint ibo;
	int num_objects; 

	glm::mat4 model = glm::mat4(1.0);

	void makeObjectCollada(Shader shader, std::vector<ColGeom>* geom_vec) {

		num_objects = (int) geom_vec->size();
		int loc;

		vaos = new GLuint[num_objects];
		glGenVertexArrays(num_objects, vaos);

		vbos = new GLuint[num_objects];
		glGenBuffers(num_objects, vbos);

		for(int i=0; i<num_objects; i++) {

			glBindVertexArray(vaos[i]);

			GLsizei positionSize = (*geom_vec)[i].map["POSITION"].stride * sizeof(float);
			int positionCount = (*geom_vec)[i].map["POSITION"].size;
			GLsizei normalSize = (*geom_vec)[i].map["NORMAL"].stride * sizeof(float);
			int normalCount = (*geom_vec)[i].map["NORMAL"].size;
			GLsizei texCoordSize = (*geom_vec)[i].map.find("TEXCOORD") != geom_vec->at(i).map.end() ? (*geom_vec)[i].map["TEXCOORD"].stride * sizeof(float) : 0;
			int textureCount = (*geom_vec)[i].map["TEXCOORD"].size;
			GLsizei vertexSize = positionSize + normalSize + texCoordSize;
			GLsizei totalSize = vertexSize * (*geom_vec)[i].index_count;
			int vertexSizeInt = (*geom_vec)[i].map["POSITION"].stride + (*geom_vec)[i].map["NORMAL"].stride  + (*geom_vec)[i].map["TEXCOORD"].stride; 

			std::cout << (*geom_vec)[i].index_count << std::endl;

			float* positionData = static_cast<float*>((*geom_vec)[i].map["POSITION"].data);
			int position_count = 0;
			for (int index = 0; index < positionCount; index++) { 
				glm::vec3 pos_temp; 
				for (int j = 0; j < (*geom_vec)[i].map["POSITION"].stride; j++) {
					pos_temp[j] = positionData[position_count + j];	
				}
				position_count += (*geom_vec)[i].map["POSITION"].stride;
				positions.push_back(pos_temp);
			}

			float* texCoordData = static_cast<float*>((*geom_vec)[i].map["TEXCOORD"].data);
			int texture_count = 0;
			for (int index = 0; index < textureCount; index++) { 
				glm::vec3 tex_temp; 
				for (int j = 0; j < (*geom_vec)[i].map["TEXCOORD"].stride; j++) {
					tex_temp[j] = texCoordData[texture_count + j];					
				}
				texture_count += (*geom_vec)[i].map["TEXCOORD"].stride;
				textures.push_back(tex_temp);
			}

			float* normalData = static_cast<float*>((*geom_vec)[i].map["NORMAL"].data);
			int normal_count = 0;
			for (int index = 0; index <normalCount ; index++) {
				glm::vec3 nor_temp; 
				for (int j = 0; j < (*geom_vec)[i].map["NORMAL"].stride; j++) {
					nor_temp[j] = normalData[normal_count + j];		
				}
				normal_count += (*geom_vec)[i].map["NORMAL"].stride;
				normals.push_back(nor_temp);
			}

			float* vertexData = new float[vertexSizeInt * (*geom_vec)[i].numVertices];
			int count = 0;
			int count_vertex = 0;
			for (int indiceIndex = 0; indiceIndex < (*geom_vec)[i].index_count; indiceIndex++) {
				int index_value = (*geom_vec)[i].indices[indiceIndex];
				if (count == 2){
					vertexData[count_vertex * 8 + 6] = textures.at(index_value).x;
					vertexData[count_vertex * 8 + 7] = textures.at(index_value).y;
					count = 0;
					count_vertex++;
				}
				else if (count == 1){
					vertexData[count_vertex * 8 + 3] = normals.at(index_value).x;
					vertexData[count_vertex * 8 + 4] = normals.at(index_value).y;
					vertexData[count_vertex * 8 + 5] = normals.at(index_value).z;
					count++;
				}
				else{
					vertexData[count_vertex * 8] = positions.at(index_value).x;
					vertexData[count_vertex * 8 + 1] = positions.at(index_value).y;
					vertexData[count_vertex * 8 + 2] = positions.at(index_value).z;
					count ++;
				}
			}

			// Set vertex coordinate data
			glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
			glBufferData(GL_ARRAY_BUFFER, vertexSize * (*geom_vec)[i].numVertices, vertexData, GL_STATIC_DRAW);

			loc = glGetAttribLocation(shader.ID, "position");
			glEnableVertexAttribArray(loc);
			glVertexAttribPointer(loc, (*geom_vec)[i].map["POSITION"].stride, (*geom_vec)[i].map["POSITION"].type, GL_FALSE, vertexSize, (void*)0);

			loc = glGetAttribLocation(shader.ID, "normal");
			glEnableVertexAttribArray(loc);
			glVertexAttribPointer(loc, (*geom_vec)[i].map["NORMAL"].stride, (*geom_vec)[i].map["NORMAL"].type, GL_FALSE, vertexSize, (void*)(3 * sizeof(float)));
			
			// Set texture coordinate data
			loc = glGetAttribLocation(shader.ID, "tex_coord"); 
			glEnableVertexAttribArray(loc);
			glVertexAttribPointer(loc, (*geom_vec)[i].map["TEXCOORD"].stride, (*geom_vec)[i].map["TEXCOORD"].type, GL_FALSE, vertexSize, (void*)(6 * sizeof(float)));

			// Nettoyage
			delete[] vertexData;
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

		
	void drawCollada(std::vector<ColGeom>* geom_vec) {
		num_objects = (int) geom_vec->size();
		for(int i = 0; i < num_objects; i++) {
			glBindVertexArray(vaos[i]);
			glDrawArrays(GL_TRIANGLES, 0, (*geom_vec)[i].numVertices);
		}
	}

};
#endif