#include "SceneSandbox.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include "StatesSandbox.h"
#include "SceneData.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"
#include <iomanip>
#include <queue>
#include <algorithm>

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
	m_speedyAntQueen->pos.Set(m_gridSize * 3.f + m_gridOffset, m_gridSize * 3.f + m_gridOffset, 0);
	m_speedyAntQueen->homeBase = m_speedyAntQueen->pos;
	m_speedyAntQueen->scale.Set(m_gridSize * 1.5f, m_gridSize * 1.5f, 1.f);
	m_speedyAntQueen->maxHealth = 50.f; m_speedyAntQueen->health = 50.f;
	m_speedyAntQueen->moveSpeed = 0.f; m_speedyAntQueen->detectionRange = m_gridSize * 8.f;

	m_speedyAntQueen->sm = new StateMachine();
	m_speedyAntQueen->sm->AddState(new StateQueenSpawning("Spawning", m_speedyAntQueen));
	m_speedyAntQueen->sm->AddState(new StateQueenEmergency("Emergency", m_speedyAntQueen));
	m_speedyAntQueen->sm->AddState(new StateQueenCooldown("Cooldown", m_speedyAntQueen));
	m_speedyAntQueen->sm->SetNextState("Spawning");

	// Spawn Strong Ant Queen (top-right corner)
	m_strongAntQueen = FetchGO(GameObject::GO_STRONG_ANT_QUEEN);
	m_strongAntQueen->teamID = 1;
	m_strongAntQueen->pos.Set(m_gridSize * (m_noGrid - 4.f) + m_gridOffset, m_gridSize * (m_noGrid - 4.f) + m_gridOffset, 0);
	m_strongAntQueen->homeBase = m_strongAntQueen->pos;
	m_strongAntQueen->scale.Set(m_gridSize * 1.5f, m_gridSize * 1.5f, 1.f);
	m_strongAntQueen->maxHealth = 50.f; m_strongAntQueen->health = 50.f;
	m_strongAntQueen->moveSpeed = 0.f; m_strongAntQueen->detectionRange = m_gridSize * 8.f;

	m_strongAntQueen->sm = new StateMachine();
	m_strongAntQueen->sm->AddState(new StateQueenSpawning("Spawning", m_strongAntQueen));
	m_strongAntQueen->sm->AddState(new StateQueenEmergency("Emergency", m_strongAntQueen));
	m_strongAntQueen->sm->AddState(new StateQueenCooldown("Cooldown", m_strongAntQueen));
	m_strongAntQueen->sm->SetNextState("Spawning");

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
		int gridX, gridY;
		bool validPos = false;

		// Attempt to find valid position
		while (!validPos)
		{
			if (i < foodCount / 2) {
				int minC = static_cast<int>(m_noGrid * 0.3f);
				int maxC = static_cast<int>(m_noGrid * 0.7f);
				gridX = Math::RandIntMinMax(minC, maxC);
				gridY = Math::RandIntMinMax(minC, maxC);
			}
			else {
				gridX = Math::RandIntMinMax(2, m_noGrid - 3);
				gridY = Math::RandIntMinMax(2, m_noGrid - 3);
			}

			// Check walls
			if (!IsWithinBoundary(gridX) || !IsWithinBoundary(gridY) || m_wallGrid[Get1DIndex(gridX, gridY)])
				continue;

			// Check Speedy Ant Colony (Bottom-Left approx 0-7)
			if (gridX <= 8 && gridY <= 8) continue;

			// Check Strong Ant Colony (Top-Right approx 22-29)
			if (gridX >= 21 && gridY >= 21) continue;

			validPos = true;
		}

		float worldX = gridX * m_gridSize + m_gridOffset;
		float worldY = gridY * m_gridSize + m_gridOffset;

		food->pos.Set(worldX, worldY, 0);
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
	for (size_t i = 0; i < m_goList.size(); ++i) {
		GameObject* go = m_goList[i];
		if (!go->active && go->type == type) { go->active = true; return go; }
	}
	for (unsigned i = 0; i < 10; ++i) {
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
	case MessageSpawnUnit::UNIT_SPEEDY_ANT_WORKER:
		unit = FetchGO(GameObject::GO_SPEEDY_ANT_WORKER);
		unit->teamID = 0;
		unit->homeBase = m_speedyAntQueen->pos;
		unit->maxHealth = 5.f; unit->health = 5.f; unit->attackPower = 0.2f; unit->moveSpeed = 6.f; unit->baseSpeed = 6.f;
		unit->detectionRange = m_gridSize * 7.f; unit->attackRange = m_gridSize * 0.8f;

		unit->sm = new StateMachine();
		unit->sm->AddState(new StateWorkerIdle("Idle", unit));
		unit->sm->AddState(new StateWorkerSearching("Searching", unit));
		unit->sm->AddState(new StateWorkerGathering("Gathering", unit));
		unit->sm->AddState(new StateWorkerFleeing("Fleeing", unit));
		unit->sm->SetNextState("Idle");
		m_speedyAntWorkerCount++;
		break;

	case MessageSpawnUnit::UNIT_SPEEDY_ANT_SOLDIER:
		unit = FetchGO(GameObject::GO_SPEEDY_ANT_SOLDIER);
		unit->teamID = 0;
		unit->homeBase = m_speedyAntQueen->pos;
		unit->maxHealth = 10.f; unit->health = 10.f; unit->attackPower = 1.5f; unit->moveSpeed = 4.f; unit->baseSpeed = 4.f;
		unit->detectionRange = m_gridSize * 9.f; unit->attackRange = m_gridSize * 1.2f;

		unit->sm = new StateMachine();
		unit->sm->AddState(new StateSoldierPatrolling("Patrolling", unit));
		unit->sm->AddState(new StateSoldierAttacking("Attacking", unit));
		unit->sm->AddState(new StateSoldierResting("Resting", unit)); // Merged Defending
		unit->sm->AddState(new StateSoldierRetreating("Retreating", unit));
		unit->sm->SetNextState("Patrolling");
		m_speedyAntSoldierCount++;
		break;

	case MessageSpawnUnit::UNIT_STRONG_ANT_WORKER:
		unit = FetchGO(GameObject::GO_STRONG_ANT_WORKER);
		unit->teamID = 1;
		unit->homeBase = m_strongAntQueen->pos;
		unit->maxHealth = 15.f; unit->health = 15.f; unit->attackPower = 1.0f; unit->moveSpeed = 3.0f; unit->baseSpeed = 3.f;
		unit->detectionRange = m_gridSize * 5.f; unit->attackRange = m_gridSize * 0.7f;

		unit->sm = new StateMachine();
		unit->sm->AddState(new StateWorkerIdle("Idle", unit));
		unit->sm->AddState(new StateWorkerSearching("Searching", unit));
		unit->sm->AddState(new StateWorkerGathering("Gathering", unit));
		unit->sm->AddState(new StateWorkerFleeing("Fleeing", unit));
		unit->sm->SetNextState("Idle");
		m_strongAntWorkerCount++;
		break;

	case MessageSpawnUnit::UNIT_STRONG_ANT_SOLDIER:
		unit = FetchGO(GameObject::GO_STRONG_ANT_SOLDIER);
		unit->teamID = 1;
		unit->homeBase = m_strongAntQueen->pos;
		unit->maxHealth = 30.f; unit->health = 30.f; unit->attackPower = 5.0f; unit->moveSpeed = 2.0f; unit->baseSpeed = 2.f;
		unit->detectionRange = m_gridSize * 6.f; unit->attackRange = m_gridSize * 1.3f;

		unit->sm = new StateMachine();
		unit->sm->AddState(new StateSoldierPatrolling("Patrolling", unit));
		unit->sm->AddState(new StateSoldierAttacking("Attacking", unit));
		unit->sm->AddState(new StateSoldierResting("Resting", unit));
		unit->sm->AddState(new StateSoldierRetreating("Retreating", unit));
		unit->sm->SetNextState("Patrolling");
		m_strongAntWarriorCount++;
		break;
	case MessageSpawnUnit::UNIT_HEALER:
		unit = FetchGO(GameObject::GO_HEALER);
		unit->teamID = teamID;
		unit->homeBase = (teamID == 0) ? m_speedyAntQueen->pos : m_strongAntQueen->pos;
		unit->maxHealth = 8.f; unit->health = 8.f; unit->moveSpeed = 4.f; unit->baseSpeed = 4.f;
		unit->sm = new StateMachine();
		unit->sm->AddState(new StateHealerIdle("Idle", unit));
		unit->sm->AddState(new StateHealerTraveling("Traveling", unit));
		unit->sm->AddState(new StateHealerHealing("Healing", unit));
		unit->sm->SetNextState("Idle");
		break;

	case MessageSpawnUnit::UNIT_SCOUT:
		unit = FetchGO(GameObject::GO_SCOUT);
		unit->teamID = teamID;
		unit->homeBase = (teamID == 0) ? m_speedyAntQueen->pos : m_strongAntQueen->pos;
		unit->maxHealth = 5.f; unit->health = 5.f; unit->moveSpeed = 8.f; unit->baseSpeed = 8.f;
		unit->detectionRange = m_gridSize * 15.f; // Huge vision
		unit->sm = new StateMachine();
		unit->sm->AddState(new StateScoutPatrolling("Patrolling", unit));
		unit->sm->AddState(new StateScoutReporting("Reporting", unit));
		unit->sm->AddState(new StateScoutHiding("Hiding", unit));
		unit->sm->SetNextState("Patrolling");
		break;

	case MessageSpawnUnit::UNIT_TANK:
		unit = FetchGO(GameObject::GO_TANK);
		unit->teamID = teamID;
		unit->homeBase = (teamID == 0) ? m_speedyAntQueen->pos : m_strongAntQueen->pos;
		unit->maxHealth = 40.f; unit->health = 40.f; unit->moveSpeed = 1.5f; unit->baseSpeed = 1.5f;
		unit->attackPower = 1.0f; unit->attackRange = m_gridSize * 0.5f;
		unit->sm = new StateMachine();
		unit->sm->AddState(new StateTankGuarding("Guarding", unit));
		unit->sm->AddState(new StateTankBlocking("Blocking", unit));
		unit->sm->AddState(new StateTankRecovering("Recovering", unit));
		unit->sm->SetNextState("Guarding");
		break;
	}

	if (unit) {
		int gx = (int)(position.x / m_gridSize);
		int gy = (int)(position.y / m_gridSize);
		unit->pos.Set(gx * m_gridSize + m_gridOffset, gy * m_gridSize + m_gridOffset, 0);
		unit->target = unit->pos;
		unit->scale.Set(m_gridSize, m_gridSize, 1.f);
	}
}

