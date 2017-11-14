#include "MyEntityManager.h"
using namespace Simplex;
//  MyEntityManager
MyEntityManager* MyEntityManager::m_pInstance = nullptr;
std::vector<MyEntity> entities;
std::vector<String> ids;

void MyEntityManager::Init(void)
{
	entities = std::vector<MyEntity>();
	ids = std::vector<String>();
}
void MyEntityManager::Release(void)
{
}
MyEntityManager* MyEntityManager::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new MyEntityManager();
	}
	return m_pInstance;
}
void MyEntityManager::ReleaseInstance()
{
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
int Simplex::MyEntityManager::GetEntityIndex(String a_sUniqueID)
{
	int index = -1;
	for (uint i = 0; i < ids.size(); i++)
	{
		if (ids[i] == a_sUniqueID)
		{
			index = i;
		}
	}
	return index;
}
//Accessors
Model* Simplex::MyEntityManager::GetModel(uint a_uIndex)
{
	return entities[a_uIndex].GetModel();
}
Model* Simplex::MyEntityManager::GetModel(String a_sUniqueID)
{
	int index = GetEntityIndex(a_sUniqueID);

	if (index != -1) {
		return entities[index].GetModel();
	}
}
RigidBody* Simplex::MyEntityManager::GetRigidBody(uint a_uIndex)
{
	return entities[a_uIndex].GetRigidBody();
}
RigidBody* Simplex::MyEntityManager::GetRigidBody(String a_sUniqueID)
{
	int index = GetEntityIndex(a_sUniqueID);

	if (index != -1) {
		return entities[index].GetRigidBody();
	}
}
matrix4 Simplex::MyEntityManager::GetModelMatrix(uint a_uIndex)
{
	return entities[a_uIndex].GetModelMatrix();
}
matrix4 Simplex::MyEntityManager::GetModelMatrix(String a_sUniqueID)
{
	int index = GetEntityIndex(a_sUniqueID);

	if (index != -1) {
		return entities[index].GetModelMatrix();
	}
}
void Simplex::MyEntityManager::SetModelMatrix(matrix4 a_m4ToWorld, String a_sUniqueID)
{
	int index = GetEntityIndex(a_sUniqueID);

	if (index != -1) {
		entities[index].SetModelMatrix(a_m4ToWorld);
	}
}
void Simplex::MyEntityManager::SetModelMatrix(matrix4 a_m4ToWorld, uint a_uIndex)
{
	entities[a_uIndex].SetModelMatrix(a_m4ToWorld);
}
//The big 3
MyEntityManager::MyEntityManager(){Init();}
MyEntityManager::MyEntityManager(MyEntityManager const& other){ }
MyEntityManager& MyEntityManager::operator=(MyEntityManager const& other) { return *this; }
MyEntityManager::~MyEntityManager(){Release();};
// other methods
void Simplex::MyEntityManager::Update(void)
{
}
void Simplex::MyEntityManager::AddEntity(String a_sFileName, String a_sUniqueID)
{
	MyEntity newEntity = MyEntity(a_sFileName, a_sUniqueID);
	entities.push_back(newEntity);
}
void Simplex::MyEntityManager::RemoveEntity(uint a_uIndex)
{
}
void Simplex::MyEntityManager::RemoveEntity(String a_sUniqueID)
{

}
String Simplex::MyEntityManager::GetUniqueID(uint a_uIndex)
{
	return "";
}
MyEntity* Simplex::MyEntityManager::GetEntity(uint a_uIndex)
{
	return nullptr;
}
void Simplex::MyEntityManager::AddEntityToRenderList(uint a_uIndex, bool a_bRigidBody)
{

}
void Simplex::MyEntityManager::AddEntityToRenderList(String a_sUniqueID, bool a_bRigidBody)
{

}