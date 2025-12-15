#pragma once
#include "GameObject.h"
#include <vector>
#include <map>
#include "SceneBase.h"
#include "ObjectBase.h"
#include "ConcreteMessages.h"
class SceneSandbox : public SceneBase, public ObjectBase
{
public:
	SceneSandbox();
	~SceneSandbox();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject* go);
	bool Handle(Message* message);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
	void SpawnUnit(MessageSpawnUnit::UNIT_TYPE unitType, Vector3 position, int teamID);
	std::vector<MazePt> FindPath(MazePt start, MazePt end);
protected:
	// Helper functions
	int IsWithinBoundary(int x) const;
	int Get1DIndex(int x, int y) const;
	void DetectNearbyEntities(GameObject* go);
	void FindNearestResource(GameObject* go);
	bool IsInTerritory(Vector3 pos, int teamID) const;
	void UpdateSpatialGrid();
	GameObject* GetNearestEnemy(Vector3 pos, int teamID, float maxRange);
	void FindNearestInjuredAlly(GameObject* go);

	// Game state
	std::vector<GameObject*> m_goList;
	std::map<int, std::vector<GameObject*>> m_spatialGrid;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

	std::vector<bool> m_wallGrid;
	std::vector<bool> m_foodGrid;
	bool IsGridOccupied(int gridX, int gridY);
	MazePt GetNearestVacantNeighbor(MazePt target, MazePt start);
	void SpawnTrail(GameObject* startObj, GameObject* endFood, int teamID);

	// Red Colony (Team 0)
	int m_redWorkerCount;
	int m_redSoldierCount;
	int m_redHealerCount;
	int m_redScoutCount;
	int m_redTankCount;
	int m_redResources;
	GameObject* m_redQueen;

	// Blue Colony (Team 1)
	int m_blueWorkerCount;
	int m_blueSoldierCount;
	int m_blueHealerCount;
	int m_blueScoutCount;
	int m_blueTankCount;
	int m_blueResources;
	GameObject* m_blueQueen;

	// Food resources
	std::vector<Vector3> m_foodLocations;

	// Simulation state
	float m_simulationTime;
	bool m_simulationEnded;
	int m_winner; // 0 = Red, 1 = Blue, 2 = Draw

	// Performance optimization
	float m_updateTimer;
	int m_updateCycle;
	bool m_coloniesDetected;
};

