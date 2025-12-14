#include "SceneQueen.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include <conio.h>

SceneQueen::SceneQueen()
//zero initialize every data member
	: m_speed{},
	m_worldWidth{},
	m_worldHeight{},
	m_objectCount{},
	m_noGrid{},
	m_gridSize{},
	m_gridOffset{},
	// Exercise Week 6
	m_queen{},
	m_numSolutions{},
	m_startGrid{}
{
}

SceneQueen::~SceneQueen()
{
}

void SceneQueen::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	m_speed = 1.f;

	Math::InitRNG();

	m_objectCount = 0;
	m_noGrid = 8;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	bStop = false;

	// Exercise Week 6
	//m_queen stores the column number [1-8] of a queen in its own row. m_queen[i] stores the column number for the queen on row i.
	m_queen.resize(m_noGrid, -1); //use -1 to indicate the row is empty
	m_numSolutions = 0;

	// Exercise Week 06
	m_startGrid = 0;

	DFS(m_startGrid); //start at m_startGrid
	std::cout << "Num solutions: " << m_numSolutions << std::endl;
}

void SceneQueen::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}
	if (Application::IsKeyPressed('R'))
	{
		// Exercise Week 06
		// Exercise: Implement Reset button
		m_numSolutions = 0;
		std::fill(m_queen.begin(), m_queen.end(), -1);

		bStop = false;
		//std::fill(m_grid_results.begin(), m_grid_results.end(), -1);

		m_startGrid = rand() % (m_noGrid * m_noGrid);
		std::cout << "\n********\nReset and restart the DFS at : " << m_startGrid << std::endl;
		DFS(m_startGrid); // Start at m_startGrid
	}

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * m_worldWidth;
		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;

		// Exercise Week 06
		int tileX = (int)((posX) / m_gridSize);
		int tileY = (int)((posY) / m_gridSize);
		// Calculate the m_startGrid
		m_startGrid = tileY * m_noGrid + tileX;

		// Reset the parameters
		m_numSolutions = 0;
		std::fill(m_queen.begin(), m_queen.end(), -1);

		//Exercise: Implement Reset button
		bStop = false;
		//std::fill(m_grid_results.begin(), m_grid_results.end(), -1);

		// Find from this m_startGrid grid
		std::cout << "\n********\nReset and restart the DFS at : " << m_startGrid << std::endl;
		DFS(m_startGrid);
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}
}

void SceneQueen::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
	);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	RenderMesh(meshList[GEO_AXES], false);

	modelStack.PushMatrix();
	modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
	modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_CHESSBOARD], false);
	modelStack.PopMatrix();

	//Print queens
	for (int i = 0; i < m_noGrid; ++i)
	{
		int tileX = m_queen[i] - 1;
		int tileY = i;

		modelStack.PushMatrix();
		modelStack.Translate(tileX * m_gridSize + m_gridOffset, tileY * m_gridSize + m_gridOffset, 0.f);
		modelStack.Scale(m_gridSize, m_gridSize, 1.f);
		RenderMesh(meshList[GEO_QUEEN], false);
		modelStack.PopMatrix();
	}

	//On screen text
	std::ostringstream ss;
	//ss << "Something" << something;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	//ss << "Something" << something;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	//ss << "Something" << something;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	RenderTextOnScreen(meshList[GEO_TEXT], "8 Queens", Color(0, 1, 0), 3, 50, 0);
}

void SceneQueen::Exit()
{
	SceneBase::Exit();
}

//print positions stored in m_queen
void SceneQueen::PrintSolution()
{
	// Exercise Week 06
	std::cout << "Solution: ";
	for (int pos : m_queen)
	{
		std::cout << pos << " ";
	}
	std::cout << std::endl;
}

//returns true if none of the current queens are in each other's way
bool SceneQueen::CheckQueen()
{
	//compare each queen with every other queens to see if 
	//they are in the way
	for (int i = 0; i < m_noGrid - 1; ++i)
	{
		int pos1[] = { m_queen[i], i + 1 }; //stores tileX, tileY position of queen
		if (pos1[0] < 0) break; //nothing is in this row, skip this

		for (int j = i + 1; j < m_noGrid; ++j)
		{
			int pos2[] = { m_queen[j], j + 1 }; //stores tileX, tileY position of other queen
			if (pos2[0] < 0) break; //nothing is in this row, skip this

			if (pos1[0] == pos2[0]) //check if queens are on the same col
				return false;

			int dx = pos2[0] - pos1[0]; //calc position relative to pos1
			int dy = pos2[1] - pos1[1]; //calc position relative to pos1
			//check for diagonals
			if (abs(dx) == abs(dy))
				return false;
		}
	}

	return true;
}

void SceneQueen::DFS(int row)
{
	//for each column
	for (int i = 1; i <= m_noGrid; ++i)
	{
		//set this queen on this spot
		m_queen[row] = i;
		Application::GetInstance().Iterate(); //draw to screen

		if (CheckQueen())
		{
			//check if we've reached the end of the board.
			//if so, print solution.
			//if not, move to the next row by recursively calling DFS(row+1)
			if ((bStop == false) && (row < m_noGrid - 1))
			{
				DFS(row + 1);
			}
			else
			{
				++m_numSolutions; //found 1 solution!
				PrintSolution();
			}
		}
		m_queen[row] = -1;

		if ((bStop == false) && (Application::IsKeyPressed('S')))
		{
			bStop = true;
			std::cout << "*** Stop key (S) pressed. Stopping the DFS now ***" << std::endl;
			break;
		}
	}
}
