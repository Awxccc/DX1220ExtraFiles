#include "StatesSandbox.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"
#include "SceneData.h"
#include "MyMath.h"

// --- NEW GLOBALS ---
static std::vector<bool> g_visitedNodes[2];
static bool g_enemyColonyFound[2] = { false, false }; // [Cite: User Requirement 2]
static Vector3 g_enemyColonyPos[2];

void ResizeVisitedNodes() {
	int size = SceneData::GetInstance()->GetNumGrid() * SceneData::GetInstance()->GetNumGrid();
	if (g_visitedNodes[0].size() != size) g_visitedNodes[0].assign(size, false);
	if (g_visitedNodes[1].size() != size) g_visitedNodes[1].assign(size, false);
}
void ResetGlobalSandboxVars() {
	g_enemyColonyFound[0] = false;
	g_enemyColonyFound[1] = false;
	g_enemyColonyPos[0].SetZero();
	g_enemyColonyPos[1].SetZero();

	// Also clear visited nodes if you want a fresh exploration map
	ResizeVisitedNodes();
	std::fill(g_visitedNodes[0].begin(), g_visitedNodes[0].end(), false);
	std::fill(g_visitedNodes[1].begin(), g_visitedNodes[1].end(), false);
}

// ... [Keep GetRandomExplorationTarget, MarkVisited, GetRandomEntrance, GetRandomGridPosAround as is] ...
Vector3 GetRandomExplorationTarget(Vector3 center, int teamID) { /* Keep existing code */ return Vector3(0, 0, 0); /*placeholder for brevity*/ }
void MarkVisited(Vector3 pos, int teamID) { /* Keep existing code */ }
Vector3 GetRandomEntrance(int teamID) { /* Keep existing code */ return Vector3(0, 0, 0); /*placeholder*/ }
Vector3 GetRandomGridPosAround(Vector3 center, float range) { /* Keep existing code */ return Vector3(0, 0, 0); /*placeholder*/ }

