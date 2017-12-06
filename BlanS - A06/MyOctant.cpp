#include "MyOctant.h"

using namespace Simplex;

//Static variables to keep track of overall octree information
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 0;
uint MyOctant::m_uIdealEntityCount = 0;

//Initialize an octant given a max level and an ideal entity count
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	//Grab the entity manager and mesh manager
	Init();

	//Set initial variables
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uLevel = 0;

	m_uIdealEntityCount = a_nIdealEntityCount;

	m_uID = m_uOctantCount;
	m_uOctantCount++;

	m_pParent = nullptr;
	m_pRoot = this;

	//Set up child pointers
	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}

	//Grab a comprehensive list of the min and max of every entity
	std::vector<vector3> points;
	for (uint i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
	{
		MyRigidBody* entityRB = m_pEntityMngr->GetEntity(i)->GetRigidBody();
		points.push_back(entityRB->GetMaxGlobal());
		points.push_back(entityRB->GetMinGlobal());
	}

	//Create a new rigidbody from all of the entity points
	MyRigidBody* rigidBody = new MyRigidBody(points);

	//Calculate the largest half width
	vector3 halfWidth = rigidBody->GetHalfWidth();
	float largestDimension = halfWidth.x;

	for (uint i = 1; i < 3; i++)
	{
		if (largestDimension < halfWidth[i])
		{
			largestDimension = halfWidth[i];
		}
	}

	//Calculate the octree's location and size in space
	m_v3Center = rigidBody->GetCenterLocal();
	m_fSize = largestDimension * 2.0f;
	m_v3Min = m_v3Center - vector3(largestDimension);
	m_v3Max = m_v3Center + vector3(largestDimension);

	//Delete the rigidbody
	SafeDelete(rigidBody);

	//Start constructing the tree
	ConstructTree(a_nMaxLevel);
}

//Given a center point and size, construct a new octant
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();

	//Setup initial variables
	m_uID = m_uOctantCount;
	m_uOctantCount++;
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	//Calculate the min and max
	vector3 minMax = vector3(m_fSize) / 2.0f;
	m_v3Min = m_v3Center - minMax;
	m_v3Max = m_v3Center + minMax;

	//Setup child pointers
	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}

//Create a new octant given another octant
MyOctant::MyOctant(MyOctant const & other)
{
	//Copy the pointers and variables of the other octant into this one
	m_pMeshMngr = other.m_pMeshMngr;
	m_pEntityMngr = other.m_pEntityMngr;

	m_pRoot = other.m_pRoot;
	m_pParent = other.m_pParent;
	m_uChildren = other.m_uChildren;

	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}

	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_uMaxLevel = other.m_uMaxLevel;

	m_fSize = other.m_fSize;
	m_v3Min = other.m_v3Min;
	m_v3Center = other.m_v3Center;
	m_v3Max = other.m_v3Max;
}

//Set this octant equal to another octant
MyOctant& MyOctant::operator=(MyOctant const & other)
{
	//Release this octant, create a new one (based off of the other octant), swap the values of this octant and the new octant and return it
	Release();
	MyOctant octant = MyOctant(other);
	Swap(octant);
	return *this;
}

//Release all allocated data
MyOctant::~MyOctant(void)
{
	Release();
}

//Swap the data between this octant and another octant
void MyOctant::Swap(MyOctant & other)
{
	std::swap(m_pMeshMngr, other.m_pMeshMngr);
	std::swap(m_pEntityMngr, other.m_pEntityMngr);

	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_pParent, other.m_pParent);
	std::swap(m_uChildren, other.m_uChildren);

	for (uint i = 0; i < other.m_uChildren; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}

	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uMaxLevel, other.m_uMaxLevel);

	std::swap(m_fSize, other.m_fSize);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Max, other.m_v3Max);
}

//Return the octant's size
float MyOctant::GetSize(void)
{
	return m_fSize;
}

//Return the octant's global center vector
vector3 MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

//Return the octant's global minimum vector
vector3 MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

//Return the octant's global maximum vector
vector3 MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

//Check to see if an object is colliding with this octant
bool MyOctant::IsColliding(uint a_uRBIndex)
{
	//If the octant doesn't have a rigidbody yet, create one given it's min and max
	if (octantBody == nullptr)
	{
		std::vector<vector3> minMax;
		minMax.push_back(m_v3Min);
		minMax.push_back(m_v3Max);
		octantBody = new MyRigidBody(minMax);
	}

	//Check to see if there is a collision between the octant's rigidbody and the given entity's rigidbody
	return octantBody->IsColliding(m_pEntityMngr->GetRigidBody(a_uRBIndex));
}

//Display a particular octant
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	//If the index is -1, display all octants starting with the root
	if (a_nIndex == -1)
	{
		m_pRoot->Display(a_v3Color);
	}
	//If this octant's id matches the one we want to display
	else if (a_nIndex == m_uID)
	{
		//Draw this octant, and return
		matrix4 wireCube = glm::translate(IDENTITY_M4, m_v3Center);
		wireCube = glm::scale(wireCube, vector3(m_fSize));
		m_pMeshMngr->AddWireCubeToRenderList(wireCube, a_v3Color, RENDER_WIRE);
		return;
	}
	//If we haven't yet found the octant to display
	else
	{
		//Search through the children to find the octant with the matching id
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->Display(a_nIndex, a_v3Color);
		}
	}
}

