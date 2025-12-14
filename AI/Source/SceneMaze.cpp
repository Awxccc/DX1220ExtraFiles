#include "SceneMaze.h"
#include "GL\glew.h"
#include "Application.h"
#include <stack>
#include <sstream>

//ugly hack: do not follow.
extern int gSleepDuration; //declare existence of global var defined in Maze.cpp

SceneMaze::SceneMaze()
	: m_goList{}, m_speed{}, m_worldWidth{},
	  m_worldHeight{}, m_objectCount{}, m_noGrid{}, 
	  m_gridSize{}, m_gridOffset{}, m_maze{}, 
	  m_start{}, m_end{}, m_myGrid{}, 
	  m_visited{}, m_queue{}, m_previous{}, 
	  m_shortestPath{}, m_mazeKey{}
{
}

SceneMaze::~SceneMaze()
{
}

void SceneMaze::Init()
{
	SceneBase::Init();
	bLightEnabled = true;

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();
	
	m_noGrid = 12;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	// Exercise Week 07
	m_start.Set(0, 0);									 //set agent's starting position
	m_mazeKey = 0;										 //seed for maze generation
	m_maze.Generate(m_mazeKey, m_noGrid, m_start, 0.3f); //generate a maze using the provided function
	m_myGrid.resize(m_noGrid * m_noGrid);				 //this is the maze from the agent's POV. an unexplored maze is just FOG to the agent
	m_visited.resize(m_noGrid * m_noGrid);
	m_previous.resize(m_noGrid * m_noGrid);				 //this is used by BFS to retrace the found path from endpt to startpoint
	std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
	std::fill(m_visited.begin(), m_visited.end(), false);
	m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY; //assume first cell is empty(guaranteed by m_maze.Generate)
	DFS(m_start);
}

GameObject* SceneMaze::FetchGO()
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
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject *go = new GameObject(GameObject::GO_CROSS);
		m_goList.push_back(go);
	}
	return FetchGO();
}

void SceneMaze::Update(double dt)
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
		
		// Exercise Week 07 - turn mouse click into end point and run BFS
		m_start = m_maze.GetCurr();
		m_end.Set(Math::Clamp(static_cast<int>(posX / m_gridSize), 0, m_noGrid - 1), 
				  Math::Clamp(static_cast<int>(posY / m_gridSize), 0, m_noGrid - 1));
		BFS(m_start, m_end);
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
		if (gSleepDuration == 100)
			gSleepDuration = 0;
		else
			gSleepDuration = 100;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}
}


void SceneMaze::RenderGO(GameObject *go)
{
	switch (go->type)
	{
	case GameObject::GO_NONE:
		break;
	}
}

void SceneMaze::Render()
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

	// Render the maze
	size_t size = m_myGrid.size();
	for (size_t i=0; i<size; ++i)
	{
		//convert from 1d index to 2d indices
		size_t tX = i % m_noGrid;
		size_t tY = i / m_noGrid;

		//calc position of tile using tX and tY
		modelStack.PushMatrix();
		modelStack.Translate(tX * m_gridSize + m_gridOffset, tY * m_gridSize + m_gridOffset, 0.f);
		modelStack.Scale(m_gridSize, m_gridSize, 1.f);

		// Exercise Week 07 - Draw floor
		modelStack.PushMatrix();
		modelStack.Translate(0.f, 0.f, -1.f); //draw floor beneath other stuff
		RenderMesh(meshList[GEO_FLOOR], false);
		modelStack.PopMatrix();

		if (m_myGrid[i] == Maze::TILE_CONTENT::TILE_WALL)
		{
			// Exercise Week 07 - Draw wall
			RenderMesh(meshList[GEO_WALL], false);
		}
		else if (m_myGrid[i] == Maze::TILE_CONTENT::TILE_FOG)
		{
			// Exercise Week 07 - Draw fog
			meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.f, 0.f, 0.f); //framework-specific code: set quad to black color
			RenderMesh(meshList[GEO_WHITEQUAD], true);
		}
		//let's color visited cells to make visualization easier when running BFS
		else if (m_shortestPath.empty() && m_visited[i] && !m_queue.empty())
		{
			// Exercise Week 07 - Draw floor with green tint
			meshList[GEO_FLOOR]->material.kAmbient.Set(0.2f, 0.8f, 0.2f); //framework-specific code: set quad to green color
			RenderMesh(meshList[GEO_FLOOR], true);
		}
		modelStack.PopMatrix();
	}

	MazePt currPos{ m_maze.GetCurr() };
	//render shortest path
	if (currPos.x == m_end.x && currPos.y == m_end.y && //check if shortest path found
		!m_shortestPath.empty())
	{
		for (size_t i = 0; i < m_shortestPath.size() - 1; ++i)
		{
			// Exercise Week 07 - For each MazePt in m_shortestPath, render a sprite
			const MazePt& pt = m_shortestPath[i];

			modelStack.PushMatrix();
			modelStack.Translate(pt.x * m_gridSize + m_gridOffset, pt.y * m_gridSize + m_gridOffset, 0.f);
			modelStack.Scale(m_gridSize, m_gridSize, 1.f);
			RenderMesh(meshList[GEO_WAYPOINT], false);
			modelStack.PopMatrix();

			//todo: cout the shortest path yourself for debugging purpose!
		}
	}

	// Exercise Week 07 - Render agent's current pos
	modelStack.PushMatrix();
	modelStack.Translate(currPos.x * m_gridSize + m_gridOffset, currPos.y * m_gridSize + m_gridOffset, 0.1f);
	modelStack.Scale(m_gridSize, m_gridSize, 1.f);
	RenderMesh(meshList[GEO_AGENT], false);
	modelStack.PopMatrix();

	//On screen text
	std::ostringstream ss;
	ss.str("");
	ss << "Press SPACE to speed up";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	// Exercise Week 07 - Render �Num Move� in the right panel using m_maze.GetNumMove()
	ss.str("");
	ss << "Num Move:" << m_maze.GetNumMove();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	// Exercise Week 07 - Render the maze key in the right panal e.g. �Maze 0� or �Maze 8�
	ss.str("");
	ss << "Maze " << m_maze.GetKey();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);
}

