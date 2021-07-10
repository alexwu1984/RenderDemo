#pragma once
#include <btBulletDynamicsCommon.h>
#include <functional>
#include <list>
#include <tuple>
#include "MathLib.h"

struct BulletRenderItem;

class BulletPhysic
{
public:
	BulletPhysic();
	~BulletPhysic();

	void Init();
	void UnInit();
	
	void CreateGroundPlane();
	void CreateDynamicObject(BulletRenderItem* item);
	void UpdateScene(float deltaTime);
	void RemoveItems();

private:
	std::tuple< Vector3f, float > GenItemPos();

private:
	//variables for to bullet physics API
	btBroadphaseInterface* m_broadphase = nullptr;
	btCollisionDispatcher* m_dispatcher = nullptr;
	btConstraintSolver* m_solver = nullptr;
	btDefaultCollisionConfiguration* m_collisionConfiguration = nullptr;
	btDynamicsWorld* m_dynamicsWorld = nullptr; //this is the most important class
	std::list< btCollisionObject*> m_reUseList;
};