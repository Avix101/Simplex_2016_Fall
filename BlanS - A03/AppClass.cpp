#include "AppClass.h"
void Application::InitVariables(void)
{
	////Change this to your name and email
	//m_sProgrammer = "Alberto Bobadilla - labigm@rit.edu";

	////Alberto needed this at this position for software recording.
	//m_pWindow->setPosition(sf::Vector2i(710, 0));
	
	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUp(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

//if the background is cornflowerblue change it to black (its easier to see)
if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
{
	m_v4ClearColor = vector4(ZERO_V3, 1.0f);
}

//if there are no segments create 7
if (m_uOrbits < 1)
	m_uOrbits = 7;

float fSize = 1.0f; //initial size of orbits
float fRadius = 0.95f; //radius of stop points on the first orbit

//creating a color using the spectrum 
uint uColor = 650; //650 is Red
//prevent division by 0
float decrements = 250.0f / (m_uOrbits > 1 ? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)
/*
	This part will create the orbits, it start at 3 because that is the minimum subdivisions a torus can have
*/
uint uSides = 3; //start with the minimal 3 sides
float fRadiansInACircle = 2 * PI; //Note how many radians are in a circle

//Iterate over all of the orbits
for (uint i = uSides; i < m_uOrbits + uSides; i++)
{
	//Create a new stop list and set the angle that we use to 0
	std::vector<vector3> stopList = std::vector<vector3>();
	float fAngle = 0;

	//Iterate over all of the sides (stop points) of the current orbit
	for (uint j = 0; j < i; j++)
	{
		//Generate the x and y coordinates of the stop point using the angle and radius(z never changes);
		float x = cos(fAngle) * fRadius;
		float y = sin(fAngle) * fRadius;
		float z = 0.0f;

		//Push back a new point into the current stop list
		stopList.push_back(vector3(x, y, z));

		//Increment the angle (radians in a circle divided by the number of stop points in the orbit); 
		fAngle += fRadiansInACircle / (float) i;
	}

	//Add the new stop list into the list of stop lists, and push back a 0 to the currentStops list to account for the new orbit
	m_stopLists.push_back(stopList);
	m_currentStops.push_back(0);

	vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength
	m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager
	fSize += 0.5f; //increment the size for the next orbit
	fRadius += 0.5f; //increment the radius of stop points for the next orbit
	uColor -= static_cast<uint>(decrements); //decrease the wavelength
}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system
	/*
		The following offset will orient the orbits as in the demo, start without it to make your life easier.
	*/
	m4Offset = glm::rotate(IDENTITY_M4, 90.0f, AXIS_Z);

	// draw a shapes
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 90.0f, AXIS_X));

		//Grab the current stop from the currentStops list and calculate the next stop (wrap if necessary)
		uint currentStop = m_currentStops[i];
		uint nextStop = (currentStop + 1) % m_stopLists[i].size();

		//Calculate the current position by lerping from the current orbit's current stop and next stop,
		//using the same lerp progress variable for all orbits
		vector3 v3CurrentPos = glm::lerp(m_stopLists[i][currentStop], m_stopLists[i][nextStop], m_fLerpProgress);

		//Generate the m4Model matrix by translating the m4Offset by the current position
		matrix4 m4Model = glm::translate(m4Offset, v3CurrentPos);

		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.1)), C_WHITE);
	}

	//Increment the orbits' lerp progress by 0.02f
	m_fLerpProgress += 0.02f;

	//If the lerp progress is greater than or equal to 1 (maximum value)
	if (m_fLerpProgress >= 1.0f)
	{
		//Set the lerp progress to 0.0f (if we didn't set it to 0.0f right when it hit 1.0f, the spheres would spend two frames on each stop)
		m_fLerpProgress = 0.0f;

		//Iterate over the currentStops list
		for (uint i = 0; i < m_currentStops.size(); i++)
		{
			//Advance all of the current stops by 1, and wrap if necessary
			m_currentStops[i] = (m_currentStops[i] + 1) % m_stopLists[i].size();
		}
	}

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//release GUI
	ShutdownGUI();
}