void SceneMaze::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
}

//helper function to convert from 2d indices to 1d index
int SceneMaze::Get1DIndex(int x, int y) const
{
	return y * m_noGrid + x;
}

//helper function to check if given index is within boundary
int SceneMaze::IsWithinBoundary(int x) const
{
	return x >= 0 && x < m_noGrid;
}

//week 7
//time for the agent to explore the maze
//we're mimicking how in Micromouse, a robot would first explore the given maze
//to build a image of the environment, before being tasked to solve the maze in the shortest time possible
//when the agent finishes exploring, it should end up in the starting spot
void SceneMaze::DFS(const MazePt& curr)
{
	m_visited[Get1DIndex(curr.x, curr.y)] = true; //mark current cell as visited

	// Exercise Week 07 - Add this code to help real time rendering
	Application::GetInstance().Iterate();
	Sleep(100);

	// Exercise Week 07 - MOVE UP
	//attempt to move up. check if such move is within boundary.
	//in addition, movement is only possible if the next cell is unvisited
	//unless it's on a return trip, this agent will attempt to make each step into uncharted ground
	int nextIndex = Get1DIndex(curr.x, curr.y + 1);
	if (IsWithinBoundary(curr.y + 1) && !m_visited[nextIndex])
	{
		if (m_maze.Move(Maze::DIRECTION::DIR_UP)) //move agent up
		{
			m_myGrid[nextIndex] = Maze::TILE_CONTENT::TILE_EMPTY;
			DFS(MazePt{ curr.x, curr.y + 1 });
			m_maze.Move(Maze::DIRECTION::DIR_DOWN); //return trip
		}
		else //wall up ahead, can't move to cell
		{
			m_myGrid[nextIndex] = Maze::TILE_CONTENT::TILE_WALL;
		}
	}

	// Exercise Week 07 - MOVE DOWN
	nextIndex = Get1DIndex(curr.x, curr.y - 1);
	if (IsWithinBoundary(curr.y - 1) && !m_visited[nextIndex])
	{
		if (m_maze.Move(Maze::DIRECTION::DIR_DOWN))
		{
			m_myGrid[nextIndex] = Maze::TILE_CONTENT::TILE_EMPTY;
			DFS(MazePt{ curr.x, curr.y - 1 });
			m_maze.Move(Maze::DIRECTION::DIR_UP); //return trip
		}
		else //wall up ahead, can't move to cell
		{
			m_myGrid[nextIndex] = Maze::TILE_CONTENT::TILE_WALL;
		}
	}

	// Exercise Week 07 - MOVE LEFT
	nextIndex = Get1DIndex(curr.x - 1, curr.y);
	if (IsWithinBoundary(curr.x - 1) && !m_visited[nextIndex])
	{
		if (m_maze.Move(Maze::DIRECTION::DIR_LEFT))
		{
			m_myGrid[nextIndex] = Maze::TILE_CONTENT::TILE_EMPTY;
			DFS(MazePt{ curr.x - 1, curr.y });
			m_maze.Move(Maze::DIRECTION::DIR_RIGHT); //return trip
		}
		else //wall up ahead, can't move to cell
		{
			m_myGrid[nextIndex] = Maze::TILE_CONTENT::TILE_WALL;
		}
	}

	// Exercise Week 07 - MOVE RIGHT
	nextIndex = Get1DIndex(curr.x + 1, curr.y);
	if (IsWithinBoundary(curr.x + 1) && !m_visited[nextIndex])
	{
		if (m_maze.Move(Maze::DIRECTION::DIR_RIGHT))
		{
			m_myGrid[nextIndex] = Maze::TILE_CONTENT::TILE_EMPTY;
			DFS(MazePt{ curr.x + 1, curr.y });
			m_maze.Move(Maze::DIRECTION::DIR_LEFT); //return trip
		}
		else //wall up ahead, can't move to cell
		{
			m_myGrid[nextIndex] = Maze::TILE_CONTENT::TILE_WALL;
		}
	}
}

