#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//Top and bottom of the cone (center in the middle of the cone)
	vector3 top(0, a_fHeight / 2.0f, 0);
	vector3 bottom(0, -a_fHeight / 2.0f, 0);

	//Create an array to hold all of the ring points
	vector3* ringPoints = new vector3[a_nSubdivisions];

	//Run through all of the cone's subdivisions
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//Calculate the angle of this current subdivision
		float angle = (float) i * (radiansInCircle / (float) a_nSubdivisions);

		//Originally I had x = cos(angle) and z = sin(angle), but the cone rotated weirdly, and swapping the trig functions fixed it
		//Geometry is the same, the starting orientation is just slightly different
		//Calculate x and z values using the angle and radius (y is the same for all ring points)
		float x = sin(angle) * a_fRadius;
		float z = cos(angle) * a_fRadius;

		//Generate a new point from the calculated values, and put it in the array
		vector3 newPoint(x, -a_fHeight / 2.0f, z);
		ringPoints[i] = newPoint;

		//If this isn't the first ring point
		if (i > 0)
		{
			//Add two triangles connecting the current ring point and previous ring point to the top / bottom points
			AddTri(ringPoints[i - 1], ringPoints[i], top);
			AddTri(ringPoints[i], ringPoints[i - 1], bottom);

			//If we've generated all of the points, we still need to draw faces to connect the last points and first points
			if (i == a_nSubdivisions - 1)
			{
				AddTri(ringPoints[i], ringPoints[0], top);
				AddTri(ringPoints[0], ringPoints[i], bottom);
			}
		}
	}

	//Clean up memory!
	delete[] ringPoints;

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//Calculate the top and bottom points of the cylinder
	vector3 top(0, a_fHeight / 2.0f, 0);
	vector3 bottom(0, -a_fHeight / 2.0f, 0);

	//Create two arrays for points in the top and bottom rings
	vector3* upperRingPoints = new vector3[a_nSubdivisions];
	vector3* lowerRingPoints = new vector3[a_nSubdivisions];

	//Iterate over all of the subdivisions
	for (int i = 0; i < a_nSubdivisions; i++) 
	{
		//Calculate angle, x, and z
		float angle = (float) i * (radiansInCircle / (float) a_nSubdivisions);
		float x = cos(angle) * a_fRadius;
		float z = sin(angle) * a_fRadius;

		//Generate upper and lower ring points (they only differ in height: y value)
		vector3 newUpperPoint(x, a_fHeight / 2.0f, z);
		vector3 newLowerPoint(x, -a_fHeight / 2.0f, z);

		//Load points into their respective arrays
		upperRingPoints[i] = newUpperPoint;
		lowerRingPoints[i] = newLowerPoint;

		//Start connecting faces after the first set of points has been calculated
		if (i > 0)
		{
			//Connect the current ring points and previous ring points to each other (quad), and to the top / bottom points
			AddTri(upperRingPoints[i], upperRingPoints[i - 1], top);
			AddTri(lowerRingPoints[i - 1], lowerRingPoints[i], bottom);
			AddQuad(lowerRingPoints[i], lowerRingPoints[i - 1], upperRingPoints[i], upperRingPoints[i - 1]);

			//If all points have been calculated, clean up and draw the faces to connect the last set of points to the first
			if (i == a_nSubdivisions - 1)
			{
				AddTri(upperRingPoints[0], upperRingPoints[i], top);
				AddTri(lowerRingPoints[i], lowerRingPoints[0], bottom);
				AddQuad(lowerRingPoints[0], lowerRingPoints[i], upperRingPoints[0], upperRingPoints[i]);
			}
		}
	}

	//Clean up memory!
	delete[] upperRingPoints;
	delete[] lowerRingPoints;

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//No need to calculate a top / bottom point, but we still need the y values
	float upperY = a_fHeight / 2.0f;
	float lowerY = -a_fHeight / 2.0f;

	//Create 4 arrays: combos of inner / outer and upper / bottom points
	vector3* upperInnerRingPoints = new vector3[a_nSubdivisions];
	vector3* upperOuterRingPoints = new vector3[a_nSubdivisions];
	vector3* lowerInnerRingPoints = new vector3[a_nSubdivisions];
	vector3* lowerOuterRingPoints = new vector3[a_nSubdivisions];

	//Iterate over all subdivisions
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//Calculate the angle, inner & outer X, inner & outer Z
		float angle = (float) i * (radiansInCircle / (float) a_nSubdivisions);
		float innerX = cos(angle) * a_fInnerRadius;
		float outerX = cos(angle) * a_fOuterRadius;
		float innerZ = sin(angle) * a_fInnerRadius;
		float outerZ = sin(angle) * a_fOuterRadius;

		//Create 4 points from the calculated values, and put them in their respective array
		upperInnerRingPoints[i] = vector3(innerX, upperY, innerZ);
		upperOuterRingPoints[i] = vector3(outerX, upperY, outerZ);
		lowerInnerRingPoints[i] = vector3(innerX, lowerY, innerZ);
		lowerOuterRingPoints[i] = vector3(outerX, lowerY, outerZ);

		//After the first set of points
		if (i > 0)
		{
			//Add 4 quads: outer panel, inner panel, top panel, and bottom panel of the subdivision
			AddQuad(lowerOuterRingPoints[i], lowerOuterRingPoints[i - 1], upperOuterRingPoints[i], upperOuterRingPoints[i - 1]);
			AddQuad(lowerInnerRingPoints[i - 1], lowerInnerRingPoints[i], upperInnerRingPoints[i - 1], upperInnerRingPoints[i]);
			AddQuad(upperOuterRingPoints[i], upperOuterRingPoints[i - 1], upperInnerRingPoints[i], upperInnerRingPoints[i - 1]);
			AddQuad(lowerOuterRingPoints[i - 1], lowerOuterRingPoints[i], lowerInnerRingPoints[i - 1], lowerInnerRingPoints[i]);

			//Clean up: draw faces connecting last set of points and first set of points
			if (i == a_nSubdivisions - 1)
			{
				AddQuad(lowerOuterRingPoints[0], lowerOuterRingPoints[i], upperOuterRingPoints[0], upperOuterRingPoints[i]);
				AddQuad(lowerInnerRingPoints[i], lowerInnerRingPoints[0], upperInnerRingPoints[i], upperInnerRingPoints[0]);
				AddQuad(upperOuterRingPoints[0], upperOuterRingPoints[i], upperInnerRingPoints[0], upperInnerRingPoints[i]);
				AddQuad(lowerOuterRingPoints[i], lowerOuterRingPoints[0], lowerInnerRingPoints[i], lowerInnerRingPoints[0]);
			}
		}
	}


	//Clean up memory!
	delete[] upperInnerRingPoints;
	delete[] upperOuterRingPoints;
	delete[] lowerInnerRingPoints;
	delete[] lowerOuterRingPoints;

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	//Array of pointers to torus rings
	vector3** torusRings = new vector3*[a_nSubdivisionsA];

	//First angle to manage angle of the torus subdivision
	float angleTheta = 0;

	//Iterate over the first count of subdivisions
	for (int i = 0; i < a_nSubdivisionsA; i++)
	{
		//Create a new array of points for the current ring on the torus and store it
		vector3* ringPoints = new vector3[a_nSubdivisionsB];
		torusRings[i] = ringPoints;

		//Second angle to manage the current angle on the current torus ring
		float anglePhi = 0;

		//Iterate over the second count of subdivisions
		for (int j = 0; j < a_nSubdivisionsB; j++)
		{
			//Calculate the radius of the torus ring, and the radius from the center of the torus to the center of the torus ring
			float ringRadius = (a_fOuterRadius - a_fInnerRadius) / 2.0f;
			float centerRadius = ringRadius + a_fInnerRadius;

			//Calculate the center of the current torus ring
			vector3 ringCenter(cos(angleTheta) * centerRadius, 0, sin(angleTheta) * centerRadius);

			//Calculate a point on the current torus ring using trig and the current torus ring's center point
			float x = (cos(angleTheta) * cos(anglePhi) * ringRadius) + ringCenter.x;
			float y = (sin(anglePhi) * ringRadius) + ringCenter.y;
			float z = (sin(angleTheta) * cos(anglePhi) * ringRadius) + ringCenter.z;

			//Create the point and load it into the current ring's array
			vector3 newPoint(x, y, z);
			ringPoints[j] = newPoint;

			//If we aren't dealing with the first set of points on the torus / current ring
			if (i > 0 && j > 0)
			{
				//Connect the current point and previous point on the current torus ring and previous ring
				AddQuad(torusRings[i][j], torusRings[i][j - 1], torusRings[i - 1][j], torusRings[i - 1][j - 1]);

				//Clean up and draw a face connecting the last and first points on the current and previous torus rings
				if (j == a_nSubdivisionsB - 1)
				{
					AddQuad(torusRings[i][0], torusRings[i][j], torusRings[i - 1][0], torusRings[i - 1][j]);
				}

				//Clean up and draw faces connecting the last and first torus rings
				if (i == a_nSubdivisionsA - 1)
				{
					AddQuad(torusRings[0][j], torusRings[0][j - 1], torusRings[i][j], torusRings[i][j - 1]);

					//Clean up and draw faces connecting the last and first points on the last and first torus rings
					if (j == a_nSubdivisionsB - 1)
					{
						AddQuad(torusRings[0][0], torusRings[0][j], torusRings[i][0], torusRings[i][j]);
					}
				}
			}

			//Increment second angle as we move around the current torus ring
			anglePhi += radiansInCircle / (float) a_nSubdivisionsB;
		}

		//Increment the first angle as we move to the next torus ring
		angleTheta += radiansInCircle / (float) a_nSubdivisionsA;
	}

	//Clean up memory (in sub arrays and then main array)
	for (int i = 0; i < a_nSubdivisionsA; i++)
	{
		delete[] torusRings[i];
	}

	delete[] torusRings;

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	//Calculate the top and bottom points on the sphere
	vector3 top(0, a_fRadius, 0);
	vector3 bottom(0, -a_fRadius, 0);
	
	//Create an array of sphere rings
	vector3** sphereRings = new vector3*[a_nSubdivisions];

	//First angle to move to the next sphere ring
	float angleTheta = radiansInCircle / (float) a_nSubdivisions / 2.0f;

	//Iterate over subdivisions
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//Create a new array of ring points and put it in the main array
		vector3* ringPoints = new vector3[a_nSubdivisions];
		sphereRings[i] = ringPoints;

		//Second angle to move to next point on the current sphere ring
		float anglePhi = radiansInCircle / (float)a_nSubdivisions;

		//Iterate over subdivisions (this time for the current sphere ring)
		for (int j = 0; j < a_nSubdivisions; j++)
		{
			//Calculate the x, y, and z values of the current point on the current sphere ring
			float x = sin(angleTheta) * cos(anglePhi) * a_fRadius;
			float y = cos(angleTheta) * a_fRadius;
			float z = sin(angleTheta) * sin(anglePhi) * a_fRadius;
			
			//Generate the new point and put into the current sphere ring
			vector3 newPoint(x, y, z);
			ringPoints[j] = newPoint;

			//If this isn't the first point on the current sphere ring
			if (j > 0)
			{
				//If this is the first sphere ring
				if (i == 0)
				{
					//Draw a triangle from the current point to the previous point (on the current sphere ring) to the top
					AddTri(sphereRings[i][j], sphereRings[i][j - 1], top);

					//Clean up and draw a triangle from the first and last points on the current sphere ring to the top
					if (j == a_nSubdivisions - 1)
					{
						AddTri(sphereRings[i][0], sphereRings[i][j], top);
					}
				}
				//If this isn't the first sphere ring
				if (i > 0)
				{
					//Draw a quad between the current and previous points on the current and previous sphere rings
					AddQuad(sphereRings[i][j], sphereRings[i][j - 1], sphereRings[i - 1][j], sphereRings[i - 1][j - 1]);

					//Clean up and draw a quad between the first and last points on the current and previous sphere rings
					if (j == a_nSubdivisions - 1)
					{
						int currIndex = 0;
						int prevIndex = j;
						AddQuad(sphereRings[i][0], sphereRings[i][j], sphereRings[i - 1][0], sphereRings[i - 1][j]);
					}
				}

				//If this is the last sphere ring
				if (i == a_nSubdivisions - 1)
				{
					//Draw a triangle between the current and previous points on the current sphere ring and the bottom point
					AddTri(sphereRings[i][j - 1], sphereRings[i][j], bottom);

					//Clean up and draw a triangle between the first and last points on the current sphere ring and the bottom point
					if (j == a_nSubdivisions - 1)
					{
						AddTri(sphereRings[i][j], sphereRings[i][0], bottom);
					}
				}
			}

			//Increment the second angle as it moves around the current sphere ring
			anglePhi += radiansInCircle / (float) a_nSubdivisions;
		}

		//Increment the first angle as it moves to the next sphere ring
		angleTheta += radiansInCircle / (float) a_nSubdivisions / 2.0f;
	}

	//Clean up memory!
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		delete[] sphereRings[i];
	}

	delete[] sphereRings;

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}