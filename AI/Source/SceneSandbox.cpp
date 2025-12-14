#include "SceneSandbox.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include "StatesSandbox.h"
#include "SceneData.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

SceneSandbox::SceneSandbox()
	: m_goList{}, m_spatialGrid{}, m_speed{}, m_worldWidth{}, m_worldHeight{},
	m_noGrid{}, m_gridSize{}, m_gridOffset{}, m_speedyAntWorkerCount{}, m_speedyAntSoldierCount{},
	m_strongAntWorkerCount{}, m_strongAntWarriorCount{}, m_speedyAntResources{}, m_strongAntResources{},
	m_speedyAntQueen{}, m_strongAntQueen{}, m_foodLocations{}, m_simulationTime{},
	m_simulationEnded{}, m_winner{}, m_updateTimer{}, m_updateCycle{}, m_wallGrid{}
{
}

SceneSandbox::~SceneSandbox()
{
}

void SceneSandbox::Init()
{
	SceneBase::Init();
	bLightEnabled = false;

	// Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	// Physics code
	m_speed = 1.f;

	Math::InitRNG();

	// Grid setup - 30x30
	m_noGrid = 30;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_wallGrid.assign(m_noGrid * m_noGrid, false);

	// 1. Walls around Speedy Ant Colony
	for (int y = 0; y <= 7; ++y)
	{
		// Leave gap at y=3 and y=4 for entry/exit
		if (y != 3 && y != 4)
			m_wallGrid[Get1DIndex(7, y)] = true;
	}
	// Horizontal Wall at y=7
	for (int x = 0; x <= 7; ++x)
	{
		// Leave gap at x=3 and x=4
		if (x != 3 && x != 4)
			m_wallGrid[Get1DIndex(x, 7)] = true;
	}

	// 2. Walls around Strong Ant Colony
	for (int y = 22; y < m_noGrid; ++y)
	{
		// Leave gap at y=26 and y=27
		if (y != 26 && y != 27)
			m_wallGrid[Get1DIndex(22, y)] = true;
	}
	// Horizontal Wall at y=22
	for (int x = 22; x < m_noGrid; ++x)
	{
		// Leave gap at x=26 and x=27
		if (x != 26 && x != 27)
			m_wallGrid[Get1DIndex(x, 22)] = true;
	}

	SceneData::GetInstance()->SetObjectCount(0);
	SceneData::GetInstance()->SetFishCount(0);
	SceneData::GetInstance()->SetNumGrid(m_noGrid);
	SceneData::GetInstance()->SetGridSize(m_gridSize);
	SceneData::GetInstance()->SetGridOffset(m_gridOffset);

	// Register scene with post office
	PostOffice::GetInstance()->Register("Scene", this);

	// Initialize game state
	m_speedyAntWorkerCount = 0;
	m_speedyAntSoldierCount = 0;
	m_strongAntWorkerCount = 0;
	m_strongAntWarriorCount = 0;
	m_speedyAntResources = 0;
	m_strongAntResources = 0;
	m_simulationTime = 0.f;
	m_simulationEnded = false;
	m_winner = 2; // Draw by default
	m_updateTimer = 0.f;
	m_updateCycle = 0;

	// Spawn Speedy Ant Queen (bottom-left corner)
	m_speedyAntQueen = FetchGO(GameObject::GO_SPEEDY_ANT_QUEEN);
	m_speedyAntQueen->teamID = 0;
	m_speedyAntQueen->pos.Set(m_gridSize * 3.f, m_gridSize * 3.f, 0);
	m_speedyAntQueen->homeBase = m_speedyAntQueen->pos;
	m_speedyAntQueen->scale.Set(m_gridSize * 1.5f, m_gridSize * 1.5f, 1.f);
	m_speedyAntQueen->maxHealth = 50.f;
	m_speedyAntQueen->health = 50.f;
	m_speedyAntQueen->attackPower = 0.f;
	m_speedyAntQueen->moveSpeed = 0.f;
	m_speedyAntQueen->detectionRange = m_gridSize * 8.f;
	m_speedyAntQueen->attackRange = 0.f;

	// Spawn Strong Ant Queen (top-right corner)
	m_strongAntQueen = FetchGO(GameObject::GO_STRONG_ANT_QUEEN);
	m_strongAntQueen->teamID = 1;
	m_strongAntQueen->pos.Set(m_gridSize * (m_noGrid - 3.f), m_gridSize * (m_noGrid - 3.f), 0);
	m_strongAntQueen->homeBase = m_strongAntQueen->pos;
	m_strongAntQueen->scale.Set(m_gridSize * 1.5f, m_gridSize * 1.5f, 1.f);
	m_strongAntQueen->maxHealth = 50.f;
	m_strongAntQueen->health = 50.f;
	m_strongAntQueen->attackPower = 0.f;
	m_strongAntQueen->moveSpeed = 0.f;
	m_strongAntQueen->detectionRange = m_gridSize * 8.f;
	m_strongAntQueen->attackRange = 0.f;

	// Spawn initial workers for both teams
	for (int i = 0; i < 3; ++i)
	{
		// Speedy Ant workers
		SpawnUnit(MessageSpawnUnit::UNIT_SPEEDY_ANT_WORKER,
			m_speedyAntQueen->pos + Vector3(Math::RandFloatMinMax(-2, 2) * m_gridSize,
				Math::RandFloatMinMax(-2, 2) * m_gridSize, 0), 0);

		// Strong workers
		SpawnUnit(MessageSpawnUnit::UNIT_STRONG_ANT_WORKER,
			m_strongAntQueen->pos + Vector3(Math::RandFloatMinMax(-2, 2) * m_gridSize,
				Math::RandFloatMinMax(-2, 2) * m_gridSize, 0), 1);
	}

	// Spawn initial soldiers
	for (int i = 0; i < 2; ++i)
	{
		SpawnUnit(MessageSpawnUnit::UNIT_SPEEDY_ANT_SOLDIER,
			m_speedyAntQueen->pos + Vector3(Math::RandFloatMinMax(-3, 3) * m_gridSize,
				Math::RandFloatMinMax(-3, 3) * m_gridSize, 0), 0);

		SpawnUnit(MessageSpawnUnit::UNIT_STRONG_ANT_SOLDIER,
			m_strongAntQueen->pos + Vector3(Math::RandFloatMinMax(-3, 3) * m_gridSize,
				Math::RandFloatMinMax(-3, 3) * m_gridSize, 0), 1);
	}

	// Spawn food resources in center and various locations
	m_foodLocations.clear();
	int foodCount = Math::RandIntMinMax(15, 25);
	for (int i = 0; i < foodCount; ++i)
	{
		GameObject* food = FetchGO(GameObject::GO_FOOD);

		// More food in center
		float x, y;
		if (i < foodCount / 2)
		{
			// Central food zone
			x = Math::RandFloatMinMax(m_noGrid * 0.3f, m_noGrid * 0.7f) * m_gridSize;
			y = Math::RandFloatMinMax(m_noGrid * 0.3f, m_noGrid * 0.7f) * m_gridSize;
		}
		else
		{
			// Random scattered food
			x = Math::RandFloatMinMax(2, m_noGrid - 2) * m_gridSize;
			y = Math::RandFloatMinMax(2, m_noGrid - 2) * m_gridSize;
		}
		int gx = (int)(x / m_gridSize);
		int gy = (int)(y / m_gridSize);
		if (IsWithinBoundary(gx) && IsWithinBoundary(gy) && m_wallGrid[Get1DIndex(gx, gy)])
		{
			x += m_gridSize;
		}

		food->pos.Set(x, y, 0);
		food->scale.Set(m_gridSize * 0.8f, m_gridSize * 0.8f, 1.f);
		food->moveSpeed = 0.f;
		food->health = 1.f;
		m_foodLocations.push_back(food->pos);
	}

	std::cout << "=== Sandbox Simulation Started ===" << std::endl;
	std::cout << "Grid Size: " << m_noGrid << "x" << m_noGrid << std::endl;
	std::cout << "Food Resources: " << foodCount << std::endl;
	std::cout << "Speedy Ant Colony: Bottom-Left | Strong Ant Colony: Top-Right" << std::endl;
	std::cout << "Simulation will run for 5 minutes..." << std::endl;
}

