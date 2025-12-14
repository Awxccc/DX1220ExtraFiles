#include "SceneTicTacToe.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneTicTacToe::SceneTicTacToe()
{
}

SceneTicTacToe::~SceneTicTacToe()
{
}

void SceneTicTacToe::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	m_objectCount = 0;
	m_noGrid = 3;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_bCrossTurn = true;
	m_bGameOver = false;
	m_winner = WHO_CONTENT::WHO_NONE;
	m_numberOfMoves = 0;

	for (unsigned i = 0; i < 9; ++i)
	{
		GameObject *go = new GameObject(GameObject::GO_CROSS);
		m_goList.push_back(go);
	}

}

GameObject* SceneTicTacToe::FetchGO()
{
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
		{
			go->active = true;
			++m_objectCount;
			return go;
		}
	}
	std::cout << "Add 10 more" << std::endl;
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject *go = new GameObject(GameObject::GO_CROSS);
		m_goList.push_back(go);
	}
	std::cout << "Added" << std::endl;
	return FetchGO();
}

//Exercise: This function should return 0 to 8, i.e. AI player's decision to put circle on one of the grids
int SceneTicTacToe::GetAIDecision()
{
	// Try 5 times only. If unable to find a tile which is not used, then return -1
	for (int i = 0; i < 5; i++)
	{
		int iProposedIndex = Math::RandIntMinMax(0, 8);
		bool bResult = true;
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if ((go->active == true) && (go->id == iProposedIndex))
			{
				bResult = false;
				break;
			}
		}

		if (bResult)
			return iProposedIndex;
	}

	return -1;
}

