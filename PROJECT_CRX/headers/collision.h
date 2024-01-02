#ifndef COLLISION_H
#define COLLISION_H

#include<iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>

#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <LinearMath/btVector3.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>

#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <LinearMath/btDefaultMotionState.h>
#include <LinearMath/btMatrix3x3.h>
#include <LinearMath/btAlignedObjectArray.h>


class Collision
{
public:

	static btDynamicsWorld* createPhysicalWorld() {
		btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
		btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
		btBroadphaseInterface* broadphase = new btDbvtBroadphase();
		btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
    	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

    	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    	dynamicsWorld->setGravity(btVector3(0, 0, 0)); //Gravity second term but =0 here because we are in the space

		return dynamicsWorld;

	}

	static btRigidBody* createRigidBody(btDynamicsWorld* dynamicsWorld, float radiusValue, int massValue, btVector3 startPosition) { 
		btCollisionShape* planetShape = new btSphereShape(radiusValue); 

		btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), startPosition));
		btScalar mass(massValue); 
		btVector3 inertia(0, 0, 0);
		planetShape->calculateLocalInertia(mass, inertia);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, planetShape, inertia);
		btRigidBody* planetRigidBody = new btRigidBody(rbInfo);

		//Add the rigid body to the dynamic world
		dynamicsWorld->addRigidBody(planetRigidBody);

		return planetRigidBody;

	}

	static void freePhysicalWorld(btDynamicsWorld* dynamicsWorld){
		//Delete all the rigid body from the physical world
		for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);

			if (body && body->getMotionState()) {
				delete body->getMotionState();
			}

			dynamicsWorld->removeCollisionObject(obj);
			delete obj;
		}

		//Delete all the collision Shapes
		// for (int j = 0; j < collisionShapes.size(); j++) {
		// 	btCollisionShape* shape = collisionShapes[j];
		// 	collisionShapes[j] = 0;
		// 	delete shape;
		// }

		//Delete the actual world
		delete dynamicsWorld;
	}
};
#endif