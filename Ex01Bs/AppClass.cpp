#include "AppClass.h"
void Application::InitVariables(void)
{
	////Change this to your name and email
	m_sProgrammer = "Alberto Bobadilla - labigm@rit.edu";

	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUp(vector3(10.0f, 0.0f, 10.0f), ZERO_V3, AXIS_Y);
	
	//init the mesh
	m_pMesh = new MyMesh();
	m_pMesh->GenerateCone(0.5f, 1.0f, 7, C_RED);

	m_stopsList.push_back(vector3(0.0f, 0.0f, 0.0f));
	m_stopsList.push_back(vector3(1.0f, 1.0f, 0.0f));
	m_stopsList.push_back(vector3(3.0f, -1.0f, 0.0f));
	m_stopsList.push_back(vector3(5.0f, 1.0f, 0.0f));
	m_stopsList.push_back(vector3(7.0f, -1.0f, 0.0f));
	m_stopsList.push_back(vector3(-1.0f, 2.0f, 0.0f));
	
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

	//calculate the current position
	static float fTime = 0.0f;
	static float fTime2 = 0.0f;
	static uint uClock = m_pSystem->GenClock();
	static uint nRoute = 0;
	float fDelta = m_pSystem->GetDeltaTime(uClock);
	fTime += fDelta;
	fTime2 += fDelta;

	vector3 v3Current;
	if (nRoute < m_stopsList.size() -1)
		v3Current = glm::lerp(m_stopsList[nRoute], m_stopsList[nRoute + 1], fTime);
	else
		v3Current = glm::lerp(m_stopsList[m_stopsList.size() -1 ], m_stopsList[0], fTime);

	matrix4 m4Model = glm::translate(IDENTITY_M4, v3Current) * glm::rotate(IDENTITY_M4, fTime2 * 15.0f, AXIS_Z);

	if (fTime >= 1.0f)
	{
		fTime = 0.0f;
		++nRoute;
		nRoute %= m_stopsList.size();
	}
	

	// render the object
	m_pMesh->Render(m4Projection, m4View, m4Model);
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	//draw the center of the world
	for (uint i = 0; i < m_stopsList.size(); ++i)
	{
		m_pMeshMngr->AddSphereToRenderList(glm::translate(m_stopsList[i]) * glm::scale(vector3(0.1f)), C_RED, RENDER_WIRE);
	}
	

	//print the time on the screen
#pragma region Debugging Information
	//Print info on the screen
	uint nEmptyLines = 21;
	for (uint i = 0; i < nEmptyLines; ++i)
		m_pMeshMngr->PrintLine("");//Add a line on top
								   //m_pMeshMngr->Print("						");
	m_pMeshMngr->PrintLine(m_pSystem->GetAppName(), C_YELLOW);

	//m_pMeshMngr->Print("						");
	m_pMeshMngr->Print("FPS:");
	m_pMeshMngr->PrintLine(std::to_string(m_pSystem->GetFPS()), C_RED);
#pragma endregion
		
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
	//release the mesh
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}