std::vector<MazePt> SceneSandbox::FindPath(MazePt start, MazePt end)
{
	std::vector<MazePt> path;
	if (start.x == end.x && start.y == end.y) return path;
	if (!IsWithinBoundary(end.x) || !IsWithinBoundary(end.y)) return path;
	if (m_wallGrid[Get1DIndex(end.x, end.y)]) return path;

	std::vector<bool> visited(m_noGrid * m_noGrid, false);
	std::vector<int> parent(m_noGrid * m_noGrid, -1);
	std::queue<MazePt> q;

	q.push(start);
	visited[Get1DIndex(start.x, start.y)] = true;

	bool found = false;
	int dx[] = { 0, 0, -1, 1 };
	int dy[] = { 1, -1, 0, 0 };

	while (!q.empty())
	{
		MazePt curr = q.front();
		q.pop();

		if (curr.x == end.x && curr.y == end.y) { found = true; break; }

		for (int i = 0; i < 4; ++i)
		{
			int nx = curr.x + dx[i];
			int ny = curr.y + dy[i];
			if (IsWithinBoundary(nx) && IsWithinBoundary(ny))
			{
				int nIdx = Get1DIndex(nx, ny);
				if (!visited[nIdx] && !m_wallGrid[nIdx])
				{
					visited[nIdx] = true;
					parent[nIdx] = Get1DIndex(curr.x, curr.y);
					q.push(MazePt(nx, ny));
				}
			}
		}
	}

	if (found) {
		int currIdx = Get1DIndex(end.x, end.y);
		int startIdx = Get1DIndex(start.x, start.y);
		while (currIdx != startIdx) {
			path.push_back(MazePt(currIdx % m_noGrid, currIdx / m_noGrid));
			currIdx = parent[currIdx];
		}
		std::reverse(path.begin(), path.end());
	}
	return path;
}

