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
protected:
	// Helper functions
	int IsWithinBoundary(int x) const;
	int Get1DIndex(int x, int y) const;
	void DetectNearbyEntities(GameObject* go);
	void FindNearestResource(GameObject* go);
	bool IsInTerritory(Vector3 pos, int teamID) const;
	void UpdateSpatialGrid();
	GameObject* GetNearestEnemy(Vector3 pos, int teamID, float maxRange);

	// Game state
	std::vector<GameObject*> m_goList;
	std::map<int, std::vector<GameObject*>> m_spatialGrid; // For optimization
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

	// Team statistics
	int m_speedyAntWorkerCount;
	int m_speedyAntSoldierCount;
	int m_strongAntWorkerCount;
	int m_strongAntWarriorCount;
	int m_speedyAntResources;
	int m_strongAntResources;
	GameObject* m_speedyAntQueen;
	GameObject* m_strongAntQueen;

	// Food resources
	std::vector<Vector3> m_foodLocations;

	// Simulation state
	float m_simulationTime;
	bool m_simulationEnded;
	int m_winner; // 0 = Ants, 1 = Beetles, 2 = Draw

	// Performance optimization
	float m_updateTimer;
	int m_updateCycle;
};

