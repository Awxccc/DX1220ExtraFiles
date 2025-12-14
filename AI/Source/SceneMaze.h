#ifndef SCENE_MAZE_H
#define SCENE_MAZE_H

#include "GameObject.h"
#include <vector>
#include <queue>
#include "SceneBase.h"
#include "Maze.h"

class SceneMaze : public SceneBase
{
public:
	SceneMaze();
	~SceneMaze();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);
	GameObject* FetchGO();

protected:
	int IsWithinBoundary(int x) const;
	int Get1DIndex(int x, int y) const;

	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

	// Exercise Week 07
	void DFS(const MazePt& curr);
	bool BFS(const MazePt& start, const MazePt& end);
	Maze m_maze;
	MazePt m_start;
	MazePt m_end;
	std::vector<Maze::TILE_CONTENT> m_myGrid; //read maze and store here
	std::vector<bool> m_visited; //visited set for DFS/BFS
	std::queue<MazePt> m_queue; //queue for BFS
	std::vector<MazePt> m_previous; //to store previous tile
	std::vector<MazePt> m_shortestPath; //to store shortest path
	unsigned m_mazeKey;
};

#endif