GameObject* SceneSandbox::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active && go->type == type)
		{
			go->active = true;
			return go;
		}
	}

	// Create new game objects
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject* go = new GameObject(type);
		m_goList.push_back(go);
	}
	return FetchGO(type);
}

void SceneSandbox::SpawnUnit(MessageSpawnUnit::UNIT_TYPE unitType, Vector3 position, int teamID)
{
	GameObject* unit = nullptr;

	switch (unitType)
	{
		// --- COLONY 1: SPEEDY ANTS (Formerly Ant Worker/Soldier) ---
	case MessageSpawnUnit::UNIT_SPEEDY_ANT_WORKER:
		unit = FetchGO(GameObject::GO_SPEEDY_ANT_WORKER);
		unit->teamID = 0;
		unit->homeBase = m_speedyAntQueen->pos;
		unit->maxHealth = 5.f;     // Fragile
		unit->health = 5.f;
		unit->attackPower = 0.2f;  // Weak bite
		unit->moveSpeed = 6.f;     // FAST (Standard was 3)
		unit->detectionRange = m_gridSize * 7.f;
		unit->attackRange = m_gridSize * 0.8f;

		unit->sm = new StateMachine();
		unit->sm->AddState(new StateAntWorkerIdle("Idle", unit));
		unit->sm->AddState(new StateAntWorkerSearching("Searching", unit));
		unit->sm->AddState(new StateAntWorkerGathering("Gathering", unit));
		unit->sm->AddState(new StateAntWorkerFleeing("Fleeing", unit));
		unit->sm->SetNextState("Idle");
		m_speedyAntWorkerCount++;
		break;

	case MessageSpawnUnit::UNIT_SPEEDY_ANT_SOLDIER:
		unit = FetchGO(GameObject::GO_SPEEDY_ANT_SOLDIER);
		unit->teamID = 0;
		unit->homeBase = m_speedyAntQueen->pos;
		unit->maxHealth = 10.f;
		unit->health = 10.f;
		unit->attackPower = 1.5f; // Lower damage
		unit->moveSpeed = 8.f;    // VERY FAST (Standard was 4)
		unit->detectionRange = m_gridSize * 9.f;
		unit->attackRange = m_gridSize * 1.2f;

		unit->sm = new StateMachine();
		unit->sm->AddState(new StateSpeedyAntSoldierPatrolling("Patrolling", unit));
		unit->sm->AddState(new StateSpeedyAntSoldierAttacking("Attacking", unit));
		unit->sm->AddState(new StateSpeedyAntSoldierDefending("Defending", unit));
		unit->sm->AddState(new StateSpeedyAntSoldierRetreating("Retreating", unit));
		unit->sm->SetNextState("Patrolling");
		m_speedyAntSoldierCount++;
		break;

		// --- COLONY 2: STRONG ANTS ---
	case MessageSpawnUnit::UNIT_STRONG_ANT_WORKER:
		unit = FetchGO(GameObject::GO_STRONG_ANT_WORKER);
		unit->teamID = 1;
		unit->homeBase = m_strongAntQueen->pos;
		unit->maxHealth = 15.f;    // Tough worker
		unit->health = 15.f;
		unit->attackPower = 1.0f;  // Stronger bite
		unit->moveSpeed = 2.0f;    // SLOW (Standard was 2.5)
		unit->detectionRange = m_gridSize * 5.f;
		unit->attackRange = m_gridSize * 0.7f;

		unit->sm = new StateMachine();
		unit->sm->AddState(new StateStrongAntWorkerIdle("Idle", unit));
		unit->sm->AddState(new StateStrongAntWorkerForaging("Foraging", unit));
		unit->sm->AddState(new StateStrongAntWorkerCollecting("Collecting", unit));
		unit->sm->AddState(new StateStrongAntWorkerEscaping("Escaping", unit));
		unit->sm->SetNextState("Idle");
		m_strongAntWorkerCount++;
		break;

	case MessageSpawnUnit::UNIT_STRONG_ANT_SOLDIER:
		unit = FetchGO(GameObject::GO_STRONG_ANT_SOLDIER);
		unit->teamID = 1;
		unit->homeBase = m_strongAntQueen->pos;
		unit->maxHealth = 30.f;    // Tank
		unit->health = 30.f;
		unit->attackPower = 5.0f;  // Heavy Hitter
		unit->moveSpeed = 3.0f;    // SLOW (Standard was 5)
		unit->detectionRange = m_gridSize * 6.f;
		unit->attackRange = m_gridSize * 1.3f;

		unit->sm = new StateMachine();
		unit->sm->AddState(new StateStrongAntSoldierHunting("Hunting", unit));
		unit->sm->AddState(new StateStrongAntSoldierCombat("Combat", unit));
		unit->sm->AddState(new StateStrongAntSoldierResting("Resting", unit));
		unit->sm->AddState(new StateStrongAntSoldierWithdrawing("Withdrawing", unit));
		unit->sm->SetNextState("Hunting");
		m_strongAntWarriorCount++;
		break;
	}

	if (unit)
	{
		unit->pos = position;
		unit->target = position;
		unit->scale.Set(m_gridSize, m_gridSize, 1.f);
	}
}