void SceneTicTacToe::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	else if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}
	// If the player presses R, then reset the game
	if (Application::IsKeyPressed('R'))
	{
		// Exercise: Implement Reset button
		// Exercise: Reset the relevant variables to default values
		m_bCrossTurn = true;
		m_bGameOver = false;
		m_winner = WHO_CONTENT::WHO_NONE;
		m_numberOfMoves = 0;

		// Exercise: Use loops to reset the active status of the game objects in m_goList
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active)
			{
				go->active = false;
			}
		}
		// Exercise: Use loops to reset m_grid array to WHO_NONE
		for (int i = 0; i < 9; i++)
		{
			m_grid[i] = WHO_NONE;
		}
	}

	// Don't proceed if the game is over
	if (m_bGameOver == true)
		return;

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
		//Exercise: Game inputs by the player
		if (m_bCrossTurn)
		{
			// Get the tile indices
			int tileX = static_cast<int>(posX) / static_cast<int>(m_gridSize);
			int tileY = static_cast<int>(posY) / static_cast<int>(m_gridSize);
			// Get a new GameObject instance
			GameObject* go = FetchGO();
			// Set the screen-space position
			go->pos.Set(tileX * m_gridSize + m_gridOffset, tileY * m_gridSize + m_gridOffset, 0.0f);
			// Set the screen-space scale
			go->scale.Set(m_gridSize, m_gridSize, 0.0f);
			// Set this GameObject to GO_CROSS
			go->type = GameObject::GO_CROSS;
			// Update the grid ID for the game object
			go->id = tileY * m_noGrid + tileX;
			// Set this GameObject to WHO_CROSS
			m_grid[go->id] = WHO_CONTENT::WHO_CROSS;
			// Set this GameObject to active
			go->active = true;
			// Change the turn to the circle
			m_bCrossTurn = !m_bCrossTurn;
			// Increase moves by 1
			m_numberOfMoves++;
		}
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

	if (!m_bCrossTurn)
	{
		//Exercise: Call GetAIDecision() and put circle on grid
		int index = GetAIDecision();
		if (index >= 0)
		{
			// Get the tile indices
			int tileY = index / m_noGrid;
			int tileX = index - tileY * m_noGrid;
			// Get a new GameObject instance
			GameObject* go = FetchGO();
			// Set the screen-space position
			go->pos.Set(tileX * m_gridSize + m_gridOffset, tileY * m_gridSize + m_gridOffset, 0.0f);
			// Set the screen-space scale
			go->scale.Set(m_gridSize, m_gridSize, 0.0f);
			// Set this GameObject to GO_CIRCLE
			go->type = GameObject::GO_CIRCLE;
			// Update the grid ID for the game object
			go->id = tileY * m_noGrid + tileX;
			// Update the grid for the AI
			m_grid[go->id] = WHO_CONTENT::WHO_CIRCLE;
			// Set this GameObject to active
			go->active = true;
			// Change the turn to the circle
			m_bCrossTurn = !m_bCrossTurn;
			// Increase moves by 1
			m_numberOfMoves++;
		}
	}

	// Game Logic Section
	// Exercise: Check draw, cross wins or circle wins
	// Check if a row or a column has a clear winner
	for (int i = 0; i < 3; ++i)
	{
		// Check a row
		if ((m_grid[i * 3 + 0] != WHO_NONE) &&
			(m_grid[i * 3 + 0] == m_grid[i * 3 + 1] && m_grid[i * 3 + 0] == m_grid[i * 3 + 2]))
		{
			m_bGameOver = true;
			m_winner = m_grid[i * 3 + 0];
			break;
		}
		// Check a column
		if ((m_grid[0 * 3 + i] != WHO_NONE) &&
			(m_grid[0 * 3 + i] == m_grid[1 * 3 + i] && m_grid[0 * 3 + i] == m_grid[2 * 3 + i]))
		{
			m_bGameOver = true;
			m_winner = m_grid[0 * 3 + i];
			break;
		}
	}

	// Check diagonals
	if ((m_grid[0] != WHO_NONE) &&
		(m_grid[0] == m_grid[4] && m_grid[0] == m_grid[8]))
	{
		m_bGameOver = true;
		m_winner = m_grid[0];
	}
	if ((m_grid[2] != WHO_NONE) &&
		(m_grid[2] == m_grid[4] && m_grid[2] == m_grid[6]))
	{
		m_bGameOver = true;
		m_winner = m_grid[2];
	}

	// Check if all grids have been filled up
	if (m_numberOfMoves == 9)
	{
		m_bGameOver = true;
		PrintSelf();
	}
}

void SceneTicTacToe::RenderGO(GameObject *go)
{
	switch (go->type)
	{
	case GameObject::GO_CROSS:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_CROSS], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_CIRCLE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_CIRCLE], false);
		modelStack.PopMatrix();
		break;
	}
}

void SceneTicTacToe::Render()
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
	RenderMesh(meshList[GEO_TICTACTOE], false);
	modelStack.PopMatrix();

	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (go->active)
		{
			RenderGO(go);
		}
	}

	//On screen text
	static std::string winnerText[] = {"Draw", "Cross wins", "Circle wins"};

	std::ostringstream ss;
	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Count:" << m_objectCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Turn:" << (m_bCrossTurn ? "Cross" : "Circle");
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 12);

	if (m_bGameOver)
	{
		ss.str("");
		ss << winnerText[static_cast<int>(m_winner)];
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 15);
	}

	RenderTextOnScreen(meshList[GEO_TEXT], "Tic Tac Toe (R to reset)", Color(0, 1, 0), 3, 50, 0);
}

void SceneTicTacToe::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if (m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}

void SceneTicTacToe::PrintSelf()
{
	// Check if a row or a column has a clear winner
	std::cout << "Col/Row\t0\t1\t2" << std::endl;
	for (int i = 2; i >= 0; i--)
	{
		std::cout << i << "\t";
		for (int j = 0; j < 3; j++)
		{
			std::cout << m_grid[i * 3 + j] << "\t";
		}
		std::cout << std::endl;
	}
	std::cout << "========================" << std::endl;
}
