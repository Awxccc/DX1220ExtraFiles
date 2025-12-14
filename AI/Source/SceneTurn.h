#ifndef SCENE_TURN_H
#define SCENE_TURN_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "Maze.h"
#include <queue>

class SceneTurn : public SceneBase
{
public:
	SceneTurn();
	~SceneTurn();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);
	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE);

	void DFS(MazePt curr);
	bool BFS(MazePt start, MazePt end);

protected:
	int IsWithinBoundary(int x) const;
	int Get1DIndex(int x, int y) const;

	// Exercise Week 8
	// 5.	Declare these methods and variable in SceneTurn.h


	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

	// Exercise Week 8
	// 5.	Declare these methods and variable in SceneTurn.h
	void DFSOnce(GameObject* go);
	bool BFSLimit(GameObject* go, MazePt end, int limit);
	int m_turn;

	Maze m_maze;
	MazePt m_start;
	MazePt m_end;
	std::vector<Maze::TILE_CONTENT> m_myGrid; //read maze and store here
	std::vector<bool> m_visited; //visited set for DFS/BFS
	std::queue<MazePt> m_queue; //queue for BFS
	std::vector<MazePt> m_previous; //to store previous tile
	std::vector<MazePt> m_shortestPath;  //to store shortest path
	unsigned m_mazeKey;
	float m_wallLoad;
};

#endif