void SceneSandbox::Update(double dt)
{
	SceneBase::Update(dt);

	// Update world dimensions
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	// Speed controls
	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}

	// Simulation time
	m_simulationTime += static_cast<float>(dt) * m_speed;

	// Check win conditions
	if (!m_simulationEnded)
	{
		// 5 minute timer
		if (m_simulationTime >= 300.f)
		{
			m_simulationEnded = true;
			// Determine winner based on resources and population
			int speedyAntTotal = m_speedyAntWorkerCount + m_speedyAntSoldierCount + m_speedyAntResources;
			int strongAntTotal = m_strongAntWorkerCount + m_strongAntWarriorCount + m_strongAntResources;

			if (speedyAntTotal > strongAntTotal)
				m_winner = 0;
			else if (strongAntTotal > speedyAntTotal)
				m_winner = 1;
			else
				m_winner = 2;

			std::cout << "\n=== SIMULATION ENDED ===" << std::endl;
			std::cout << "Winner: " << (m_winner == 0 ? "SPEEDY ANT COLONY" :
				m_winner == 1 ? "STRONG ANT COLONY" : "DRAW") << std::endl;
		}

		// Check if queens are dead
		if (!m_speedyAntQueen->active)
		{
			m_simulationEnded = true;
			m_winner = 1;
			std::cout << "\n=== SPEEDY ANT QUEEN ELIMINATED - STRONG ANTS WIN ===" << std::endl;
		}
		if (!m_strongAntQueen->active)
		{
			m_simulationEnded = true;
			m_winner = 0;
			std::cout << "\n=== STRONG ANT QUEEN ELIMINATED - SPEEDY ANTS WIN ===" << std::endl;
		}
	}

	// Update cycle for optimization (stagger updates)
	m_updateTimer += static_cast<float>(dt);
	if (m_updateTimer > 0.033f) // ~30 updates per second
	{
		m_updateTimer = 0.f;
		m_updateCycle = (m_updateCycle + 1) % 3;
		UpdateSpatialGrid();
	}

	// State machine updates
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;

		if (go->sm)
			go->sm->Update(dt * m_speed);
	}

	// Detection and interaction (optimized with spatial grid)
	int cycleCheck = 0;
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;

		// Stagger detection updates
		if ((cycleCheck % 3) == m_updateCycle)
		{
			DetectNearbyEntities(go);
			if (go->type == GameObject::GO_SPEEDY_ANT_WORKER || go->type == GameObject::GO_STRONG_ANT_WORKER)
			{
				FindNearestResource(go);
			}
		}
		cycleCheck++;
	}

	// Movement
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active || go->moveSpeed <= 0.f)
			continue;

		// 1. Calculate Vector to Target
		Vector3 toTarget = go->target - go->pos;
		float distToTarget = toTarget.Length();

		// 2. Identify Current Grid Center
		int gridX = static_cast<int>((go->pos.x - m_gridOffset + m_gridSize * 0.5f) / m_gridSize);
		int gridY = static_cast<int>((go->pos.y - m_gridOffset + m_gridSize * 0.5f) / m_gridSize);

		gridX = Math::Clamp(gridX, 0, m_noGrid - 1);
		gridY = Math::Clamp(gridY, 0, m_noGrid - 1);

		Vector3 tileCenter;
		tileCenter.x = gridX * m_gridSize + m_gridOffset;
		tileCenter.y = gridY * m_gridSize + m_gridOffset;
		tileCenter.z = go->pos.z;

		// 3. Move Logic
		float moveStep = go->moveSpeed * static_cast<float>(dt) * m_speed;
		float distToCenter = (go->pos - tileCenter).Length();

		// If we are at the tile center (or close enough)
		if (distToCenter < moveStep)
		{
			go->pos = tileCenter; // Snap to center

			if (distToTarget < m_gridSize * 0.5f)
			{
				go->pos = go->target; // Arrived at destination
			}
			else
			{
				// Manhattan Movement with Collision Avoidance
				Vector3 moveDir(0, 0, 0);
				int nextX = gridX;
				int nextY = gridY;

				bool preferX = (abs(toTarget.x) >= abs(toTarget.y));

				// Attempt Preferred Direction
				if (preferX) nextX += (toTarget.x > 0) ? 1 : -1;
				else nextY += (toTarget.y > 0) ? 1 : -1;

				bool blocked = false;
				if (IsWithinBoundary(nextX) && IsWithinBoundary(nextY))
				{
					if (m_wallGrid[Get1DIndex(nextX, nextY)]) blocked = true;
				}
				else blocked = true; // blocked by boundary (or stay in bounds)

				if (blocked)
				{
					// Try the alternate axis (Slide along wall)
					nextX = gridX;
					nextY = gridY;
					if (!preferX) nextX += (toTarget.x > 0) ? 1 : -1; // Switch to X
					else nextY += (toTarget.y > 0) ? 1 : -1; // Switch to Y

					if (IsWithinBoundary(nextX) && IsWithinBoundary(nextY) &&
						!m_wallGrid[Get1DIndex(nextX, nextY)])
					{
						// Alternate is free
						if (!preferX) moveDir.x = (toTarget.x > 0) ? 1.f : -1.f;
						else moveDir.y = (toTarget.y > 0) ? 1.f : -1.f;
					}
					else
					{
						// Both blocked: Stand still (moveDir remains 0)
					}
				}
				else
				{
					// Preferred is free
					if (preferX) moveDir.x = (toTarget.x > 0) ? 1.f : -1.f;
					else moveDir.y = (toTarget.y > 0) ? 1.f : -1.f;
				}

				go->pos += moveDir * moveStep;
			}
		}
		else
		{
			// In Transit - Lock to axis
			float offsetX = abs(go->pos.x - tileCenter.x);
			float offsetY = abs(go->pos.y - tileCenter.y);
			Vector3 moveDir(0, 0, 0);

			if (offsetX > offsetY) // Moving X
				moveDir.x = (toTarget.x > 0) ? 1.f : -1.f;
			else // Moving Y
				moveDir.y = (toTarget.y > 0) ? 1.f : -1.f;

			go->pos += moveDir * moveStep;
		}

		// Boundary check
		go->pos.x = Math::Clamp(go->pos.x, m_gridOffset, (m_noGrid - 1) * m_gridSize + m_gridOffset);
		go->pos.y = Math::Clamp(go->pos.y, m_gridOffset, (m_noGrid - 1) * m_gridSize + m_gridOffset);
	}

	// Update counts
	m_speedyAntWorkerCount = 0;
	m_speedyAntSoldierCount = 0;
	m_strongAntWorkerCount = 0;
	m_strongAntWarriorCount = 0;
	int totalObjects = 0;

	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;

		totalObjects++;

		switch (go->type)
		{
		case GameObject::GO_SPEEDY_ANT_WORKER:
			m_speedyAntWorkerCount++;
			break;
		case GameObject::GO_SPEEDY_ANT_SOLDIER:
			m_speedyAntSoldierCount++;
			break;
		case GameObject::GO_STRONG_ANT_WORKER:
			m_strongAntWorkerCount++;
			break;
		case GameObject::GO_STRONG_ANT_SOLDIER:
			m_strongAntWarriorCount++;
			break;
		}
	}

	SceneData::GetInstance()->SetObjectCount(totalObjects);
}

