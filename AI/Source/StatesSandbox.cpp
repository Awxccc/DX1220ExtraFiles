#include "StatesSandbox.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"
#include "SceneData.h"
#include "MyMath.h"

static std::vector<bool> g_visitedNodes[2];
void ResizeVisitedNodes() {
	int size = SceneData::GetInstance()->GetNumGrid() * SceneData::GetInstance()->GetNumGrid();
	if (g_visitedNodes[0].size() != size) g_visitedNodes[0].assign(size, false);
	if (g_visitedNodes[1].size() != size) g_visitedNodes[1].assign(size, false);
}

// Get Random Unexplored Position (or random if mostly explored)
Vector3 GetRandomExplorationTarget(Vector3 center, int teamID)
{
	ResizeVisitedNodes();
	float gridSize = SceneData::GetInstance()->GetGridSize();
	float offset = SceneData::GetInstance()->GetGridOffset();
	int gridNum = SceneData::GetInstance()->GetNumGrid();

	// Try 10 times to find an unexplored spot
	for (int i = 0; i < 10; ++i) {
		int nX = Math::RandIntMinMax(0, gridNum - 1);
		int nY = Math::RandIntMinMax(0, gridNum - 1);
		int idx = nY * gridNum + nX;
		if (!g_visitedNodes[teamID][idx]) {
			return Vector3(nX * gridSize + offset, nY * gridSize + offset, 0);
		}
	}
	// Fallback: Just random
	int nX = Math::RandIntMinMax(0, gridNum - 1);
	int nY = Math::RandIntMinMax(0, gridNum - 1);
	return Vector3(nX * gridSize + offset, nY * gridSize + offset, 0);
}

// Mark current position as visited
void MarkVisited(Vector3 pos, int teamID) {
	ResizeVisitedNodes();
	int gridNum = SceneData::GetInstance()->GetNumGrid();
	int gx = (int)(pos.x / SceneData::GetInstance()->GetGridSize());
	int gy = (int)(pos.y / SceneData::GetInstance()->GetGridSize());
	if (gx >= 0 && gx < gridNum && gy >= 0 && gy < gridNum) {
		g_visitedNodes[teamID][gy * gridNum + gx] = true;
	}
}

Vector3 GetRandomEntrance(int teamID)
{
	float gridSize = SceneData::GetInstance()->GetGridSize();
	float offset = SceneData::GetInstance()->GetGridOffset();

	int gx, gy;
	int choice = Math::RandIntMinMax(0, 3); // 4 gaps per base

	if (teamID == 0) // Red Colony (Bottom Left)
	{
		// Gaps at (3,7), (4,7) [Top Wall] and (7,3), (7,4) [Right Wall]
		if (choice == 0) { gx = 3; gy = 7; }
		else if (choice == 1) { gx = 4; gy = 7; }
		else if (choice == 2) { gx = 7; gy = 3; }
		else { gx = 7; gy = 4; }
	}
	else // Blue Colony (Top Right)
	{
		// Gaps at (26,22), (27,22) [Bottom Wall] and (22,26), (22,27) [Left Wall]
		if (choice == 0) { gx = 26; gy = 22; }
		else if (choice == 1) { gx = 27; gy = 22; }
		else if (choice == 2) { gx = 22; gy = 26; }
		else { gx = 22; gy = 27; }
	}

	return Vector3(gx * gridSize + offset, gy * gridSize + offset, 0);
}

Vector3 GetRandomGridPosAround(Vector3 center, float range)
{
	float gridSize = SceneData::GetInstance()->GetGridSize();
	float offset = SceneData::GetInstance()->GetGridOffset();
	int gridNum = SceneData::GetInstance()->GetNumGrid();
	int cX = (int)(center.x / gridSize);
	int cY = (int)(center.y / gridSize);
	int r = (int)range;
	int nX = Math::Clamp(Math::RandIntMinMax(cX - r, cX + r), 0, gridNum - 1);
	int nY = Math::Clamp(Math::RandIntMinMax(cY - r, cY + r), 0, gridNum - 1);
	return Vector3(nX * gridSize + offset, nY * gridSize + offset, 0);
}

