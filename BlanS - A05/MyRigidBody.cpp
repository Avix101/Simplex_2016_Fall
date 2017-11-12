#include "MyRigidBody.h"
using namespace Simplex;
//Allocation
void MyRigidBody::Init(void)
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_bVisibleBS = false;
	m_bVisibleOBB = true;
	m_bVisibleARBB = false;

	m_fRadius = 0.0f;

	m_v3ColorColliding = C_RED;
	m_v3ColorNotColliding = C_WHITE;

	m_v3Center = ZERO_V3;
	m_v3MinL = ZERO_V3;
	m_v3MaxL = ZERO_V3;

	m_v3MinG = ZERO_V3;
	m_v3MaxG = ZERO_V3;

	m_v3HalfWidth = ZERO_V3;
	m_v3ARBBSize = ZERO_V3;

	m_m4ToWorld = IDENTITY_M4;
}
void MyRigidBody::Swap(MyRigidBody& a_pOther)
{
	std::swap(m_pMeshMngr, a_pOther.m_pMeshMngr);
	std::swap(m_bVisibleBS, a_pOther.m_bVisibleBS);
	std::swap(m_bVisibleOBB, a_pOther.m_bVisibleOBB);
	std::swap(m_bVisibleARBB, a_pOther.m_bVisibleARBB);

	std::swap(m_fRadius, a_pOther.m_fRadius);

	std::swap(m_v3ColorColliding, a_pOther.m_v3ColorColliding);
	std::swap(m_v3ColorNotColliding, a_pOther.m_v3ColorNotColliding);

	std::swap(m_v3Center, a_pOther.m_v3Center);
	std::swap(m_v3MinL, a_pOther.m_v3MinL);
	std::swap(m_v3MaxL, a_pOther.m_v3MaxL);

	std::swap(m_v3MinG, a_pOther.m_v3MinG);
	std::swap(m_v3MaxG, a_pOther.m_v3MaxG);

	std::swap(m_v3HalfWidth, a_pOther.m_v3HalfWidth);
	std::swap(m_v3ARBBSize, a_pOther.m_v3ARBBSize);

	std::swap(m_m4ToWorld, a_pOther.m_m4ToWorld);

	std::swap(m_CollidingRBSet, a_pOther.m_CollidingRBSet);
}
void MyRigidBody::Release(void)
{
	m_pMeshMngr = nullptr;
	ClearCollidingList();
}
//Accessors
bool MyRigidBody::GetVisibleBS(void) { return m_bVisibleBS; }
void MyRigidBody::SetVisibleBS(bool a_bVisible) { m_bVisibleBS = a_bVisible; }
bool MyRigidBody::GetVisibleOBB(void) { return m_bVisibleOBB; }
void MyRigidBody::SetVisibleOBB(bool a_bVisible) { m_bVisibleOBB = a_bVisible; }
bool MyRigidBody::GetVisibleARBB(void) { return m_bVisibleARBB; }
void MyRigidBody::SetVisibleARBB(bool a_bVisible) { m_bVisibleARBB = a_bVisible; }
float MyRigidBody::GetRadius(void) { return m_fRadius; }
vector3 MyRigidBody::GetColorColliding(void) { return m_v3ColorColliding; }
vector3 MyRigidBody::GetColorNotColliding(void) { return m_v3ColorNotColliding; }
void MyRigidBody::SetColorColliding(vector3 a_v3Color) { m_v3ColorColliding = a_v3Color; }
void MyRigidBody::SetColorNotColliding(vector3 a_v3Color) { m_v3ColorNotColliding = a_v3Color; }
vector3 MyRigidBody::GetCenterLocal(void) { return m_v3Center; }
vector3 MyRigidBody::GetMinLocal(void) { return m_v3MinL; }
vector3 MyRigidBody::GetMaxLocal(void) { return m_v3MaxL; }
vector3 MyRigidBody::GetCenterGlobal(void){	return vector3(m_m4ToWorld * vector4(m_v3Center, 1.0f)); }
vector3 MyRigidBody::GetMinGlobal(void) { return m_v3MinG; }
vector3 MyRigidBody::GetMaxGlobal(void) { return m_v3MaxG; }
vector3 MyRigidBody::GetHalfWidth(void) { return m_v3HalfWidth; }
matrix4 MyRigidBody::GetModelMatrix(void) { return m_m4ToWorld; }
void MyRigidBody::SetModelMatrix(matrix4 a_m4ModelMatrix)
{
	//to save some calculations if the model matrix is the same there is nothing to do here
	if (a_m4ModelMatrix == m_m4ToWorld)
		return;

	//Assign the model matrix
	m_m4ToWorld = a_m4ModelMatrix;

	//Calculate the 8 corners of the cube
	vector3 v3Corner[8];
	//Back square
	v3Corner[0] = m_v3MinL;
	v3Corner[1] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MinL.z);
	v3Corner[2] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MinL.z);
	v3Corner[3] = vector3(m_v3MaxL.x, m_v3MaxL.y, m_v3MinL.z);

	//Front square
	v3Corner[4] = vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Corner[5] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Corner[6] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MaxL.z);
	v3Corner[7] = m_v3MaxL;

	//Place them in world space
	for (uint uIndex = 0; uIndex < 8; ++uIndex)
	{
		v3Corner[uIndex] = vector3(m_m4ToWorld * vector4(v3Corner[uIndex], 1.0f));
	}

	//Identify the max and min as the first corner
	m_v3MaxG = m_v3MinG = v3Corner[0];

	//get the new max and min for the global box
	for (uint i = 1; i < 8; ++i)
	{
		if (m_v3MaxG.x < v3Corner[i].x) m_v3MaxG.x = v3Corner[i].x;
		else if (m_v3MinG.x > v3Corner[i].x) m_v3MinG.x = v3Corner[i].x;

		if (m_v3MaxG.y < v3Corner[i].y) m_v3MaxG.y = v3Corner[i].y;
		else if (m_v3MinG.y > v3Corner[i].y) m_v3MinG.y = v3Corner[i].y;

		if (m_v3MaxG.z < v3Corner[i].z) m_v3MaxG.z = v3Corner[i].z;
		else if (m_v3MinG.z > v3Corner[i].z) m_v3MinG.z = v3Corner[i].z;
	}

	//we calculate the distance between min and max vectors
	m_v3ARBBSize = m_v3MaxG - m_v3MinG;
}
//The big 3
MyRigidBody::MyRigidBody(std::vector<vector3> a_pointList)
{
	Init();
	//Count the points of the incoming list
	uint uVertexCount = a_pointList.size();

	//If there are none just return, we have no information to create the BS from
	if (uVertexCount == 0)
		return;

	//Max and min as the first vector of the list
	m_v3MaxL = m_v3MinL = a_pointList[0];

	//Get the max and min out of the list
	for (uint i = 1; i < uVertexCount; ++i)
	{
		if (m_v3MaxL.x < a_pointList[i].x) m_v3MaxL.x = a_pointList[i].x;
		else if (m_v3MinL.x > a_pointList[i].x) m_v3MinL.x = a_pointList[i].x;

		if (m_v3MaxL.y < a_pointList[i].y) m_v3MaxL.y = a_pointList[i].y;
		else if (m_v3MinL.y > a_pointList[i].y) m_v3MinL.y = a_pointList[i].y;

		if (m_v3MaxL.z < a_pointList[i].z) m_v3MaxL.z = a_pointList[i].z;
		else if (m_v3MinL.z > a_pointList[i].z) m_v3MinL.z = a_pointList[i].z;
	}

	//with model matrix being the identity, local and global are the same
	m_v3MinG = m_v3MinL;
	m_v3MaxG = m_v3MaxL;

	//with the max and the min we calculate the center
	m_v3Center = (m_v3MaxL + m_v3MinL) / 2.0f;

	//we calculate the distance between min and max vectors
	m_v3HalfWidth = (m_v3MaxL - m_v3MinL) / 2.0f;

	//Get the distance between the center and either the min or the max
	m_fRadius = glm::distance(m_v3Center, m_v3MinL);
}
MyRigidBody::MyRigidBody(MyRigidBody const& a_pOther)
{
	m_pMeshMngr = a_pOther.m_pMeshMngr;

	m_bVisibleBS = a_pOther.m_bVisibleBS;
	m_bVisibleOBB = a_pOther.m_bVisibleOBB;
	m_bVisibleARBB = a_pOther.m_bVisibleARBB;

	m_fRadius = a_pOther.m_fRadius;

	m_v3ColorColliding = a_pOther.m_v3ColorColliding;
	m_v3ColorNotColliding = a_pOther.m_v3ColorNotColliding;

	m_v3Center = a_pOther.m_v3Center;
	m_v3MinL = a_pOther.m_v3MinL;
	m_v3MaxL = a_pOther.m_v3MaxL;

	m_v3MinG = a_pOther.m_v3MinG;
	m_v3MaxG = a_pOther.m_v3MaxG;

	m_v3HalfWidth = a_pOther.m_v3HalfWidth;
	m_v3ARBBSize = a_pOther.m_v3ARBBSize;

	m_m4ToWorld = a_pOther.m_m4ToWorld;

	m_CollidingRBSet = a_pOther.m_CollidingRBSet;
}
MyRigidBody& MyRigidBody::operator=(MyRigidBody const& a_pOther)
{
	if (this != &a_pOther)
	{
		Release();
		Init();
		MyRigidBody temp(a_pOther);
		Swap(temp);
	}
	return *this;
}
MyRigidBody::~MyRigidBody() { Release(); };
//--- a_pOther Methods
void MyRigidBody::AddCollisionWith(MyRigidBody* a_pOther)
{
	/*
		check if the object is already in the colliding set, if
		the object is already there return with no changes
	*/
	auto element = m_CollidingRBSet.find(a_pOther);
	if (element != m_CollidingRBSet.end())
		return;
	// we couldn't find the object so add it
	m_CollidingRBSet.insert(a_pOther);
}
void MyRigidBody::RemoveCollisionWith(MyRigidBody* a_pOther)
{
	m_CollidingRBSet.erase(a_pOther);
}
void MyRigidBody::ClearCollidingList(void)
{
	m_CollidingRBSet.clear();
}
bool MyRigidBody::IsColliding(MyRigidBody* const a_pOther)
{
	//check if spheres are colliding as pre-test
	bool bColliding = (glm::distance(GetCenterGlobal(), a_pOther->GetCenterGlobal()) < m_fRadius + a_pOther->m_fRadius);
	
	//if they are colliding check the SAT
	if (bColliding)
	{
		if(SAT(a_pOther) != eSATResults::SAT_NONE)
			bColliding = false;// reset to false
	}

	if (bColliding) //they are colliding
	{
		this->AddCollisionWith(a_pOther);
		a_pOther->AddCollisionWith(this);
	}
	else //they are not colliding
	{
		this->RemoveCollisionWith(a_pOther);
		a_pOther->RemoveCollisionWith(this);
	}

	return bColliding;
}
void MyRigidBody::AddToRenderList(void)
{
	if (m_bVisibleBS)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), C_BLUE_CORNFLOWER);
		else
			m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), C_BLUE_CORNFLOWER);
	}
	if (m_bVisibleOBB)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3ColorColliding);
		else
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3ColorNotColliding);
	}
	if (m_bVisibleARBB)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(m_v3ARBBSize), C_YELLOW);
		else
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(m_v3ARBBSize), C_YELLOW);
	}
}