// ================= WORKER STATES (Keep Unchanged) =================
// ... [StateWorkerIdle, StateWorkerSearching, StateWorkerGathering, StateWorkerFleeing implementation unchanged] ...
StateWorkerIdle::StateWorkerIdle(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerIdle::~StateWorkerIdle() {}
void StateWorkerIdle::Enter() { m_go->moveSpeed = 0.f; }
void StateWorkerIdle::Update(double dt) { static float timer = 0.f; timer += (float)dt; if (timer > 1.f) { timer = 0.f; m_go->sm->SetNextState("Searching"); } }
void StateWorkerIdle::Exit() {}

StateWorkerSearching::StateWorkerSearching(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerSearching::~StateWorkerSearching() {}
void StateWorkerSearching::Enter() { m_go->moveSpeed = m_go->baseSpeed; m_go->targetResource.SetZero(); m_go->targetFoodItem = nullptr; m_go->pathHistory.clear(); }
void StateWorkerSearching::Update(double dt) {
	if (m_go->targetEnemy != nullptr && m_go->health < m_go->maxHealth * 0.4f) { m_go->sm->SetNextState("Fleeing"); return; }
	if (!m_go->targetFoodItem) { if ((m_go->pos - m_go->target).LengthSquared() < 0.1f) { if (m_go->teamID == 0) m_go->target = GetRandomGridPosAround(Vector3(4.f * SceneData::GetInstance()->GetGridSize(), 4.f * SceneData::GetInstance()->GetGridSize(), 0), 4); else m_go->target = GetRandomGridPosAround(Vector3(26.f * SceneData::GetInstance()->GetGridSize(), 26.f * SceneData::GetInstance()->GetGridSize(), 0), 4); } }
	else { m_go->sm->SetNextState("Gathering"); }
}
void StateWorkerSearching::Exit() {}

StateWorkerGathering::StateWorkerGathering(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerGathering::~StateWorkerGathering() {}
void StateWorkerGathering::Enter() { m_go->moveSpeed = m_go->baseSpeed * 0.66f; m_go->gatherTimer = 0.f; m_go->isCarryingResource = false; if (m_go->targetFoodItem) m_go->targetFoodItem->harvesterCount++; }
void StateWorkerGathering::Update(double dt) {
	if (m_go->targetEnemy != nullptr && m_go->health < m_go->maxHealth * 0.4f) { m_go->sm->SetNextState("Fleeing"); return; }
	float interactSq = (SceneData::GetInstance()->GetGridSize() * 2.0f) * (SceneData::GetInstance()->GetGridSize() * 2.0f);
	if (!m_go->isCarryingResource) { if (m_go->targetFoodItem && m_go->targetFoodItem->active) { m_go->target = m_go->targetFoodItem->pos; if ((m_go->pos - m_go->targetFoodItem->pos).LengthSquared() < interactSq) { m_go->gatherTimer += (float)dt; if (m_go->gatherTimer > 2.f) { m_go->isCarryingResource = true; m_go->carriedResources = 1; m_go->gatherTimer = 0.f; m_go->targetFoodItem->resourceCount--; if (m_go->targetFoodItem->resourceCount <= 0) m_go->targetFoodItem->active = false; if (m_go->targetFoodItem) m_go->targetFoodItem->harvesterCount--; m_go->targetFoodItem = nullptr; m_go->targetResource.SetZero(); if (!m_go->pathHistory.empty()) { m_go->path = m_go->pathHistory; std::reverse(m_go->path.begin(), m_go->path.end()); m_go->pathHistory.clear(); } } } } else { m_go->targetFoodItem = nullptr; m_go->sm->SetNextState("Searching"); } }
	else { if (m_go->path.empty()) m_go->target = m_go->homeBase; if ((m_go->pos - m_go->homeBase).LengthSquared() < interactSq) { PostOffice::GetInstance()->Send("Scene", new MessageResourceDelivered(m_go, m_go->carriedResources, m_go->teamID)); m_go->isCarryingResource = false; m_go->carriedResources = 0; m_go->targetFoodItem = nullptr; m_go->sm->SetNextState("Idle"); } }
}
void StateWorkerGathering::Exit() { if (m_go->targetFoodItem) m_go->targetFoodItem->harvesterCount--; }

StateWorkerFleeing::StateWorkerFleeing(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerFleeing::~StateWorkerFleeing() {}
void StateWorkerFleeing::Enter() { m_go->moveSpeed = m_go->baseSpeed * 1.5f; PostOffice::GetInstance()->Send("Scene", new MessageRequestHelp(m_go, m_go->pos, m_go->teamID)); }
void StateWorkerFleeing::Update(double dt) { if (m_go->targetEnemy && m_go->targetEnemy->active) { Vector3 dir = m_go->pos - m_go->targetEnemy->pos; if (dir.LengthSquared() > 0.1f) { dir.Normalize(); m_go->target = GetRandomGridPosAround(m_go->pos + dir * SceneData::GetInstance()->GetGridSize() * 3.f, 1); } else { m_go->target = m_go->homeBase; } if ((m_go->pos - m_go->targetEnemy->pos).LengthSquared() > m_go->detectionRange * m_go->detectionRange * 4.f) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Idle"); } } else { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Idle"); } }
void StateWorkerFleeing::Exit() {}

// ================= SOLDIER STATES =================
StateSoldierPatrolling::StateSoldierPatrolling(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), patrolTimer(0.f) {}
StateSoldierPatrolling::~StateSoldierPatrolling() {}
void StateSoldierPatrolling::Enter() { m_go->moveSpeed = m_go->baseSpeed; patrolTimer = 0.f; patrolTarget.SetZero(); }
void StateSoldierPatrolling::Update(double dt) {
	patrolTimer += (float)dt;
	if (m_go->targetEnemy && m_go->targetEnemy->active) {
		PostOffice::GetInstance()->Send("Scene", new MessageEnemySpotted(m_go, m_go->targetEnemy, m_go->teamID));
		m_go->sm->SetNextState("Attacking");
		return;
	}

	// --- NEW: ATTACK COLONY IF FOUND ---
	if (g_enemyColonyFound[m_go->teamID]) {
		m_go->target = g_enemyColonyPos[m_go->teamID];
		// Reset patrol timer to prevent interference, just march
		patrolTimer = 0.f;
	}
	else {
		// Standard Patrol Logic
		if (patrolTimer > 4.f || patrolTarget.IsZero() || (m_go->pos - m_go->target).LengthSquared() < 0.5f) {
			patrolTimer = 0.f;
			patrolTarget = GetRandomEntrance(m_go->teamID);
			m_go->target = patrolTarget;
		}
	}
}
void StateSoldierPatrolling::Exit() {}

StateSoldierAttacking::StateSoldierAttacking(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), attackCooldown(0.f) {}
StateSoldierAttacking::~StateSoldierAttacking() {}
void StateSoldierAttacking::Enter() { m_go->moveSpeed = m_go->baseSpeed; attackCooldown = 0.f; }
void StateSoldierAttacking::Update(double dt) {
	if (m_go->health < m_go->maxHealth * 0.3f) { m_go->sm->SetNextState("Retreating"); return; }
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

	// --- NEW: CONDITIONAL HEALING ---
	// "if colony has at least one enemy npc they wont be able to heal"
	// We check if the soldier currently perceives an enemy near the base
	bool baseIsSafe = true;
	if (m_go->targetEnemy && m_go->targetEnemy->active) {
		float distToHomeSq = (m_go->targetEnemy->pos - m_go->homeBase).LengthSquared();
		// If enemy is within ~5 grids of home, consider it unsafe
		if (distToHomeSq < 100.f) baseIsSafe = false;
	}

	if (baseIsSafe && (m_go->pos - m_go->homeBase).LengthSquared() < 4.f) {
		m_go->health = Math::Min(m_go->maxHealth, m_go->health + (float)dt * 1.f);
	}

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
	// --- NEW: QUEEN FLEE CHECK ---
	if (m_go->health < m_go->maxHealth * 0.2f) { // Low Health Flee
		m_go->sm->SetNextState("Fleeing");
		return;
	}

	m_go->spawnCooldown += (float)dt;
	if (m_go->targetEnemy && m_go->targetEnemy->active) { PostOffice::GetInstance()->Send("Scene", new MessageQueenThreat(m_go, m_go->teamID)); m_go->sm->SetNextState("Emergency"); return; }
	if (m_go->spawnCooldown > 3.f) {
		m_go->spawnCooldown = 0.f;
		int rng = Math::RandIntMinMax(0, 4);
		MessageSpawnUnit::UNIT_TYPE type;
		if (m_go->teamID == 0) { switch (rng) { case 0: type = MessageSpawnUnit::UNIT_SPEEDY_ANT_WORKER; break; case 1: type = MessageSpawnUnit::UNIT_SPEEDY_ANT_SOLDIER; break; case 2: type = MessageSpawnUnit::UNIT_HEALER; break; case 3: type = MessageSpawnUnit::UNIT_SCOUT; break; case 4: type = MessageSpawnUnit::UNIT_TANK; break; default: type = MessageSpawnUnit::UNIT_SPEEDY_ANT_WORKER; break; } }
													  else { switch (rng) { case 0: type = MessageSpawnUnit::UNIT_STRONG_ANT_WORKER; break; case 1: type = MessageSpawnUnit::UNIT_STRONG_ANT_SOLDIER; break; case 2: type = MessageSpawnUnit::UNIT_HEALER; break; case 3: type = MessageSpawnUnit::UNIT_SCOUT; break; case 4: type = MessageSpawnUnit::UNIT_TANK; break; default: type = MessageSpawnUnit::UNIT_STRONG_ANT_WORKER; break; } }
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
void StateQueenEmergency::Update(double dt) {
	if (m_go->health < m_go->maxHealth * 0.2f) { m_go->sm->SetNextState("Fleeing"); return; } // Flee Check
	if (!m_go->targetEnemy || !m_go->targetEnemy->active) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Cooldown"); }
}
void StateQueenEmergency::Exit() {}

StateQueenCooldown::StateQueenCooldown(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), timer(0.f) {}
StateQueenCooldown::~StateQueenCooldown() {}
void StateQueenCooldown::Enter() { timer = 0.f; }
void StateQueenCooldown::Update(double dt) {
	if (m_go->health < m_go->maxHealth * 0.2f) { m_go->sm->SetNextState("Fleeing"); return; } // Flee Check
	timer += (float)dt; if (timer > 2.f) m_go->sm->SetNextState("Spawning");
}
void StateQueenCooldown::Exit() {}

// --- NEW: QUEEN FLEEING STATE ---
StateQueenFleeing::StateQueenFleeing(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateQueenFleeing::~StateQueenFleeing() {}
void StateQueenFleeing::Enter() {
	m_go->moveSpeed = m_go->baseSpeed * 0.5f; // Slow movement
}
void StateQueenFleeing::Update(double dt) {
	// Simple flee logic: Move to own base (corner) or away from specific threat
	// Since the Queen usually sits AT the base, fleeing implies running to the safest extreme corner 
	// or kiting. Let's make her move to the absolute corner of her territory.
	float max = SceneData::GetInstance()->GetGridSize() * SceneData::GetInstance()->GetNumGrid();
	if (m_go->teamID == 0) m_go->target.Set(0, 0, 0); // Red Base Corner
	else m_go->target.Set(max, max, 0); // Blue Base Corner

	// Recover? If health restored (by Healers)
	if (m_go->health > m_go->maxHealth * 0.5f) m_go->sm->SetNextState("Spawning");
}
void StateQueenFleeing::Exit() { m_go->moveSpeed = 0.f; }


// ================= SCOUT STATES =================
void StateScoutPatrolling::Enter() {
	m_go->moveSpeed = m_go->baseSpeed; timer = 5.f;
	m_go->targetFoodItem = nullptr;
	m_go->targetResource.SetZero();
}
void StateScoutPatrolling::Update(double dt) {
	timer += (float)dt;
	MarkVisited(m_go->pos, m_go->teamID);

	float gridSize = SceneData::GetInstance()->GetGridSize();
	float worldSize = gridSize * SceneData::GetInstance()->GetNumGrid();
	Vector3 enemyBasePos = (m_go->teamID == 0) ? Vector3(worldSize, worldSize, 0) : Vector3(0, 0, 0);

	// Check distance to enemy base (e.g., within 5 grid blocks)
	if (!g_enemyColonyFound[m_go->teamID]) {
		if ((m_go->pos - enemyBasePos).LengthSquared() < (gridSize * 5) * (gridSize * 5)) {
			// COLONY FOUND!
			g_enemyColonyFound[m_go->teamID] = true;
			g_enemyColonyPos[m_go->teamID] = enemyBasePos;
			// Optional: Send message
			PostOffice::GetInstance()->Send("Scene", new MessageSpawnUnit(m_go, MessageSpawnUnit::UNIT_PHEROMONE, m_go->pos));
		}
	}

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

	// NEW: Initialize last trail position to current position
	lastTrailPos = m_go->pos;

	if (m_go->targetEnemy) PostOffice::GetInstance()->Send("Scene", new MessageEnemySpotted(m_go, m_go->targetEnemy, m_go->teamID));
}
void StateScoutReturnToColony::Update(double dt) {
	m_go->target = m_go->homeBase;

	// --- FIX: DISTANCE-BASED TRAIL LOGIC ---
	if (m_go->targetFoodItem != nullptr) {
		// Calculate distance squared from the last dropped pheromone
		float distSq = (m_go->pos - lastTrailPos).LengthSquared();

		// Threshold: Drop a trail every 1.5 units (Adjust this value for smaller/larger gaps)
		float trailSpacing = 1.5f;

		if (distSq > trailSpacing * trailSpacing) {
			PostOffice::GetInstance()->Send("Scene", new MessageSpawnUnit(m_go, MessageSpawnUnit::UNIT_PHEROMONE, m_go->pos));
			lastTrailPos = m_go->pos; // Update the last drop position
		}
	}
	// -----------------------------------

	if ((m_go->pos - m_go->homeBase).LengthSquared() < 5.f) {
		m_go->targetEnemy = nullptr;
		m_go->sm->SetNextState("Patrolling");
	}
}
void StateScoutReturnToColony::Exit() {}

void StateScoutHiding::Enter() { m_go->moveSpeed = m_go->baseSpeed; timer = 0.f; float max = SceneData::GetInstance()->GetGridSize() * SceneData::GetInstance()->GetNumGrid(); if (m_go->teamID == 0) m_go->target.Set(0, max, 0); else m_go->target.Set(max, 0, 0); }
void StateScoutHiding::Update(double dt) { timer += (float)dt; if (timer > 5.f) m_go->sm->SetNextState("Patrolling"); }
void StateScoutHiding::Exit() {}

// ================= HEALER / TANK (Unchanged or Minor Tweak for Recovery) =================
// Note: StateTankRecovering needs similar check to SoldierResting
void StateTankRecovering::Enter() { m_go->moveSpeed = m_go->baseSpeed; m_go->target = m_go->homeBase; }
void StateTankRecovering::Update(double dt) {
	// Conditional Healing Check for Tank
	bool baseIsSafe = true;
	if (m_go->targetEnemy && m_go->targetEnemy->active) {
		if ((m_go->targetEnemy->pos - m_go->homeBase).LengthSquared() < 100.f) baseIsSafe = false;
	}
	if (baseIsSafe) m_go->health += (float)dt * 2.0f;

	if (m_go->health >= m_go->maxHealth) { m_go->health = m_go->maxHealth; m_go->sm->SetNextState("Guarding"); }
}
void StateTankRecovering::Exit() {}

// [Keep the rest of the Tank/Healer states as provided in original]
void StateHealerIdle::Enter() { m_go->moveSpeed = 0.f; }
void StateHealerIdle::Update(double dt) { if (m_go->targetAlly && m_go->targetAlly->active && m_go->targetAlly->health < m_go->targetAlly->maxHealth) { m_go->sm->SetNextState("Traveling"); } }
void StateHealerIdle::Exit() {}
void StateHealerTraveling::Enter() { m_go->moveSpeed = m_go->baseSpeed; }
void StateHealerTraveling::Update(double dt) { if (!m_go->targetAlly || !m_go->targetAlly->active) { m_go->targetAlly = nullptr; m_go->sm->SetNextState("Idle"); return; } m_go->target = m_go->targetAlly->pos; if ((m_go->pos - m_go->target).LengthSquared() < SceneData::GetInstance()->GetGridSize() * 2.f) { m_go->sm->SetNextState("Healing"); } }
void StateHealerTraveling::Exit() {}
void StateHealerHealing::Enter() { m_go->moveSpeed = 0.f; timer = 0.f; }
void StateHealerHealing::Update(double dt) { timer += (float)dt; if (!m_go->targetAlly || !m_go->targetAlly->active) { m_go->sm->SetNextState("Idle"); return; } m_go->targetAlly->health += (float)dt * 5.0f; if (m_go->targetAlly->health >= m_go->targetAlly->maxHealth) { m_go->targetAlly->health = m_go->targetAlly->maxHealth; m_go->targetAlly = nullptr; m_go->sm->SetNextState("Idle"); } }
void StateHealerHealing::Exit() {}
void StateTankGuarding::Enter() { m_go->moveSpeed = m_go->baseSpeed; }
void StateTankGuarding::Update(double dt) { m_go->target = m_go->homeBase; if (m_go->targetEnemy && m_go->targetEnemy->active) { if ((m_go->pos - m_go->targetEnemy->pos).LengthSquared() < m_go->attackRange * m_go->attackRange) m_go->sm->SetNextState("Blocking"); } if (m_go->health < m_go->maxHealth * 0.4f) m_go->sm->SetNextState("Recovering"); }
void StateTankGuarding::Exit() {}
void StateTankBlocking::Enter() { m_go->moveSpeed = 0.f; attackTimer = 0.f; }
void StateTankBlocking::Update(double dt) { if (!m_go->targetEnemy || !m_go->targetEnemy->active || (m_go->pos - m_go->targetEnemy->pos).LengthSquared() > m_go->attackRange * m_go->attackRange * 1.5f) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Guarding"); return; } attackTimer += (float)dt; if (attackTimer > 1.5f) { m_go->targetEnemy->health -= m_go->attackPower; attackTimer = 0.f; if (m_go->targetEnemy->health <= 0) { PostOffice::GetInstance()->Send("Scene", new MessageUnitDied(m_go->targetEnemy, m_go->targetEnemy->teamID, m_go->targetEnemy->type)); m_go->targetEnemy->active = false; } } if (m_go->health < m_go->maxHealth * 0.3f) m_go->sm->SetNextState("Recovering"); }
void StateTankBlocking::Exit() {}