// ================= WORKER STATES =================
StateWorkerIdle::StateWorkerIdle(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerIdle::~StateWorkerIdle() {}
void StateWorkerIdle::Enter() { m_go->moveSpeed = 0.f; }
void StateWorkerIdle::Update(double dt) { static float timer = 0.f; timer += (float)dt; if (timer > 1.f) { timer = 0.f; m_go->sm->SetNextState("Searching"); } }
void StateWorkerIdle::Exit() {}

StateWorkerSearching::StateWorkerSearching(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerSearching::~StateWorkerSearching() {}
void StateWorkerSearching::Enter() {
	m_go->moveSpeed = m_go->baseSpeed;
	m_go->targetResource.SetZero();
	m_go->targetFoodItem = nullptr;
	m_go->pathHistory.clear(); // --- FIX: Clear history on start ---
}
void StateWorkerSearching::Update(double dt) {
	// --- UPDATED FLEE CHECK (Health < 40%) ---
	if (m_go->targetEnemy != nullptr && m_go->health < m_go->maxHealth * 0.4f) {
		m_go->sm->SetNextState("Fleeing"); return;
	}
	// -----------------------------------------

	if (!m_go->targetFoodItem) {
		if ((m_go->pos - m_go->target).LengthSquared() < 0.1f) {
			if (m_go->teamID == 0) m_go->target = GetRandomGridPosAround(Vector3(4.f * SceneData::GetInstance()->GetGridSize(), 4.f * SceneData::GetInstance()->GetGridSize(), 0), 4);
			else m_go->target = GetRandomGridPosAround(Vector3(26.f * SceneData::GetInstance()->GetGridSize(), 26.f * SceneData::GetInstance()->GetGridSize(), 0), 4);
		}
	}
	else { m_go->sm->SetNextState("Gathering"); }
}
void StateWorkerSearching::Exit() {}

StateWorkerGathering::StateWorkerGathering(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerGathering::~StateWorkerGathering() {}
void StateWorkerGathering::Enter() {
	m_go->moveSpeed = m_go->baseSpeed * 0.66f;
	m_go->gatherTimer = 0.f;
	m_go->isCarryingResource = false;
	// RESERVE SPOT
	if (m_go->targetFoodItem) m_go->targetFoodItem->harvesterCount++;
}
void StateWorkerGathering::Update(double dt) {
	// --- UPDATED FLEE CHECK (Health < 40%) ---
	if (m_go->targetEnemy != nullptr && m_go->health < m_go->maxHealth * 0.4f) {
		m_go->sm->SetNextState("Fleeing"); return;
	}
	// -----------------------------------------

	float interactSq = (SceneData::GetInstance()->GetGridSize() * 2.0f) * (SceneData::GetInstance()->GetGridSize() * 2.0f);

	if (!m_go->isCarryingResource) {
		if (m_go->targetFoodItem && m_go->targetFoodItem->active) {
			m_go->target = m_go->targetFoodItem->pos;
			if ((m_go->pos - m_go->targetFoodItem->pos).LengthSquared() < interactSq) {
				m_go->gatherTimer += (float)dt;
				if (m_go->gatherTimer > 2.f) {
					m_go->isCarryingResource = true; m_go->carriedResources = 1; m_go->gatherTimer = 0.f;
					m_go->targetFoodItem->resourceCount--;
					if (m_go->targetFoodItem->resourceCount <= 0) m_go->targetFoodItem->active = false;

					if (m_go->targetFoodItem) m_go->targetFoodItem->harvesterCount--;
					m_go->targetFoodItem = nullptr;
					m_go->targetResource.SetZero();

					if (!m_go->pathHistory.empty()) {
						m_go->path = m_go->pathHistory;
						std::reverse(m_go->path.begin(), m_go->path.end());
						m_go->pathHistory.clear();
					}
				}
			}
		}
		else { m_go->targetFoodItem = nullptr; m_go->sm->SetNextState("Searching"); }
	}
	else {
		if (m_go->path.empty()) m_go->target = m_go->homeBase;

		if ((m_go->pos - m_go->homeBase).LengthSquared() < interactSq) {
			PostOffice::GetInstance()->Send("Scene", new MessageResourceDelivered(m_go, m_go->carriedResources, m_go->teamID));
			m_go->isCarryingResource = false; m_go->carriedResources = 0; m_go->targetFoodItem = nullptr; m_go->sm->SetNextState("Idle");
		}
	}
}
void StateWorkerGathering::Exit() {
	if (m_go->targetFoodItem) m_go->targetFoodItem->harvesterCount--;
}

StateWorkerFleeing::StateWorkerFleeing(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerFleeing::~StateWorkerFleeing() {}
void StateWorkerFleeing::Enter() { m_go->moveSpeed = m_go->baseSpeed * 1.5f; PostOffice::GetInstance()->Send("Scene", new MessageRequestHelp(m_go, m_go->pos, m_go->teamID)); }
void StateWorkerFleeing::Update(double dt) {
	if (m_go->targetEnemy && m_go->targetEnemy->active) {
		Vector3 dir = m_go->pos - m_go->targetEnemy->pos;
		if (dir.LengthSquared() > 0.1f) { dir.Normalize(); m_go->target = GetRandomGridPosAround(m_go->pos + dir * SceneData::GetInstance()->GetGridSize() * 3.f, 1); }
		else { m_go->target = m_go->homeBase; }
		if ((m_go->pos - m_go->targetEnemy->pos).LengthSquared() > m_go->detectionRange * m_go->detectionRange * 4.f) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Idle"); }
	}
	else { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Idle"); }
}
void StateWorkerFleeing::Exit() {}

// ================= SOLDIER STATES =================
StateSoldierPatrolling::StateSoldierPatrolling(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), patrolTimer(0.f) {}
StateSoldierPatrolling::~StateSoldierPatrolling() {}
void StateSoldierPatrolling::Enter() { m_go->moveSpeed = m_go->baseSpeed; patrolTimer = 0.f; patrolTarget.SetZero(); }
void StateSoldierPatrolling::Update(double dt) { patrolTimer += (float)dt; if (m_go->targetEnemy && m_go->targetEnemy->active) { PostOffice::GetInstance()->Send("Scene", new MessageEnemySpotted(m_go, m_go->targetEnemy, m_go->teamID)); m_go->sm->SetNextState("Attacking"); return; } if (patrolTimer > 4.f || patrolTarget.IsZero() || (m_go->pos - m_go->target).LengthSquared() < 0.5f) { patrolTimer = 0.f; patrolTarget = GetRandomEntrance(m_go->teamID); m_go->target = patrolTarget; } }
void StateSoldierPatrolling::Exit() {}

StateSoldierAttacking::StateSoldierAttacking(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), attackCooldown(0.f) {}
StateSoldierAttacking::~StateSoldierAttacking() {}
void StateSoldierAttacking::Enter() { m_go->moveSpeed = m_go->baseSpeed; attackCooldown = 0.f; }
void StateSoldierAttacking::Update(double dt) {
	// --- UPDATED RETREAT CHECK ---
	if (m_go->health < m_go->maxHealth * 0.3f) { m_go->sm->SetNextState("Retreating"); return; }
	// -----------------------------
	attackCooldown += (float)dt; if (!m_go->targetEnemy || !m_go->targetEnemy->active) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Resting"); return; } m_go->target = m_go->targetEnemy->pos; if ((m_go->pos - m_go->targetEnemy->pos).LengthSquared() < m_go->attackRange * m_go->attackRange) { if (attackCooldown > 0.5f) { m_go->targetEnemy->health -= m_go->attackPower; attackCooldown = 0.f; if (m_go->targetEnemy->health <= 0.f) { PostOffice::GetInstance()->Send("Scene", new MessageUnitDied(m_go->targetEnemy, m_go->targetEnemy->teamID, m_go->targetEnemy->type)); m_go->targetEnemy->active = false; m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Resting"); } } }
}
void StateSoldierAttacking::Exit() {}

StateSoldierResting::StateSoldierResting(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), restTimer(0.f) {}
StateSoldierResting::~StateSoldierResting() {}
void StateSoldierResting::Enter() { m_go->moveSpeed = m_go->baseSpeed; m_go->target = m_go->homeBase; restTimer = 0.f; }
void StateSoldierResting::Update(double dt) {
	restTimer += (float)dt;
	if ((m_go->pos - m_go->homeBase).LengthSquared() > 1.f) m_go->target = m_go->homeBase;
	if (m_go->targetEnemy && m_go->targetEnemy->active) { m_go->sm->SetNextState("Attacking"); return; }
	if ((m_go->pos - m_go->homeBase).LengthSquared() < 4.f) { m_go->health = Math::Min(m_go->maxHealth, m_go->health + (float)dt * 1.f); }
	if (m_go->health > m_go->maxHealth * 0.9f && restTimer > 2.f) m_go->sm->SetNextState("Patrolling");
}
void StateSoldierResting::Exit() {}

StateSoldierRetreating::StateSoldierRetreating(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateSoldierRetreating::~StateSoldierRetreating() {}
void StateSoldierRetreating::Enter() { m_go->moveSpeed = m_go->baseSpeed * 1.5f; PostOffice::GetInstance()->Send("Scene", new MessageRequestHelp(m_go, m_go->pos, m_go->teamID)); }
void StateSoldierRetreating::Update(double dt) { m_go->target = m_go->homeBase; if ((m_go->pos - m_go->homeBase).LengthSquared() < 4.f) m_go->sm->SetNextState("Resting"); }
void StateSoldierRetreating::Exit() {}

// ================= QUEEN STATES =================
StateQueenSpawning::StateQueenSpawning(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateQueenSpawning::~StateQueenSpawning() {}
void StateQueenSpawning::Enter() { m_go->moveSpeed = 0.f; m_go->spawnCooldown = 0.f; }
void StateQueenSpawning::Update(double dt) {
	m_go->spawnCooldown += (float)dt;
	if (m_go->targetEnemy && m_go->targetEnemy->active) { PostOffice::GetInstance()->Send("Scene", new MessageQueenThreat(m_go, m_go->teamID)); m_go->sm->SetNextState("Emergency"); return; }

	// Try spawning every 3 seconds
	if (m_go->spawnCooldown > 3.f) {
		m_go->spawnCooldown = 0.f;

		// --- UPDATED SPAWN LOGIC: EQUAL CHANCE ---
		int rng = Math::RandIntMinMax(0, 4); // 0 to 4 (5 types)
		MessageSpawnUnit::UNIT_TYPE type;

		if (m_go->teamID == 0) // RED TEAM
		{
			switch (rng) {
			case 0: type = MessageSpawnUnit::UNIT_SPEEDY_ANT_WORKER; break;
			case 1: type = MessageSpawnUnit::UNIT_SPEEDY_ANT_SOLDIER; break;
			case 2: type = MessageSpawnUnit::UNIT_HEALER; break;
			case 3: type = MessageSpawnUnit::UNIT_SCOUT; break;
			case 4: type = MessageSpawnUnit::UNIT_TANK; break;
			default: type = MessageSpawnUnit::UNIT_SPEEDY_ANT_WORKER; break;
			}
		}
		else // BLUE TEAM
		{
			switch (rng) {
			case 0: type = MessageSpawnUnit::UNIT_STRONG_ANT_WORKER; break;
			case 1: type = MessageSpawnUnit::UNIT_STRONG_ANT_SOLDIER; break;
			case 2: type = MessageSpawnUnit::UNIT_HEALER; break;
			case 3: type = MessageSpawnUnit::UNIT_SCOUT; break;
			case 4: type = MessageSpawnUnit::UNIT_TANK; break;
			default: type = MessageSpawnUnit::UNIT_STRONG_ANT_WORKER; break;
			}
		}
		// -----------------------------------------

		PostOffice::GetInstance()->Send("Scene", new MessageSpawnUnit(m_go, type, m_go->pos));
		m_go->unitsSpawned++;
		m_go->sm->SetNextState("Cooldown");
	}
}
void StateQueenSpawning::Exit() {}

StateQueenEmergency::StateQueenEmergency(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateQueenEmergency::~StateQueenEmergency() {}
void StateQueenEmergency::Enter() {
	m_go->moveSpeed = 0.f;
	MessageSpawnUnit::UNIT_TYPE type = (m_go->teamID == 0) ? MessageSpawnUnit::UNIT_SPEEDY_ANT_SOLDIER : MessageSpawnUnit::UNIT_STRONG_ANT_SOLDIER;
	for (int i = 0; i < 3; ++i) PostOffice::GetInstance()->Send("Scene", new MessageSpawnUnit(m_go, type, m_go->pos));
}
void StateQueenEmergency::Update(double dt) { if (!m_go->targetEnemy || !m_go->targetEnemy->active) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Cooldown"); } }
void StateQueenEmergency::Exit() {}

StateQueenCooldown::StateQueenCooldown(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), timer(0.f) {}
StateQueenCooldown::~StateQueenCooldown() {}
void StateQueenCooldown::Enter() { timer = 0.f; }
void StateQueenCooldown::Update(double dt) { timer += (float)dt; if (timer > 2.f) m_go->sm->SetNextState("Spawning"); }
void StateQueenCooldown::Exit() {}

// ================= HEALER STATES =================
void StateHealerIdle::Enter() { m_go->moveSpeed = 0.f; }
void StateHealerIdle::Update(double dt) {
	if (m_go->targetAlly && m_go->targetAlly->active && m_go->targetAlly->health < m_go->targetAlly->maxHealth) {
		m_go->sm->SetNextState("Traveling");
	}
}
void StateHealerIdle::Exit() {}

void StateHealerTraveling::Enter() { m_go->moveSpeed = m_go->baseSpeed; }
void StateHealerTraveling::Update(double dt) {
	if (!m_go->targetAlly || !m_go->targetAlly->active) {
		m_go->targetAlly = nullptr; m_go->sm->SetNextState("Idle"); return;
	}
	m_go->target = m_go->targetAlly->pos;
	if ((m_go->pos - m_go->target).LengthSquared() < SceneData::GetInstance()->GetGridSize() * 2.f) {
		m_go->sm->SetNextState("Healing");
	}
}
void StateHealerTraveling::Exit() {}

void StateHealerHealing::Enter() { m_go->moveSpeed = 0.f; timer = 0.f; }
void StateHealerHealing::Update(double dt) {
	timer += (float)dt;
	if (!m_go->targetAlly || !m_go->targetAlly->active) { m_go->sm->SetNextState("Idle"); return; }
	m_go->targetAlly->health += (float)dt * 5.0f; // Heal 5 HP/sec
	if (m_go->targetAlly->health >= m_go->targetAlly->maxHealth) {
		m_go->targetAlly->health = m_go->targetAlly->maxHealth; m_go->targetAlly = nullptr; m_go->sm->SetNextState("Idle");
	}
}
void StateHealerHealing::Exit() {}

// ================= SCOUT STATES =================
void StateScoutPatrolling::Enter() { 
    m_go->moveSpeed = m_go->baseSpeed; timer = 5.f; 
    m_go->targetFoodItem = nullptr; 
    m_go->targetResource.SetZero();
}
void StateScoutPatrolling::Update(double dt) {
	timer += (float)dt;
	MarkVisited(m_go->pos, m_go->teamID);

	// Low Health Flee
	if (m_go->targetEnemy && m_go->targetEnemy->active && m_go->health < m_go->maxHealth * 0.4f) {
		m_go->sm->SetNextState("ReturnToColony"); return;
	}

	if (m_go->targetFoodItem && m_go->targetFoodItem->active) {
		if (m_go->targetFoodItem->isMarked) {
			m_go->targetFoodItem = nullptr;
		}
		else {
			float distSq = (m_go->pos - m_go->targetFoodItem->pos).LengthSquared();
			// --- FIX: REDUCED REACH TO 1.3 GRIDS (Requires actual visit) ---
			float reachSq = (SceneData::GetInstance()->GetGridSize() * 1.3f) * (SceneData::GetInstance()->GetGridSize() * 1.3f);

			if (distSq < reachSq) {
				m_go->targetFoodItem->isMarked = true;
				PostOffice::GetInstance()->Send("Scene", new MessageSpawnUnit(m_go, MessageSpawnUnit::UNIT_PHEROMONE, m_go->pos));
				m_go->sm->SetNextState("ReturnToColony");
				return;
			}
			else { m_go->target = m_go->targetFoodItem->pos; return; }
		}
	}
	if (timer > 2.f || (m_go->pos - m_go->target).LengthSquared() < 1.f) { m_go->target = GetRandomExplorationTarget(m_go->homeBase, m_go->teamID); timer = 0.f; }
}
void StateScoutPatrolling::Exit() {}

void StateScoutReturnToColony::Enter() {
	m_go->moveSpeed = m_go->baseSpeed * 1.5f;
	// Initialize last grid pos to trigger immediate update on move
	float gs = SceneData::GetInstance()->GetGridSize();
	lastGridPos.Set((int)(m_go->pos.x / gs), (int)(m_go->pos.y / gs));
	if (m_go->targetEnemy) PostOffice::GetInstance()->Send("Scene", new MessageEnemySpotted(m_go, m_go->targetEnemy, m_go->teamID));
}
void StateScoutReturnToColony::Update(double dt) {
	m_go->target = m_go->homeBase;

	// --- FIX: GRID-BASED TRAIL LOGIC ---
	if (m_go->targetFoodItem != nullptr) {
		float gs = SceneData::GetInstance()->GetGridSize();
		int curX = (int)(m_go->pos.x / gs);
		int curY = (int)(m_go->pos.y / gs);

		// If we moved to a new grid cell, drop trail
		if (curX != lastGridPos.x || curY != lastGridPos.y) {
			PostOffice::GetInstance()->Send("Scene", new MessageSpawnUnit(m_go, MessageSpawnUnit::UNIT_PHEROMONE, m_go->pos));
			lastGridPos.Set(curX, curY);
		}
	}
	// -----------------------------------

	if ((m_go->pos - m_go->homeBase).LengthSquared() < 5.f) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Patrolling"); }
}
void StateScoutReturnToColony::Exit() {}

void StateScoutHiding::Enter() { m_go->moveSpeed = m_go->baseSpeed; timer = 0.f; float max = SceneData::GetInstance()->GetGridSize() * SceneData::GetInstance()->GetNumGrid(); if (m_go->teamID == 0) m_go->target.Set(0, max, 0); else m_go->target.Set(max, 0, 0); }
void StateScoutHiding::Update(double dt) { timer += (float)dt; if (timer > 5.f) m_go->sm->SetNextState("Patrolling"); }
void StateScoutHiding::Exit() {}

// ================= TANK STATES =================
void StateTankGuarding::Enter() { m_go->moveSpeed = m_go->baseSpeed; }
void StateTankGuarding::Update(double dt) {
	m_go->target = m_go->homeBase;
	if (m_go->targetEnemy && m_go->targetEnemy->active) {
		if ((m_go->pos - m_go->targetEnemy->pos).LengthSquared() < m_go->attackRange * m_go->attackRange) m_go->sm->SetNextState("Blocking");
	}
	if (m_go->health < m_go->maxHealth * 0.4f) m_go->sm->SetNextState("Recovering");
}
void StateTankGuarding::Exit() {}

void StateTankBlocking::Enter() { m_go->moveSpeed = 0.f; attackTimer = 0.f; }
void StateTankBlocking::Update(double dt) {
	if (!m_go->targetEnemy || !m_go->targetEnemy->active || (m_go->pos - m_go->targetEnemy->pos).LengthSquared() > m_go->attackRange * m_go->attackRange * 1.5f) {
		m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Guarding"); return;
	}
	attackTimer += (float)dt;
	if (attackTimer > 1.5f) {
		m_go->targetEnemy->health -= m_go->attackPower; attackTimer = 0.f;
		if (m_go->targetEnemy->health <= 0) {
			PostOffice::GetInstance()->Send("Scene", new MessageUnitDied(m_go->targetEnemy, m_go->targetEnemy->teamID, m_go->targetEnemy->type));
			m_go->targetEnemy->active = false;
		}
	}
	if (m_go->health < m_go->maxHealth * 0.3f) m_go->sm->SetNextState("Recovering");
}
void StateTankBlocking::Exit() {}

void StateTankRecovering::Enter() { m_go->moveSpeed = m_go->baseSpeed; m_go->target = m_go->homeBase; }
void StateTankRecovering::Update(double dt) {
	m_go->health += (float)dt * 2.0f;
	if (m_go->health >= m_go->maxHealth) { m_go->health = m_go->maxHealth; m_go->sm->SetNextState("Guarding"); }
}
void StateTankRecovering::Exit() {}