uint MyRigidBody::SAT(MyRigidBody* const a_pOther)
{
	//Declare two arrays, one to hold this shape's 8 points (bounding box) and the other to hold the other shape's 8 points (bounding box)
	vector3 shapePoints[8];
	vector3 otherPoints[8];
	
	//Calculate all of the points in the shape's array
	shapePoints[0] = m_v3MinL;
	shapePoints[1] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MinL.z);
	shapePoints[2] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MinL.z);
	shapePoints[3] = vector3(m_v3MaxL.x, m_v3MaxL.y, m_v3MinL.z);
	shapePoints[4] = vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z);
	shapePoints[5] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MaxL.z);
	shapePoints[6] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MaxL.z);
	shapePoints[7] = m_v3MaxL;

	//Get the other shape's local min and max
	vector3 otherLocalMin = a_pOther->GetMinLocal();
	vector3 otherLocalMax = a_pOther->GetMaxLocal();

	//Calculate all of the points in the other shape's array
	otherPoints[0] = otherLocalMin;
	otherPoints[1] = vector3(otherLocalMax.x, otherLocalMin.y, otherLocalMin.z);
	otherPoints[2] = vector3(otherLocalMin.x, otherLocalMax.y, otherLocalMin.z);
	otherPoints[3] = vector3(otherLocalMax.x, otherLocalMax.y, otherLocalMin.z);
	otherPoints[4] = vector3(otherLocalMin.x, otherLocalMin.y, otherLocalMax.z);
	otherPoints[5] = vector3(otherLocalMax.x, otherLocalMin.y, otherLocalMax.z);
	otherPoints[6] = vector3(otherLocalMin.x, otherLocalMax.y, otherLocalMax.z);
	otherPoints[7] = otherLocalMax;

	//Place all of the points in world space for each shape (according to their model matricies)
	for (uint uIndex = 0; uIndex < 8; ++uIndex)
	{
		shapePoints[uIndex] = vector3(m_m4ToWorld * vector4(shapePoints[uIndex], 1.0f));
		otherPoints[uIndex] = vector3(a_pOther->GetModelMatrix() * vector4(otherPoints[uIndex], 1.0f));
	}

	//Declare an array to hold all 15 axes that we're checking
	vector3 axes[15];

	//Compute the shape's x, y, and z vectors using corner points on its bounding box
	axes[0] = shapePoints[1] - shapePoints[0]; //Ax axis
	axes[1] = shapePoints[2] - shapePoints[0]; //Ay axis
	axes[2] = shapePoints[4] - shapePoints[0]; //Az axis

	//Compute the other shape's x, y, and z vectors using corner points on its bounding box
	axes[3] = otherPoints[1] - otherPoints[0]; //Bx axis
	axes[4] = otherPoints[2] - otherPoints[0]; //By axis
	axes[5] = otherPoints[4] - otherPoints[0]; //Bz axis

	//Compute the remaining axes by finding the cross products of all of the combinations of each shape's local x, y, and z
	axes[6] = glm::cross(axes[0], axes[3]); //AxBx
	axes[7] = glm::cross(axes[0], axes[4]); //AxBy
	axes[8] = glm::cross(axes[0], axes[5]); //AxBz
	axes[9] = glm::cross(axes[1], axes[3]); //AyBx
	axes[10] = glm::cross(axes[1], axes[4]); //AyBy
	axes[11] = glm::cross(axes[1], axes[5]); //AyBz
	axes[12] = glm::cross(axes[2], axes[3]); //AzBx
	axes[13] = glm::cross(axes[2], axes[4]); //AzBy
	axes[14] = glm::cross(axes[2], axes[5]); //AzBz

	//Loop over all of the calculated axes
	for (uint i = 0; i < 15; i++)
	{
		//Grab the current axis
		vector3 axis = axes[i];

		//Declare each shape's min and max point projected onto this axis
		float shapeMin = glm::dot(axis, shapePoints[0]);
		float shapeMax = glm::dot(axis, shapePoints[0]);
		float otherMin = glm::dot(axis, otherPoints[0]);
		float otherMax = glm::dot(axis, otherPoints[0]);

		//Iterate through the remaining points in each shape's point list
		for (uint j = 1; j < 8; j++)
		{
			//Calculate that points' value projected onto the current axis
			float currentShapeValue = glm::dot(axis, shapePoints[j]);
			float currentOtherValue = glm::dot(axis, otherPoints[j]);

			//If the point's current projected value is less than the min, set the min to the current projected value
			if (currentShapeValue < shapeMin)
			{
				shapeMin = currentShapeValue;
			}
			//Same for the max value
			else if (currentShapeValue > shapeMax)
			{
				shapeMax = currentShapeValue;
			}

			//Do the same check for the other shape's min and max
			if (currentOtherValue < otherMin)
			{
				otherMin = currentOtherValue;
			}
			else if (currentOtherValue > otherMax)
			{
				otherMax = currentOtherValue;
			}
		}

		//If the shape's max is less than the other shape's min or if the other shape's max is less than this shape's min
		if (shapeMax < otherMin || otherMax < shapeMin)
		{
			//We've found a separating axis, so return it!
			//Also note that I've set up the axis array's indicies to line up exactly with the eSATResults enumeration (if you add 1)
			//So to return the separating axis, the loop variable i (+1) just needs to be converted into the enum equivalent
			eSATResults result = static_cast<eSATResults>(i + 1);
			return result;
		}
	}

	//there is no axis test that separates this two objects
	return eSATResults::SAT_NONE;
}