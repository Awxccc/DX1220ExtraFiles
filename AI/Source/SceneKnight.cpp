#include "SceneKnight.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneKnight::SceneKnight()
//zero initialize every data member
	: m_speed{},
	m_worldWidth{},
	m_worldHeight{},
	m_objectCount{},
	m_noGrid{},
	m_gridSize{},
	m_gridOffset{},
	m_grid{},
	m_numTours{},
	m_move{},
	m_call{}
{
}

SceneKnight::~SceneKnight()
{
}

void SceneKnight::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	m_speed = 1.f;

	Math::InitRNG();

	m_objectCount = 0;
	m_noGrid = 8;	// Change to 5 if you want to speed up the processing
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	bStop = false;

	//m_grid holds the num of moves that the knight has taken to reach each cell
	//m_grid[0] == 1 means that the knight started out at the lower left corner of the board
	//m_grid[8] == 2 means that the knight's next move is to cell #8
	//note: we're using a 1d array to represent a 2d board
	m_grid.resize(m_noGrid * m_noGrid, -1);
	m_grid_results.resize(m_noGrid * m_noGrid, -1);
	m_numTours = m_move = m_call = 0;
	DFS(0); //start at index 0
}

void SceneKnight::Update(double dt)
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
		//Exercise: Implement Reset button
		//DO IT YOURSELF
		bStop = false;
		std::fill(m_grid.begin(), m_grid.end(), -1);
		std::fill(m_grid_results.begin(), m_grid_results.end(), -1);
		m_move = 0;
		m_move_results = 0;
		m_call = 0;
		int iStartINdex = rand() % (m_noGrid * m_noGrid);
		std::cout << "\n********\nReset and restart the DFS at : " << iStartINdex << std::endl;
		DFS(iStartINdex); //start at index rand() % 64
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

void SceneKnight::Render()
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

	//Print knights: optional
	if (m_move_results > 0)
	{
		//draw path
		int i{ 1 };
		int tileX{}, tileX2{};
		int tileY{}, tileY2{};
		std::vector<int>::iterator it{};

		//draws the path the knight took
		while (i <= m_move_results)
		{
			//for better performance, cache the sequence of indices instead of performing a linear search every draw
			it = std::find(m_grid_results.begin(), m_grid_results.end(), i);

			// If the lastPosIndex is beyond the size of this grid, then break the loop
			if ((int)(it - m_grid_results.begin()) >= m_noGrid * m_noGrid)
				break;
			
			//find index to knight's position.
			//(iterator - container.begin()) returns the number of element from beginning of container
			// 1. it contains the value of the last grid drawn in the path above; it has the value of 64
			// 2. it - m_grid_results.begin() gives you the index value of the value 64 in the vector<>
			int lastPosIndex = it - m_grid_results.begin();
			tileX2 = lastPosIndex % m_noGrid;
			tileY2 = lastPosIndex / m_noGrid;
			float dist{ static_cast<float>(sqrt((tileX2 - tileX) * (tileX2 - tileX) + (tileY2 - tileY) * (tileY2 - tileY))) };

			modelStack.PushMatrix();
			modelStack.Translate(tileX * m_gridSize + m_gridOffset, tileY * m_gridSize + m_gridOffset, 0.f);
			modelStack.Rotate(Math::RadianToDegree(atan2(tileY2 - tileY, tileX2 - tileX)), 0.f, 0.f, 1.f);
			modelStack.Scale(dist * m_gridSize, dist * m_gridSize, 1.f);
			RenderMesh(meshList[GEO_LINE], false);
			modelStack.PopMatrix();

			tileX = tileX2;
			tileY = tileY2;

			++i;
		}

		// Use the index to knight's last position to render its texture.
		modelStack.PushMatrix();
		modelStack.Translate(tileX * m_gridSize + m_gridOffset, tileY * m_gridSize + m_gridOffset, 0.f);
		modelStack.Scale(m_gridSize, m_gridSize, 1.f);
		RenderMesh(meshList[GEO_KNIGHT], false);
		modelStack.PopMatrix();
	}

	//On screen text
	std::ostringstream ss;

	//ss << "Something" << something;
	ss << "Num tours: " << m_numTours;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	//ss << "Something" << something;
	ss << "Calls (DFS):" << m_call;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	//ss << "Something" << something;
	//ss << "Completed moves = " << m_move;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	RenderTextOnScreen(meshList[GEO_TEXT], "Knight's Tour", Color(0, 1, 0), 3, 50, 0);
}

void SceneKnight::Exit()
{
	SceneBase::Exit();
}

void SceneKnight::PrintTour()
{
	//print the knight's path
	std::cout << "Printing the Tour: " << std::endl;
	std::cout << "\tNum tours: " << m_numTours << std::endl;
	std::cout << "\tCalls (DFS):" << m_call << std::endl << std::endl;
	//for (int index = 0; index < m_grid.size(); index++)
	for (int iRow = (m_noGrid - 1); iRow >= 0; iRow--)
	{
		std::cout << "\t";
		for (int iCol = 0; iCol < m_noGrid; iCol++)
		{
			std::cout << (iRow * m_noGrid + iCol) << "[" << m_grid[iRow * m_noGrid + iCol] << "] ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void SceneKnight::DFS(int index)
{
	if (bStop)
	{
		//backtracking
		m_grid[index] = -1; //undo move
		--m_move;
		return;
	}

	//for better performance, consider implementing DFS iteratively rather than recursively
	++m_call;
	if (m_grid[index] != -1)
		return;
	++m_move;

	// Every 1000000 calls to DFS(), we print out and render to the OpenGL window
	if (m_call % 1000000 == 0)
	{
		std::cout << "Num tours: " << m_numTours << std::endl;
		std::cout << "Calls (DFS):" << m_call << std::endl;
		std::cout << "Completed moves = " << m_move << std::endl;
		// Copy the results from m_grid to m_grid_results
		for (int index = 0; index < m_grid.size(); index++)
			m_grid_results[index] = m_grid[index];
		m_move_results = m_move;
		Application::GetInstance().Iterate(); //draw to screen
	}

	m_grid[index] = m_move;
	if (m_move == m_noGrid * m_noGrid) //knight has made final move
	{
		++m_numTours;
		bStop = true;

		// Copy the results from m_grid to m_grid_results
		for (int index = 0; index < m_grid.size(); index++)
			m_grid_results[index] = m_grid[index];
		m_move_results = m_move;

		// Print the tour
		PrintTour();

		//Application::GetInstance().Iterate(); //draw to screen to visualize the search (warning: makes the search excruciatingly slow)
	}
	else
	{
		static int offsets[][2] = { {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}, {1, -2}, {2, -1} };
		for (int* offset : offsets) //loops through each offset in the above offsets
		{
			//convert from index to tileX and tileY, then add the offsets
			int tX = (index % m_noGrid) + offset[0];
			int tY = (index / m_noGrid) + offset[1];

			//ensure move is within boundary of the board
			if (tX < 0 || tX >= m_noGrid ||
				tY < 0 || tY >= m_noGrid)
				continue;

			//explore next move
			DFS(tY * m_noGrid + tX); //convert from tileX and tileY back to index
		}
	}
	//backtracking
	m_grid[index] = -1; //undo move
	--m_move;

	if ((bStop == false) && (Application::IsKeyPressed('S')))
	{
		bStop = true;
		std::cout << "*** Stop key (S) pressed. Stopping the DFS now ***" << std::endl;
	}
}