//week 7
//we use BFS to perform pathfinding. It's very brute-force and 
//is not exactly performant. we will explore the A* algorithm
//in future lessons.
bool SceneMaze::BFS(const MazePt& start, const MazePt& end)
{
	//prepare variables
	std::fill(m_visited.begin(), m_visited.end(), false);
	//each slot in m_previous is supposed to store where the agent came from
	//for example, if m_previous[3] == 2, it means that agent reached cell #3 from cell #2
	std::fill(m_previous.begin(), m_previous.end(), MazePt{-1, -1});
	m_shortestPath.clear();
	m_queue = {}; //clear the queue

	//push start into queue
	m_queue.push(start);
	//we marked any node we pushed into the queue as visited
	m_visited[Get1DIndex(start.x, start.y)] = true;

	while (!m_queue.empty())
	{
		//expand front-most node in queue
		MazePt pt{ m_queue.front()};
		m_queue.pop();

		// Exercise Week 07 - Add this code to help real time rendering
		Application::GetInstance().Iterate();
		Sleep(100);

		m_maze.SetCurr(pt); //move agent to this node
		if (pt.x == end.x && pt.y == end.y) //check if destination reached
		{
			MazePt curr{ pt };
			//calc shortest path now that we know we've found the destination
			m_shortestPath.push_back(curr);
			while (curr.x != start.x || curr.y != start.y)
			{
				curr = m_previous[Get1DIndex(curr.x, curr.y)]; //find out where agent was before reaching curr
				m_shortestPath.insert(m_shortestPath.begin(), curr); //note: not performant to keep inserting at the front of a vector, but we'll close an eye
			}
			return true; //shortest path found
		}

		// Exercise Week 07
		//push neighbouring cells into queue for further expansion
		//CHECK UP
		int nextIndex = Get1DIndex(pt.x, pt.y + 1);
		if (IsWithinBoundary(pt.y + 1) && !m_visited[nextIndex] && 
			m_myGrid[nextIndex] == Maze::TILE_CONTENT::TILE_EMPTY)
		{
			//we marked any node we pushed into the queue as visited
			m_visited[nextIndex] = true;
			m_queue.push(MazePt{ pt.x, pt.y + 1 });
			//each time we push a new node into queue, we record its prior position
			m_previous[nextIndex] = pt;
		}

		// Exercise Week 07 - CHECK DOWN
		nextIndex = Get1DIndex(pt.x, pt.y - 1);
		if (IsWithinBoundary(pt.y - 1) && !m_visited[nextIndex] &&
			m_myGrid[nextIndex] == Maze::TILE_CONTENT::TILE_EMPTY)
		{
			//we marked any node we pushed into the queue as visited
			m_visited[nextIndex] = true;
			m_queue.push(MazePt{ pt.x, pt.y - 1 });
			//each time we push a new node into queue, we record its prior position
			m_previous[nextIndex] = pt;
		}

		// Exercise Week 07 - CHECK LEFT
		nextIndex = Get1DIndex(pt.x - 1, pt.y);
		if (IsWithinBoundary(pt.x - 1) && !m_visited[nextIndex] &&
			m_myGrid[nextIndex] == Maze::TILE_CONTENT::TILE_EMPTY)
		{
			//we marked any node we pushed into the queue as visited
			m_visited[nextIndex] = true;
			m_queue.push(MazePt{ pt.x - 1, pt.y });
			//each time we push a new node into queue, we record its prior position
			m_previous[nextIndex] = pt;
		}

		// Exercise Week 07 - CHECK RIGHT
		nextIndex = Get1DIndex(pt.x + 1, pt.y);
		if (IsWithinBoundary(pt.x + 1) && !m_visited[nextIndex] &&
			m_myGrid[nextIndex] == Maze::TILE_CONTENT::TILE_EMPTY)
		{
			//we marked any node we pushed into the queue as visited
			m_visited[nextIndex] = true;
			m_queue.push(MazePt{ pt.x + 1, pt.y });
			//each time we push a new node into queue, we record its prior position
			m_previous[nextIndex] = pt;
		}
	}

	m_maze.SetCurr(start); //no path found. place agent at start pt
	return false;
}