void SceneSandbox::DetectNearbyEntities(GameObject* go)
{
	if (go->type == GameObject::GO_FOOD ||
		go->type == GameObject::GO_SPEEDY_ANT_QUEEN ||
		go->type == GameObject::GO_STRONG_ANT_QUEEN)
		return;

	go->targetEnemy = nullptr;
	float nearestEnemyDistSq = FLT_MAX;

	// Use spatial grid for optimization
	int gridX = static_cast<int>(go->pos.x / m_gridSize);
	int gridY = static_cast<int>(go->pos.y / m_gridSize);

	// Check nearby cells only
	for (int dy = -2; dy <= 2; ++dy)
	{
		for (int dx = -2; dx <= 2; ++dx)
		{
			int checkX = gridX + dx;
			int checkY = gridY + dy;
			if (checkX < 0 || checkX >= m_noGrid || checkY < 0 || checkY >= m_noGrid)
				continue;

			int cellKey = checkY * m_noGrid + checkX;
			if (m_spatialGrid.find(cellKey) == m_spatialGrid.end())
				continue;

			for (GameObject* other : m_spatialGrid[cellKey])
			{
				if (!other->active || other == go)
					continue;

				// Check if enemy
				if (other->teamID != go->teamID && other->teamID >= 0 && go->teamID >= 0)
				{
					// Skip food
					if (other->type == GameObject::GO_FOOD)
						continue;

					float distSq = (go->pos - other->pos).LengthSquared();
					float detectionRangeSq = go->detectionRange * go->detectionRange;

					if (distSq < detectionRangeSq && distSq < nearestEnemyDistSq)
					{
						nearestEnemyDistSq = distSq;
						go->targetEnemy = other;

						// Queens detect threats
						if (go->type == GameObject::GO_SPEEDY_ANT_QUEEN && other->teamID == 1)
						{
							go->targetEnemy = other;
						}
						else if (go->type == GameObject::GO_STRONG_ANT_QUEEN && other->teamID == 0)
						{
							go->targetEnemy = other;
						}
					}
				}
			}
		}
	}
}

