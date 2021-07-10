#include "BulletPhysics.h"
#include "Geometry.h"
#include "BulletRenderItem.h"

BulletPhysic::BulletPhysic()
{

}

BulletPhysic::~BulletPhysic()
{

}

void BulletPhysic::Init()
{
	//��������Ŀռ��С���޶������˶��Ŀռ䷶Χ
	btVector3 worldAabbMin(-1, -1, -1);
	btVector3 worldAabbMax(1, 1, 1);
	//������������
	int maxProxies = 1024;

	m_collisionConfiguration = new btDefaultCollisionConfiguration(); //collision configuration contains default setup for memory, collision setup
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration); //use the default collision dispatcher 
	m_broadphase = new btAxisSweep3(worldAabbMin, worldAabbMax, maxProxies);
	m_solver = new btSequentialImpulseConstraintSolver; //the default constraint solver
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_dynamicsWorld->setGravity(btVector3(0, -20, 0));
}

void BulletPhysic::UnInit()
{
	for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) //remove the rigidbodies from the dynamics world and delete them
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
			delete body->getMotionState();

		if (body && body->getCollisionShape())
		{
			delete body->getCollisionShape();
		}

		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}


	delete m_dynamicsWorld;
	m_dynamicsWorld = nullptr;
	delete m_solver;
	m_solver = nullptr;
	delete m_broadphase;
	m_broadphase = nullptr;
	delete m_dispatcher;
	m_dispatcher = nullptr;
	delete m_collisionConfiguration;
	m_collisionConfiguration = nullptr;
}

void BulletPhysic::CreateGroundPlane()
{
	//���� ����ĳ�ʼλ����ת�Ƕ���Ϣ����ת�Ƕ�0��λ����Y��-1����
	btDefaultMotionState* groundMotionState = new  btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -2.5, 0)));
	//���� ��̬ƽ����״
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	//����������Ϣ
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	//����������Ϣ ��������
	btRigidBody* groundbody = new btRigidBody(groundRigidBodyCI);
	//����Ħ��ϵ��
	groundbody->setFriction(2.0f);
	groundbody->setRestitution(0.5);
	//�����������ӵ� ��������
	m_dynamicsWorld->addRigidBody(groundbody);
}

void BulletPhysic::CreateDynamicObject(BulletRenderItem* item)
{
	auto [Pos, Restitution] = GenItemPos();
	//���� ����ĳ�ʼλ����ת�Ƕ���Ϣ����ת�Ƕ�0��λ����Y��10����ĸ߿�
	btTransform boxTransform;
	boxTransform.setIdentity();
	boxTransform.setOrigin(btVector3(Pos.x, Pos.y, Pos.z));

	btDefaultMotionState* ballMotionState = new btDefaultMotionState(boxTransform);

	float r = item->Geo.GetBoundBox().Extents.x;
	btBoxShape* boxShape = new btBoxShape(btVector3(r, r, r));
	boxShape->setLocalScaling(btVector3(item->Geo.GetScale(), item->Geo.GetScale(), item->Geo.GetScale()));
	//�����ܶȣ�����أ��ܶ�Ϊ0ʱ�ᱻ��Ϊ��̬���壬��0ʱ����Ϊ��̬���壩
	btScalar mass = 1;
	//����    ������
	btVector3 inertia;
	//�����ܶ��Զ����㲢���ù���     
	boxShape->calculateLocalInertia(mass, inertia);

	//����������Ϣ
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(mass, ballMotionState, boxShape, inertia);

	//����������Ϣ ��������
	btRigidBody* ballBody = new btRigidBody(groundRigidBodyCI);
	ballBody->setUserPointer(item);
	//����Ħ��ϵ��
	ballBody->setFriction(1.0f);
	ballBody->setRestitution(Restitution);

	//���ø�����ӵ�����������
	m_dynamicsWorld->addRigidBody(ballBody);
}

void BulletPhysic::UpdateScene(float deltaTime)
{
	//��������ģ��
   //ͨ��10���Ӳ�����⣬ģ���deltaTime�����������仯��
	m_dynamicsWorld->stepSimulation(deltaTime, 1);

	//������������ÿһ������
	auto& objectArray = m_dynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < objectArray.size(); ++i)
	{
		//���ڲ��״̬�����Ǿ�̬����Ļ����򲻴���
		BulletRenderItem* object = reinterpret_cast<BulletRenderItem*>(objectArray[i]->getUserPointer());
		if (!object)continue;
		if (!objectArray[i]->isActive())
		{
			//object->IsDelete = true;
			continue;
		}
		
		//����Ŀ�������λ��
		const auto& pos = objectArray[i]->getWorldTransform().getOrigin();
		object->Geo.SetPosition(Vector3f(pos.x(), pos.y(), pos.z()));
		//����Ŀ���������ת�Ƕ�
		const auto& rotationQ = objectArray[i]->getWorldTransform().getRotation();
		btMatrix3x3 btMat;
		btMat.setIdentity();
		btMat.setRotation(rotationQ);
		FMatrix rotateMat;
		rotateMat.row[0] = Vector3f(btMat.getRow(0).x(), btMat.getRow(0).y(), btMat.getRow(0).z());
		rotateMat.row[1] = Vector3f(btMat.getRow(1).x(), btMat.getRow(1).y(), btMat.getRow(1).z());
		rotateMat.row[2] = Vector3f(btMat.getRow(2).x(), btMat.getRow(2).y(), btMat.getRow(2).z());
		object->Geo.SetRotation(rotateMat);


	}
}

void BulletPhysic::RemoveItems()
{
	auto& objectArray = m_dynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < objectArray.size(); ++i)
	{
		BulletRenderItem* RenderItem = reinterpret_cast<BulletRenderItem*>(objectArray[i]->getUserPointer());

		if (RenderItem && RenderItem->IsDelete)
		{
			btCollisionObject* obj = objectArray[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}

			if (body && body->getCollisionShape())
			{
				delete body->getCollisionShape();
			}

			m_dynamicsWorld->removeRigidBody(body);
			delete obj;
			--i;
		}
	}

}


std::tuple< Vector3f, float > BulletPhysic::GenItemPos()
{
	static std::uniform_real_distribution<float> RandomPos(-1, 1);
	static std::default_random_engine Generator;

	static std::mt19937 gen(1701);
	static std::discrete_distribution<> distr({ 3, 3, 3, 6, 12 });

	static std::vector<float> data{ 0.2,0.5,0.8,1.0,1.5 };

	return { Vector3f(RandomPos(Generator), 2, 0), data[distr(gen)] };
}
