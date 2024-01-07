//Source: https://www.codeproject.com/script/Articles/ViewDownloads.aspx?aid=625701

#include "../headers/colladainterface.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

char array_types[7][15] = {"float_array", "int_array", "bool_array", "Name_array", 
                           "IDREF_array", "SIDREF_array", "token_array"};

char primitive_types[7][15] = {"lines", "linestrips", "polygons", "polylist", 
                               "triangles", "trifans", "tristrips"};


void ColladaInterface::readColladaFile(const char* filename, std::vector<ColGeom>* geometries) {
    readGeometries(geometries, filename);
}


void ColladaInterface::readGeometries(std::vector<ColGeom>* v, const char* filename) {
    TiXmlDocument doc(filename);
    doc.LoadFile();
    TiXmlElement* root = doc.RootElement();

    //Geomeetry
    TiXmlElement* library_geometries = root->FirstChildElement("library_geometries");
    TiXmlElement* geometry = library_geometries->FirstChildElement("geometry");

    while(geometry != NULL) {
        ColGeom data;
        data.name = geometry->Attribute("id");
        TiXmlElement* mesh = geometry->FirstChildElement("mesh");
        
        std::map<std::string, SourceData> sourceMap;
        TiXmlElement* sourceElement = mesh->FirstChildElement("source");
        while(sourceElement != NULL) {
            std::string sourceId = sourceElement->Attribute("id");
            sourceMap[sourceId] = readSource(sourceElement);
            sourceElement = sourceElement->NextSiblingElement("source");
        }

        TiXmlElement* vertices = mesh->FirstChildElement("vertices");
        TiXmlElement* input = vertices->FirstChildElement("input");
        int count = 0;
        while(input != NULL) {
            std::string semantic = input->Attribute("semantic");
            std::string sourceName = input->Attribute("source");
            sourceName.erase(0, 1); //Allow to remove the first character which is '#'

            if(sourceMap.find(sourceName) != sourceMap.end()) {
                data.map[semantic] = sourceMap[sourceName];
            }

            input = input->NextSiblingElement("input");
            count += 1;
        }

        for(int i = 0; i < 7; i++) {
            TiXmlElement* primitive = mesh->FirstChildElement(primitive_types[i]);
            if(primitive != NULL) {
                int prim_count, num_indices;
                primitive->QueryIntAttribute("count", &prim_count);
                switch(i) {
                  case 0: // lines
                      data.primitive = GL_LINES; 
                      num_indices = prim_count * 2; 
                      break;
                  case 1: // linestrips
                      data.primitive = GL_LINE_STRIP; 
                      num_indices = prim_count + 1;
                      break;
                  case 4: // triangles
                      data.primitive = GL_TRIANGLES; 
                      num_indices = prim_count * 3 * 3; //4 pour le character et 3 pour sphere
                      break;
                  case 5: // trifans
                      data.primitive = GL_TRIANGLE_FAN; 
                      num_indices = prim_count + 2; 
                      break;
                  case 6: // tristrips
                      data.primitive = GL_TRIANGLE_STRIP; 
                      num_indices = prim_count + 2; 
                      break;
                  default: 
                      std::cout << "Primitive " << primitive_types[i] << " not supported" << std::endl;
              }

                if(count == 1){ //This means that we have a blender file here
                  unsigned int num_vertices;
                  TiXmlElement* geom_shape = mesh->FirstChildElement(primitive_types[i]);
                  geom_shape->QueryUnsignedAttribute("count", &num_vertices);
                  data.numVertices = num_vertices *3;
                  TiXmlElement* input = geom_shape->FirstChildElement("input");
                  while(input != NULL) {
                      std::string semantic = input->Attribute("semantic");
                      std::string sourceName = input->Attribute("source");
                      sourceName.erase(0, 1); //Allow to remove the first character which is '#'

                      if(sourceMap.find(sourceName) != sourceMap.end()) {
                          data.map[semantic] = sourceMap[sourceName];
                      }
                      input = input->NextSiblingElement("input");
                  }
                }

                char* text = (char*)(primitive->FirstChildElement("p")->GetText());
                data.indices = new unsigned short[num_indices];
                data.indices[0] = (unsigned short)atoi(strtok(text, " "));
                for(int index = 1; index < num_indices; index++) {
                    data.indices[index] = (unsigned short)atoi(strtok(NULL, " "));
                }
                
                data.index_count = num_indices;
            }
        }

        v->push_back(data);
        geometry = geometry->NextSiblingElement("geometry");
    }
}


void ColladaInterface::freeGeometries(std::vector<ColGeom>* v) {
  
  std::vector<ColGeom>::iterator geom_it;
  SourceMap::iterator map_it;

  for(geom_it = v->begin(); geom_it < v->end(); geom_it++) {

    // Deallocate index data
    free(geom_it->indices);

    // Deallocate array data in each map value
    for(map_it = geom_it->map.begin(); map_it != geom_it->map.end(); map_it++) {
      free((*map_it).second.data);
    }

    // Erase the current ColGeom from the vector
    v->erase(geom_it);
  }
}

SourceData readSource(TiXmlElement* source) {
  SourceData source_data;
  TiXmlElement *array;
  char* text;
  unsigned int num_vals, stride;
  int check;

  for(int i=0; i<7; i++) {
    array = source->FirstChildElement(array_types[i]);
    if(array != NULL) {

      // Find number of values
      array->QueryUnsignedAttribute("count", &num_vals);
      source_data.size = num_vals;

      // Find stride
      check = source->FirstChildElement("technique_common")->FirstChildElement("accessor")->QueryUnsignedAttribute("stride", &stride);
      if(check != TIXML_NO_ATTRIBUTE) 
        source_data.stride = stride;
      else
        source_data.stride = 1;

      // Read array values
      text = (char*)(array->GetText());

      // Initialize mesh data according to data type
      switch(i) {

        // Array of floats
        case 0:
          source_data.type = GL_FLOAT;
          //source_data.size *= sizeof(float);
          source_data.data = malloc(num_vals * sizeof(float));

          // Read the float values
          ((float*)source_data.data)[0] = atof(strtok(text, " "));  
          for(unsigned int index=1; index<num_vals; index++) {
            ((float*)source_data.data)[index] = atof(strtok(NULL, " "));   
          }
        break;

        // Array of integers
        case 1:
          source_data.type = GL_INT;
          //source_data.size *= sizeof(int);
          source_data.data = malloc(num_vals * sizeof(int));

          // Read the int values
          ((int*)source_data.data)[0] = atof(strtok(text, " "));  
          for(unsigned int index=1; index<num_vals; index++) {
            ((int*)source_data.data)[index] = atof(strtok(NULL, " "));   
          }
        break;

          // Other
        default:
          std::cout << "Collada Reader doesn't support mesh data in this format" << std::endl;
        break;
      }
    }
  }
  return source_data;

}