void SceneSandbox::FindNearestResource(GameObject* go)
{
	go->targetResource.SetZero();
	float nearestDistSq = FLT_MAX;

	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* resource = (GameObject*)*it;
		if (!resource->active || resource->type != GameObject::GO_FOOD)
			continue;

		float distSq = (go->pos - resource->pos).LengthSquared();
		if (distSq < nearestDistSq)
		{
			nearestDistSq = distSq;
			go->targetResource = resource->pos;
		}
	}
}

bool SceneSandbox::IsInTerritory(Vector3 pos, int teamID) const
{
	float halfGrid = m_noGrid * 0.5f;

	if (teamID == 0) // Speedy Ant territory (bottom-left)
	{
		return pos.x < halfGrid * m_gridSize && pos.y < halfGrid * m_gridSize;
	}
	else if (teamID == 1) // Strong Ant territory (top-right)
	{
		return pos.x > halfGrid * m_gridSize && pos.y > halfGrid * m_gridSize;
	}

	return false;
}

void SceneSandbox::UpdateSpatialGrid()
{
	m_spatialGrid.clear();

	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;

		int gridX = static_cast<int>(go->pos.x / m_gridSize);
		int gridY = static_cast<int>(go->pos.y / m_gridSize);
		int cellKey = gridY * m_noGrid + gridX;

		m_spatialGrid[cellKey].push_back(go);
	}
}

