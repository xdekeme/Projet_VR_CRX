project("PROJECT_CRX")


set(SOURCES_MAIN 
"code/main.cpp" 
"code/colladainterface.cpp" 
"headers/camera.h" 
"headers/shader.h" 
"headers/colladaobject.h" 
"headers/colladaanimation.h" 
"headers/object.h" 
"headers/object_bumping.h"  
"headers/colladainterface.h"
"headers/collision.h"
"headers/particle.h")

#These commands are there to specify the path to the folder containing the object and textures files as macro
#With these you can just use PATH_TO_OBJECTS and PATH_TO_TEXTURE in your c++ code and the compiler will replace it by the correct expression
add_compile_definitions(PATH_TO_OBJECTS="${CMAKE_CURRENT_SOURCE_DIR}/objects")
add_compile_definitions(PATH_TO_TEXTURE="${CMAKE_CURRENT_SOURCE_DIR}/textures")
add_compile_definitions(PATH_TO_HEADERS="${CMAKE_CURRENT_SOURCE_DIR}/headers")
add_compile_definitions(PATH_TO_SOURCES="${CMAKE_CURRENT_SOURCE_DIR}/source")

add_executable(${PROJECT_NAME}_main ${SOURCES_MAIN})
target_link_libraries(${PROJECT_NAME}_main PUBLIC OpenGL::GL glfw glad tinyxml BulletDynamics BulletCollision LinearMath assimp)