//Fully display an octant including all of its children
void MyOctant::Display(vector3 a_v3Color)
{
	//If the octant has no entities in it and is a leaf (also if it's not the root)
	if (m_EntityList.size() == 0 && IsLeaf() && m_pRoot != this)
	{
		//Don't draw this octant because it provides no meaningful information
		return;
	}
	//Draw this octant as a wireframe cube using the given color
	matrix4 wireCube = glm::translate(IDENTITY_M4, m_v3Center);
	wireCube = glm::scale(wireCube, vector3(m_fSize));
	m_pMeshMngr->AddWireCubeToRenderList(wireCube, a_v3Color, RENDER_WIRE);

	//Draw the octant's children
	DisplayLeafs(a_v3Color);
}

//Iterate over this octant's children and draw them
void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	//Loop through all children and draw them using the given color
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}
}

//Clear the octant's entity list
void MyOctant::ClearEntityList(void)
{
	//Becuase it's an std::vector, it's as easy as calling the clear method
	m_EntityList.clear();

	//Also make each child octant clear their entity lists as well
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
}

//Create 8 new child octants if it makes sense to do so
void MyOctant::Subdivide(void)
{
	//If we've hit the max level or this octant already has children, return early
	if (m_uLevel >= m_uMaxLevel || m_uChildren == 8)
	{
		return;
	}

	//Set this octant's children count
	m_uChildren = 8;

	//Get the size and center of the first child octant
	float newSize = m_fSize / 2.0f;
	vector3 newCenter = m_v3Center - vector3(newSize / 2.0f);

	//Loop over and create 8 child octants
	for (uint i = 0; i < 8; i++)
	{
		//If it's the second or sixth octant, change the z
		if (i % 4 == 1)
		{
			newCenter.z += newSize;
		}
		//If it's the third or seventh octant, change the x
		else if (i % 4 == 2)
		{
			newCenter.x += newSize;
		}
		//If it's the fourth or either octant, change the z again
		else if (i % 4 == 3)
		{
			newCenter.z -= newSize;
		}
		//If it's the fourth octant, change the y and reset the x
		else if(i == 4)
		{
			newCenter.y += newSize;
			newCenter.x -= newSize;
		}

		//Create a new octant given the calculated center and size
		m_pChild[i] = new MyOctant(newCenter, newSize);

		//Set the child octant's root, parent, and level variables
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;

		//If the new child octant contians more than the ideal entity count, subdivide the child octant
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}

//Return the child octant at the given index
MyOctant * MyOctant::GetChild(uint a_nChild)
{
	//If the index is less than this octant's child count, return the specified child
	if (a_nChild < m_uChildren)
	{
		return m_pChild[a_nChild];
	}
	//Otherwise return a null pointer
	else
	{
		return nullptr;
	}
}

//Return this octant's parent octant
MyOctant * MyOctant::GetParent(void)
{
	return m_pParent;
}

//Return whether or not this octant is a leaf
bool MyOctant::IsLeaf(void)
{
	return m_uChildren == 0;
}

//Determine if this octant contains more than the given number of entities
bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	//Keep track of the entity count and iterate over all of the entities in the entity manager
	uint entityCount = 0;
	for (uint i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
	{
		//If the entity is colliding with the octant
		if (IsColliding(i))
		{
			//Increment the entity count
			entityCount++;
			//If the entity count has exceeded the specified count
			if (entityCount > a_nEntities)
			{
				return true;
			}
		}
	}

	//The number of entities in this octant does not exceed the given value
	return false;
}

//Destroy the branches of this octant
void MyOctant::KillBranches(void)
{
	//For all children octants
	for (uint i = 0; i < m_uChildren; i++)
	{
		//Ask the children to kill their branches, then delete the child, and the pointer to the child
		m_pChild[i]->KillBranches();
		SafeDelete(m_pChild[i]);
		m_pChild[i] = nullptr;
	}

	//All children have been removed
	m_uChildren = 0;
}

//Build the octree
void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//If the max level is 0, just return early, we only need one octant
	if (a_nMaxLevel == 0)
	{
		return;
	}

	//Clear the entity list and kill all current branches (children)
	ClearEntityList();
	KillBranches();

	//If this octant contains more than the ideal entity count, subdivide it into 8 additional child octants
	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}

	//Add all of the entities in the entity manager to a dimension (octant)
	AssignIDtoEntity();
}

//Add all entities to the smallest dimension (octant) that they occupy
void MyOctant::AssignIDtoEntity(void)
{
	//Loop over the child octants and ask them to assign their entity's dimensions first
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}

	//If this octant is a leaf
	if (IsLeaf())
	{
		//Iterate over every entity in the entity manager
		for (uint i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
		{
			//If the entity is colliding with this octant
			if (IsColliding(i))
			{
				//Push the entity into the entity list and add this dimension (octant) to the entity (for collision checks in the entity manager)
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

//Return the number of octants present in this octree
uint MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

//Release all allocated memory
void MyOctant::Release(void)
{
	//Kill this octant's branches, clear its entity list, delete the octant's rigidbody and set that pointer to null
	KillBranches();
	ClearEntityList();
	SafeDelete(octantBody);
	octantBody = nullptr;

	//Set other pointers to null
	m_pRoot = nullptr;
	m_pParent = nullptr;
	m_pEntityMngr = nullptr;
	m_pMeshMngr = nullptr;
}

//Grab the entity manager and mesh manager for later use
void MyOctant::Init(void)
{
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();
}