GameObject* SceneSandbox::GetNearestEnemy(Vector3 pos, int teamID, float maxRange)
{
	GameObject* nearest = nullptr;
	float nearestDistSq = maxRange * maxRange;

	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active || go->teamID == teamID || go->type == GameObject::GO_FOOD)
			continue;

		float distSq = (pos - go->pos).LengthSquared();
		if (distSq < nearestDistSq)
		{
			nearestDistSq = distSq;
			nearest = go;
		}
	}

	return nearest;
}

int SceneSandbox::IsWithinBoundary(int x) const
{
	return x >= 0 && x < m_noGrid;
}

int SceneSandbox::Get1DIndex(int x, int y) const
{
	return y * m_noGrid + x;
}

bool SceneSandbox::Handle(Message* message)
{
	MessageSpawnUnit* msgSpawn = dynamic_cast<MessageSpawnUnit*>(message);
	if (msgSpawn)
	{
		SpawnUnit(msgSpawn->type, msgSpawn->position, msgSpawn->spawner->teamID);
		return true;
	}

	MessageResourceDelivered* msgResource = dynamic_cast<MessageResourceDelivered*>(message);
	if (msgResource)
	{
		if (msgResource->teamID == 0)
			m_speedyAntResources += msgResource->resourceAmount;
		else
			m_strongAntResources += msgResource->resourceAmount;
		return true;
	}

	MessageUnitDied* msgDied = dynamic_cast<MessageUnitDied*>(message);
	if (msgDied)
	{
		// Update counts handled in Update()
		return true;
	}

	MessageResourceFound* msgFound = dynamic_cast<MessageResourceFound*>(message);
	if (msgFound)
	{
		// Can implement shared resource knowledge here
		return true;
	}

	MessageEnemySpotted* msgEnemy = dynamic_cast<MessageEnemySpotted*>(message);
	if (msgEnemy)
	{
		// Alert nearby allies
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active || go->teamID != msgEnemy->teamID)
				continue;

			// Soldiers/Warriors respond to enemy sightings
			if ((go->type == GameObject::GO_SPEEDY_ANT_SOLDIER || go->type == GameObject::GO_STRONG_ANT_SOLDIER) &&
				(go->pos - msgEnemy->enemy->pos).LengthSquared() < go->detectionRange * go->detectionRange * 2.f)
			{
				go->targetEnemy = msgEnemy->enemy;
			}
		}
		return true;
	}

	MessageRequestHelp* msgHelp = dynamic_cast<MessageRequestHelp*>(message);
	if (msgHelp)
	{
		// Nearby soldiers respond
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active || go->teamID != msgHelp->teamID)
				continue;

			if ((go->type == GameObject::GO_SPEEDY_ANT_SOLDIER || go->type == GameObject::GO_STRONG_ANT_SOLDIER) &&
				(go->pos - msgHelp->position).LengthSquared() < m_gridSize * m_gridSize * 64.f)
			{
				go->target = msgHelp->position;
			}
		}
		return true;
	}

	MessageQueenThreat* msgQueen = dynamic_cast<MessageQueenThreat*>(message);
	if (msgQueen)
	{
		// All military units return to defend queen
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active || go->teamID != msgQueen->teamID)
				continue;

			if (go->type == GameObject::GO_SPEEDY_ANT_SOLDIER || go->type == GameObject::GO_STRONG_ANT_SOLDIER)
			{
				go->target = msgQueen->queen->pos;
				go->targetEnemy = msgQueen->queen->targetEnemy;
			}
		}
		return true;
	}

	return false;
}

