#include "AppClass.h"
void Application::InitVariables(void)
{
	////Change this to your name and email
	//m_sProgrammer = "Stasha Blank - sob3966@rit.edu";

	////Alberto needed this at this position for software recording.
	//m_pWindow->setPosition(sf::Vector2i(710, 0));
	
	//We need 46 blocks for the alien
	blockCount = 46;

	//Initialize the blocks and locations arrays
	blocks = new MyMesh*[blockCount];
	locations = new vector3[blockCount];

	//Easy way to make the alien: character array to blocks and spaces in a grid
	char shape[] = "--*-----*--#\
				   ---*---*---#\
				   --*******--#\
				   -**-***-**-#\
				   ***********#\
				   *-*******-*#\
				   *-*-----*-*#\
				   ---**-**---#";

	//Initial grid location values
	float x = -10.0f;
	float y = 7.0f;
	float z = -5.0f;
	int blocksIndex = 0;

	//Iterate over the characters
	for (int i = 0; i < strlen(shape); i++)
	{
		//A hyphen indicates a space, so move x over 1.0f
		if (shape[i] == '-')
		{
			x += 1.0f;
		}
		//A pound symbol indicates a vertical break, so move x back to its initial value (-10.0f), and decrement y by 1.0f
		else if (shape[i] == '#')
		{
			x = -10.0f;
			y -= 1.0f;
		}
		//A asterisk indicates a block
		else if (shape[i] == '*')
		{
			//Make a new block, generate it's cube, and calculate it's initial location in space
			blocks[blocksIndex] = new MyMesh();
			blocks[blocksIndex]->GenerateCube(1.0f, C_BLACK);
			locations[blocksIndex] = vector3(x, y, z);

			//Increment the block index and increment x by 1.0f
			blocksIndex++;
			x += 1.0f;
		}
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

	//Get the camera's view and projection matrix
	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	
	//Static variables to manage the alien's x and y
	static float alienX = 0.0f;
	static float alienY = 0.0f;

	//Iterate over all of the stored blocks
	for (int i = 0; i < blockCount; i++)
	{
		//Grab the location of the current block
		vector3 location = locations[i];

		//Create the model matrix by using glm::translate on the initial location plus the static x and y variables
		matrix4 m4Model = glm::translate(IDENTITY_M4, vector3(location.x + alienX, location.y + alienY, location.z));

		//Render the current block
		blocks[i]->Render(m4Projection, m4View, m4Model);
	}

	//Increment alienX by 0.02f per call to this method, and set alienY to sin(alienX)
	alienX += 0.02f;
	alienY = sin(alienX);
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
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
	//Delete each block in the blocks array
	for (int i = 0; i < blockCount; i++)
	{
		delete blocks[i];
	}

	//Clean up memory for the blocks and locations arrays
	delete[] blocks;
	delete[] locations;

	//release GUI
	ShutdownGUI();
}