// Removed collision logic
bool SceneSandbox::IsGridOccupied(int gridX, int gridY, GameObject* self)
{
	// Only return true if out of bounds or wall. Units do not block each other.
	if (!IsWithinBoundary(gridX) || !IsWithinBoundary(gridY)) return true;
	if (m_wallGrid[Get1DIndex(gridX, gridY)]) return true;
	return false;
}

MazePt SceneSandbox::GetNearestVacantNeighbor(MazePt target, MazePt start)
{
	int dx[] = { 0, 0, -1, 1 };
	int dy[] = { 1, -1, 0, 0 };
	MazePt bestPt = target;
	float minDist = FLT_MAX;
	for (int i = 0; i < 4; ++i) {
		int nx = target.x + dx[i];
		int ny = target.y + dy[i];
		if (IsWithinBoundary(nx) && IsWithinBoundary(ny) && !m_wallGrid[Get1DIndex(nx, ny)]) {
			float dist = (float)((nx - start.x) * (nx - start.x) + (ny - start.y) * (ny - start.y));
			if (dist < minDist) { minDist = dist; bestPt.Set(nx, ny); }
		}
	}
	return bestPt;
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
	for (size_t i = 0; i < m_goList.size(); ++i) {
		GameObject* go = m_goList[i];
		if (go->active && go->sm) go->sm->Update(dt * m_speed);
	}

	// Detection and interaction (optimized with spatial grid)
	int cycleCheck = 0;
	for (size_t i = 0; i < m_goList.size(); ++i) {
		GameObject* go = m_goList[i];
		if (go->active && (cycleCheck % 3) == m_updateCycle) {
			DetectNearbyEntities(go);
			if (go->type == GameObject::GO_SPEEDY_ANT_WORKER || go->type == GameObject::GO_STRONG_ANT_WORKER)
				FindNearestResource(go);
			if (go->type == GameObject::GO_HEALER)
				FindNearestInjuredAlly(go);
			// ----------------
		}
		cycleCheck++;
	}
	// Movement
	for (size_t i = 0; i < m_goList.size(); ++i)
	{
		GameObject* go = m_goList[i];
		if (!go->active || go->moveSpeed <= 0.f) continue;

		int gridX = static_cast<int>(go->pos.x / m_gridSize);
		int gridY = static_cast<int>(go->pos.y / m_gridSize);
		int targetGridX = static_cast<int>(go->target.x / m_gridSize);
		int targetGridY = static_cast<int>(go->target.y / m_gridSize);

		// Resource Targeting
		if (!go->targetResource.IsZero() && (go->target - go->targetResource).LengthSquared() < 1.0f) {
			MazePt startPt(gridX, gridY); MazePt resPt(targetGridX, targetGridY); MazePt adjPt = GetNearestVacantNeighbor(resPt, startPt);
			targetGridX = adjPt.x; targetGridY = adjPt.y;
		}

		gridX = Math::Clamp(gridX, 0, m_noGrid - 1); gridY = Math::Clamp(gridY, 0, m_noGrid - 1);
		targetGridX = Math::Clamp(targetGridX, 0, m_noGrid - 1); targetGridY = Math::Clamp(targetGridY, 0, m_noGrid - 1);

		bool needPath = false;
		if (go->path.empty()) { if (gridX != targetGridX || gridY != targetGridY) needPath = true; }
		else { MazePt last = go->path.back(); if (last.x != targetGridX || last.y != targetGridY) needPath = true; }

		if (needPath) {
			go->path = FindPath(MazePt(gridX, gridY), MazePt(targetGridX, targetGridY));
			Vector3 center = Vector3(gridX * m_gridSize + m_gridOffset, gridY * m_gridSize + m_gridOffset, go->pos.z);
			if ((go->pos - center).LengthSquared() > 0.05f) { go->path.insert(go->path.begin(), MazePt(gridX, gridY)); }
		}

		float step = go->moveSpeed * static_cast<float>(dt) * m_speed;

		Vector3 moveVec(0, 0, 0);

		if (!go->path.empty())
		{
			MazePt nextPt = go->path.front();
			Vector3 nextPos(nextPt.x * m_gridSize + m_gridOffset, nextPt.y * m_gridSize + m_gridOffset, go->pos.z);
			Vector3 dir = nextPos - go->pos;
			float dist = dir.Length();

			moveVec = dir; // Capture direction

			if (dist <= step) { go->pos = nextPos; go->path.erase(go->path.begin()); }
			else { go->pos += dir.Normalized() * step; }
		}
		else
		{
			Vector3 center = Vector3(gridX * m_gridSize + m_gridOffset, gridY * m_gridSize + m_gridOffset, go->pos.z);
			if ((go->pos - center).LengthSquared() > 0.001f) {
				Vector3 dir = center - go->pos;
				moveVec = dir; // Capture direction
				float dist = dir.Length();
				if (dist <= step) go->pos = center; else go->pos += dir.Normalized() * step;
			}
		}

		// NEW: Update Facing Direction if moving
		if (moveVec.LengthSquared() > 0.001f) {
			go->viewDir = moveVec.Normalized();
		}
	}

	// Update counts
	int totalObjects = 0;
	m_speedyAntWorkerCount = 0; m_speedyAntSoldierCount = 0; m_strongAntWorkerCount = 0; m_strongAntWarriorCount = 0;
	for (size_t i = 0; i < m_goList.size(); ++i) {
		GameObject* go = m_goList[i];
		if (!go->active) continue;
		totalObjects++;
		if (go->type == GameObject::GO_SPEEDY_ANT_WORKER) m_speedyAntWorkerCount++;
		else if (go->type == GameObject::GO_SPEEDY_ANT_SOLDIER) m_speedyAntSoldierCount++;
		else if (go->type == GameObject::GO_STRONG_ANT_WORKER) m_strongAntWorkerCount++;
		else if (go->type == GameObject::GO_STRONG_ANT_SOLDIER) m_strongAntWarriorCount++;
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

void SceneSandbox::FindNearestInjuredAlly(GameObject* go)
{
	go->targetAlly = nullptr;
	float nearestDistSq = FLT_MAX;
	for (GameObject* other : m_goList) {
		if (!other->active || other == go) continue;
		if (other->teamID == go->teamID && other->health < other->maxHealth) {
			float distSq = (go->pos - other->pos).LengthSquared();
			if (distSq < nearestDistSq) { nearestDistSq = distSq; go->targetAlly = other; }
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
	if (msgSpawn) {
		// Define Costs
		int cost = 0;
		if (msgSpawn->type == MessageSpawnUnit::UNIT_SPEEDY_ANT_SOLDIER || msgSpawn->type == MessageSpawnUnit::UNIT_STRONG_ANT_SOLDIER)
			cost = 5;
		else
			cost = 2;

		// Check and Deduct Resources
		bool spawned = false;
		if (msgSpawn->spawner->teamID == 0) { // Speedy
			if (m_speedyAntResources >= cost) {
				m_speedyAntResources -= cost;
				SpawnUnit(msgSpawn->type, msgSpawn->position, 0);
				spawned = true;
			}
		}
		else { // Strong
			if (m_strongAntResources >= cost) {
				m_strongAntResources -= cost;
				SpawnUnit(msgSpawn->type, msgSpawn->position, 1);
				spawned = true;
			}
		}
		// If resource check failed, we just ignore the request. The Queen will try again later.
		return true;
	}

	MessageResourceDelivered* msgResource = dynamic_cast<MessageResourceDelivered*>(message);
	if (msgResource) { if (msgResource->teamID == 0) m_speedyAntResources += msgResource->resourceAmount; else m_strongAntResources += msgResource->resourceAmount; return true; }
	MessageUnitDied* msgDied = dynamic_cast<MessageUnitDied*>(message); if (msgDied) { return true; }
	MessageResourceFound* msgFound = dynamic_cast<MessageResourceFound*>(message); if (msgFound) { return true; }

	// FIX: Loop safely here too
	MessageEnemySpotted* msgEnemy = dynamic_cast<MessageEnemySpotted*>(message);
	if (msgEnemy) {
		for (size_t i = 0; i < m_goList.size(); ++i) {
			GameObject* go = m_goList[i];
			if (!go->active || go->teamID != msgEnemy->teamID) continue;
			if ((go->type == GameObject::GO_SPEEDY_ANT_SOLDIER || go->type == GameObject::GO_STRONG_ANT_SOLDIER) && (go->pos - msgEnemy->enemy->pos).LengthSquared() < go->detectionRange * go->detectionRange * 2.f) { go->targetEnemy = msgEnemy->enemy; }
		}
		return true;
	}
	MessageRequestHelp* msgHelp = dynamic_cast<MessageRequestHelp*>(message);
	if (msgHelp) {
		for (size_t i = 0; i < m_goList.size(); ++i) {
			GameObject* go = m_goList[i];
			if (!go->active || go->teamID != msgHelp->teamID) continue;
			if ((go->type == GameObject::GO_SPEEDY_ANT_SOLDIER || go->type == GameObject::GO_STRONG_ANT_SOLDIER) && (go->pos - msgHelp->position).LengthSquared() < m_gridSize * m_gridSize * 64.f) { go->target = msgHelp->position; }
		}
		return true;
	}
	MessageQueenThreat* msgQueen = dynamic_cast<MessageQueenThreat*>(message);
	if (msgQueen) {
		for (size_t i = 0; i < m_goList.size(); ++i) {
			GameObject* go = m_goList[i];
			if (!go->active || go->teamID != msgQueen->teamID) continue;
			if (go->type == GameObject::GO_SPEEDY_ANT_SOLDIER || go->type == GameObject::GO_STRONG_ANT_SOLDIER) { go->target = msgQueen->queen->pos; go->targetEnemy = msgQueen->queen->targetEnemy; }
		}
		return true;
	}
	return false;
}

void SceneSandbox::RenderGO(GameObject* go)
{
	modelStack.PushMatrix();
	modelStack.Translate(go->pos.x, go->pos.y, 0.1f);
	float angle = Math::RadianToDegree(atan2(go->viewDir.y, go->viewDir.x));
	modelStack.Rotate(angle - 90.0f, 0, 0, 1);
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
	case GameObject::GO_HEALER:
		RenderMesh(meshList[GEO_WHITEQUAD], false);
		break;
	case GameObject::GO_SCOUT:
		RenderMesh(meshList[GEO_WHITEQUAD], false);
		break;
	case GameObject::GO_TANK:
		RenderMesh(meshList[GEO_WHITEQUAD], false);
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

	//walls
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
	meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.5f, 0.5f, 0.5f);
	// Render grid lines (light)
	for (int i = 0; i <= m_noGrid; ++i)
	{
		// Vertical Lines
		modelStack.PushMatrix();
		modelStack.Translate(i * m_gridSize, m_worldHeight * 0.5f, -0.5f);
		modelStack.Scale(0.05f, m_worldHeight, 1.f);
		RenderMesh(meshList[GEO_WHITEQUAD], true);
		modelStack.PopMatrix();

		// Horizontal Lines
		// FIXED: Scale based on m_worldHeight (not Width) to keep it a perfect square
		modelStack.PushMatrix();
		modelStack.Translate(m_worldHeight * 0.5f, i * m_gridSize, -0.5f);
		modelStack.Scale(m_worldHeight, 0.05f, 1.f);
		RenderMesh(meshList[GEO_WHITEQUAD], true);
		modelStack.PopMatrix();
	}

	// Render territory markers
	float territorySize = m_gridSize * 8.f;

	// Speedy Ant Territory (Bottom-Left: 0 to 8)
	// Center = 4.0 * gridSize
	meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.8f, 0.2f, 0.2f); // RED
	modelStack.PushMatrix();
	modelStack.Translate(m_gridSize * 4.0f, m_gridSize * 4.0f, -0.8f);
	modelStack.Scale(territorySize, territorySize, 1.f);
	RenderMesh(meshList[GEO_TERRITORYRED], true);
	modelStack.PopMatrix();

	// Strong Ant Territory (Top-Right: 22 to 30)
	// Center = 26.0 * gridSize
	meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.2f, 0.2f, 0.8f); // BLUE
	modelStack.PushMatrix();
	modelStack.Translate(m_gridSize * 26.0f, m_gridSize * 26.0f, -0.8f);
	modelStack.Scale(territorySize, territorySize, 1.f);
	RenderMesh(meshList[GEO_TERRITORYBLUE], true);
	modelStack.PopMatrix();

	// Reset to White for other objects using this mesh
	meshList[GEO_WHITEQUAD]->material.kAmbient.Set(1.f, 1.f, 1.f);

	// --- NEW: STACKING LOGIC ---
	// Map: CellIndex -> GameObjectType -> Count
	std::map<int, std::map<int, int>> cellCounts;

	// Pass 1: Count objects per cell
	for (auto go : m_goList)
	{
		if (!go->active) continue;
		int gx = (int)(go->pos.x / m_gridSize);
		int gy = (int)(go->pos.y / m_gridSize);
		// Safety clamp
		if (gx < 0) gx = 0; if (gx >= m_noGrid) gx = m_noGrid - 1;
		if (gy < 0) gy = 0; if (gy >= m_noGrid) gy = m_noGrid - 1;

		int idx = gy * m_noGrid + gx;
		cellCounts[idx][go->type]++;
	}

	// Pass 2: Render unique objects with counts
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active)
		{
			int gx = (int)(go->pos.x / m_gridSize);
			int gy = (int)(go->pos.y / m_gridSize);
			if (gx < 0) gx = 0; if (gx >= m_noGrid) gx = m_noGrid - 1;
			if (gy < 0) gy = 0; if (gy >= m_noGrid) gy = m_noGrid - 1;
			int idx = gy * m_noGrid + gx;

			// Check the count for this specific type in this cell
			int count = cellCounts[idx][go->type];

			// If count > 0, it means we haven't rendered this type for this cell yet
			if (count > 0)
			{
				RenderGO(go);

				// If there is more than 1, draw the count text
				if (count > 1)
				{
					std::ostringstream ss;
					ss << count; // e.g. "3"

					modelStack.PushMatrix();
					// Position text slightly offset from the unit center (top-right)
					modelStack.Translate(go->pos.x + m_gridSize * 0.2f, go->pos.y + m_gridSize * 0.2f, 0.2f);
					// Scale text appropriate to grid size
					modelStack.Scale(m_gridSize, m_gridSize, 1.f);
					RenderText(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1)); // White text
					modelStack.PopMatrix();
				}

				// Set count to 0 so we don't render this type for this cell again this frame
				cellCounts[idx][go->type] = 0;
			}
			// If count was 0, we skip RenderGO (this unit is "hidden" inside the stack)
		}
	}

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

	// Stats Column
	float colX = 60.f;

	ss.str("");
	ss << "Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5f, colX, 57);

	ss.str("");
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5f, colX, 54);

	ss.str("");
	ss << std::fixed << std::setprecision(1);
	ss << "Time: " << m_simulationTime << "s / 300s";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.5f, colX, 51);

	// Speedy Ant Colony Stats
	ss.str("");
	ss << "=== SPEEDY ANTS COLONY ===";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.3f, 0.3f), 2.5f, colX, 46);

	ss.str("");
	ss << "Workers: " << m_speedyAntWorkerCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.5f, 0.5f), 2.5f, colX, 43);

	ss.str("");
	ss << "Soldiers: " << m_speedyAntSoldierCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.5f, 0.5f), 2.5f, colX, 40);

	ss.str("");
	ss << "Resources: " << m_speedyAntResources;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.5f, 0.5f), 2.5f, colX, 37);

	ss.str("");
	ss << "Queen HP: " << (m_speedyAntQueen->active ? static_cast<int>(m_speedyAntQueen->health) : 0);
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.5f, 0.5f), 2.5f, colX, 34);

	// Strong Ant colony Stats
	ss.str("");
	ss << "=== STRONG ANTS COLONY ===";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.3f, 0.3f, 1), 2.5f, colX, 28);

	ss.str("");
	ss << "Workers: " << m_strongAntWorkerCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.5f, 0.5f, 1), 2.5f, colX, 25);

	ss.str("");
	ss << "Soldiers: " << m_strongAntWarriorCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.5f, 0.5f, 1), 2.5f, colX, 22);

	ss.str("");
	ss << "Resources: " << m_strongAntResources;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.5f, 0.5f, 1), 2.5f, colX, 19);

	ss.str("");
	ss << "Queen HP: " << (m_strongAntQueen->active ? static_cast<int>(m_strongAntQueen->health) : 0);
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.5f, 0.5f, 1), 2.5f, colX, 16);

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