void SceneSandbox::RenderGO(GameObject* go)
{
	modelStack.PushMatrix();
	modelStack.Translate(go->pos.x, go->pos.y, 0.1f);
	modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);

	switch (go->type)
	{
	case GameObject::GO_SPEEDY_ANT_WORKER:
		RenderMesh(meshList[GEO_SPEEDY_ANT_WORKER], false);
		break;
	case GameObject::GO_SPEEDY_ANT_SOLDIER:
		RenderMesh(meshList[GEO_SPEEDY_ANT_SOLDIER], false);
		break;
	case GameObject::GO_SPEEDY_ANT_QUEEN:
		RenderMesh(meshList[GEO_SPEEDY_ANT_QUEEN], false);
		break;
	case GameObject::GO_STRONG_ANT_WORKER:
		RenderMesh(meshList[GEO_STRONG_ANT_WORKER], false);
		break;
	case GameObject::GO_STRONG_ANT_SOLDIER:
		RenderMesh(meshList[GEO_STRONG_ANT_SOLDIER], false);
		break;
	case GameObject::GO_STRONG_ANT_QUEEN:
		RenderMesh(meshList[GEO_STRONG_ANT_QUEEN], false);
		break;
	case GameObject::GO_FOOD:
		RenderMesh(meshList[GEO_FOOD], false);
		break;
	}

	// Health bar for combat units
	if (go->type != GameObject::GO_FOOD)
	{
		float healthPercent = go->health / go->maxHealth;
		modelStack.PushMatrix();
		modelStack.Translate(0, m_gridSize * 0.6f, 0.1f);
		modelStack.Scale(healthPercent, 0.1f, 1.f);
		meshList[GEO_WHITEQUAD]->material.kAmbient.Set(
			1.f - healthPercent, healthPercent, 0.f);
		RenderMesh(meshList[GEO_WHITEQUAD], true);
		modelStack.PopMatrix();
	}

	modelStack.PopMatrix();
}

void SceneSandbox::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix
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
	modelStack.LoadIdentity();

	// Render background
	modelStack.PushMatrix();
	modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
	modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_GRASS], false);
	modelStack.PopMatrix();

	for (int row = 0; row < m_noGrid; ++row)
	{
		for (int col = 0; col < m_noGrid; ++col)
		{
			if (m_wallGrid[Get1DIndex(col, row)])
			{
				modelStack.PushMatrix();
				modelStack.Translate(col * m_gridSize + m_gridOffset, row * m_gridSize + m_gridOffset, 0.1f);
				modelStack.Scale(m_gridSize, m_gridSize, 1.f);
				RenderMesh(meshList[GEO_WALL], true);
				modelStack.PopMatrix();
			}
		}
	}

	// Render grid lines (light)
	for (int i = 0; i <= m_noGrid; ++i)
	{
		modelStack.PushMatrix();
		modelStack.Translate(i * m_gridSize, m_worldHeight * 0.5f, -0.5f);
		modelStack.Scale(0.05f, m_worldHeight, 1.f);
		RenderMesh(meshList[GEO_WHITEQUAD], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldHeight * 0.5f, i * m_gridSize, -0.5f);
		modelStack.Scale(m_worldHeight, 0.05f, 1.f);
		RenderMesh(meshList[GEO_WHITEQUAD], true);
		modelStack.PopMatrix();
	}

	// Render territory markers
	// Speedy Ant territory (bottom-left) - red tint
	modelStack.PushMatrix();
	modelStack.Translate(m_gridSize * (m_noGrid * 0.25f), m_gridSize * (m_noGrid * 0.25f), -0.8f);
	modelStack.Scale(m_gridSize * m_noGrid * 0.5f, m_gridSize * m_noGrid * 0.5f, 1.f);
	RenderMesh(meshList[GEO_WHITEQUAD], true);
	modelStack.PopMatrix();

	// Strong Ants territory (top-right) - blue tint
	modelStack.PushMatrix();
	modelStack.Translate(m_gridSize * (m_noGrid * 0.75f), m_gridSize * (m_noGrid * 0.75f), -0.8f);
	modelStack.Scale(m_gridSize * m_noGrid * 0.5f, m_gridSize * m_noGrid * 0.5f, 1.f);
	RenderMesh(meshList[GEO_WHITEQUAD], true);
	modelStack.PopMatrix();

	// Render all game objects
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active)
		{
			RenderGO(go);
		}
	}

	// On screen text
	std::ostringstream ss;
	ss.precision(3);
	ss << "Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5f, 2, 57);

	ss.str("");
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5f, 2, 54);

	ss.str("");
	ss.precision(1);
	ss << "Time: " << m_simulationTime << "s / 300s";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.5f, 2, 51);

	// Speedy Ant Colony Stats
	ss.str("");
	ss << "=== SPEEDY ANTS COLONY ===";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.3f, 0.3f), 2.5f, 2, 46);

	ss.str("");
	ss << "Workers: " << m_speedyAntWorkerCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.5f, 0.5f), 2.5f, 2, 43);

	ss.str("");
	ss << "Soldiers: " << m_speedyAntSoldierCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.5f, 0.5f), 2.5f, 2, 40);

	ss.str("");
	ss << "Resources: " << m_speedyAntResources;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.5f, 0.5f), 2.5f, 2, 37);

	ss.str("");
	ss << "Queen HP: " << (m_speedyAntQueen->active ? static_cast<int>(m_speedyAntQueen->health) : 0);
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.5f, 0.5f), 2.5f, 2, 34);

	// Strong Ant colony Stats
	ss.str("");
	ss << "=== STRONG ANTS COLONY ===";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.3f, 0.3f, 1), 2.5f, 2, 28);

	ss.str("");
	ss << "Workers: " << m_strongAntWorkerCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.5f, 0.5f, 1), 2.5f, 2, 25);

	ss.str("");
	ss << "Soldiers: " << m_strongAntWarriorCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.5f, 0.5f, 1), 2.5f, 2, 22);

	ss.str("");
	ss << "Resources: " << m_strongAntResources;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.5f, 0.5f, 1), 2.5f, 2, 19);

	ss.str("");
	ss << "Queen HP: " << (m_strongAntQueen->active ? static_cast<int>(m_strongAntQueen->health) : 0);
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.5f, 0.5f, 1), 2.5f, 2, 16);

	// Win condition
	if (m_simulationEnded)
	{
		ss.str("");
		ss << "=== SIMULATION ENDED ===";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 3.f, 20, 30);

		ss.str("");
		if (m_winner == 0)
			ss << "WINNER: SPEEDY ANT COLONY";
		else if (m_winner == 1)
			ss << "WINNER: STRONG ANT COLONY";
		else
			ss << "RESULT: DRAW";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3.f, 20, 27);
	}

	RenderTextOnScreen(meshList[GEO_TEXT], "Speedy Ants vs Strong Ants Sandbox", Color(0, 1, 0), 3, 2, 2);
}
void SceneSandbox::Exit()
{
	SceneBase::Exit();
	while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		if (go->sm)
			delete go->sm;
		delete go;
		m_goList.pop_back();
	}

	m_spatialGrid.clear();
	m_foodLocations.clear();
	m_wallGrid.clear();
}