#include "StatesSandbox.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"
#include "SceneData.h"
#include "MyMath.h"

// Helper
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
void StateWorkerSearching::Enter() { m_go->moveSpeed = m_go->baseSpeed; m_go->targetResource.SetZero(); }
void StateWorkerSearching::Update(double dt) {
	if (m_go->targetResource.IsZero()) {
		if ((m_go->pos - m_go->target).LengthSquared() < 0.1f) {
			m_go->target = GetRandomGridPosAround(m_go->pos, 10);
		}
	}
	else { m_go->sm->SetNextState("Gathering"); }
	if (m_go->targetEnemy != nullptr) m_go->sm->SetNextState("Fleeing");
}
void StateWorkerSearching::Exit() {}

StateWorkerGathering::StateWorkerGathering(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
StateWorkerGathering::~StateWorkerGathering() {}
void StateWorkerGathering::Enter() { m_go->moveSpeed = m_go->baseSpeed * 0.66f; m_go->gatherTimer = 0.f; m_go->isCarryingResource = false; }
void StateWorkerGathering::Update(double dt) {
	if (m_go->targetEnemy != nullptr) { m_go->sm->SetNextState("Fleeing"); return; }
	float gridSize = SceneData::GetInstance()->GetGridSize();
	// NEW: Interaction Range covers neighbor tile + margin (e.g., 1.5 tiles)
	float interactSq = (gridSize * 1.5f) * (gridSize * 1.5f);

	if (!m_go->isCarryingResource) {
		if (!m_go->targetResource.IsZero()) {
			m_go->target = m_go->targetResource;
			// NEW: Check distance with larger range
			if ((m_go->pos - m_go->targetResource).LengthSquared() < interactSq) {
				m_go->gatherTimer += (float)dt;
				if (m_go->gatherTimer > 2.f) {
					m_go->isCarryingResource = true; m_go->carriedResources = 1; m_go->gatherTimer = 0.f;
					PostOffice::GetInstance()->Send("Scene", new MessageResourceFound(m_go, m_go->targetResource, m_go->teamID));
				}
			}
		}
		else { m_go->sm->SetNextState("Searching"); }
	}
	else {
		m_go->target = m_go->homeBase;
		if ((m_go->pos - m_go->homeBase).LengthSquared() < interactSq) {
			PostOffice::GetInstance()->Send("Scene", new MessageResourceDelivered(m_go, m_go->carriedResources, m_go->teamID));
			m_go->isCarryingResource = false; m_go->carriedResources = 0; m_go->targetResource.SetZero(); m_go->sm->SetNextState("Idle");
		}
	}
}
void StateWorkerGathering::Exit() {}

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
void StateSoldierPatrolling::Update(double dt) {
	patrolTimer += (float)dt;
	if (m_go->targetEnemy && m_go->targetEnemy->active) { PostOffice::GetInstance()->Send("Scene", new MessageEnemySpotted(m_go, m_go->targetEnemy, m_go->teamID)); m_go->sm->SetNextState("Attacking"); return; }
	if (patrolTimer > 4.f || patrolTarget.IsZero() || (m_go->pos - m_go->target).LengthSquared() < 0.5f) {
		patrolTimer = 0.f; patrolTarget = GetRandomGridPosAround(m_go->homeBase, 5.f); m_go->target = patrolTarget;
	}
}
void StateSoldierPatrolling::Exit() {}

StateSoldierAttacking::StateSoldierAttacking(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), attackCooldown(0.f) {}
StateSoldierAttacking::~StateSoldierAttacking() {}
void StateSoldierAttacking::Enter() { m_go->moveSpeed = m_go->baseSpeed; attackCooldown = 0.f; }
void StateSoldierAttacking::Update(double dt) {
	attackCooldown += (float)dt;
	if (!m_go->targetEnemy || !m_go->targetEnemy->active) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Resting"); return; }
	m_go->target = m_go->targetEnemy->pos;
	if ((m_go->pos - m_go->targetEnemy->pos).LengthSquared() < m_go->attackRange * m_go->attackRange) {
		if (attackCooldown > 0.5f) {
			m_go->targetEnemy->health -= m_go->attackPower; attackCooldown = 0.f;
			if (m_go->targetEnemy->health <= 0.f) {
				PostOffice::GetInstance()->Send("Scene", new MessageUnitDied(m_go->targetEnemy, m_go->targetEnemy->teamID, m_go->targetEnemy->type));
				m_go->targetEnemy->active = false; m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Resting");
			}
		}
	}
	if (m_go->health < m_go->maxHealth * 0.3f) m_go->sm->SetNextState("Retreating");
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

		// NEW: Random Chance (50% Soldier, 50% Worker)
		bool spawnSoldier = (Math::RandIntMinMax(0, 100) < 50);

		MessageSpawnUnit::UNIT_TYPE type;
		if (m_go->teamID == 0) type = spawnSoldier ? MessageSpawnUnit::UNIT_SPEEDY_ANT_SOLDIER : MessageSpawnUnit::UNIT_SPEEDY_ANT_WORKER;
		else type = spawnSoldier ? MessageSpawnUnit::UNIT_STRONG_ANT_SOLDIER : MessageSpawnUnit::UNIT_STRONG_ANT_WORKER;

		// Send request. If resources fail, nothing happens, loop continues.
		PostOffice::GetInstance()->Send("Scene", new MessageSpawnUnit(m_go, type, m_go->pos));

		// We assume success or wait for next cycle. No complex feedback loop needed for this level.
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
void StateScoutPatrolling::Enter() { m_go->moveSpeed = m_go->baseSpeed; timer = 5.f; }
void StateScoutPatrolling::Update(double dt) {
	timer += (float)dt;
	if (m_go->targetEnemy && m_go->targetEnemy->active) { m_go->sm->SetNextState("Reporting"); return; }
	if (timer > 2.f || (m_go->pos - m_go->target).LengthSquared() < 1.f) {
		// Patrol far (radius 15) using existing helper
		m_go->target = GetRandomGridPosAround(m_go->homeBase, 15.f); timer = 0.f;
	}
}
void StateScoutPatrolling::Exit() {}

void StateScoutReporting::Enter() {
	m_go->moveSpeed = m_go->baseSpeed * 1.5f;
	if (m_go->targetEnemy) PostOffice::GetInstance()->Send("Scene", new MessageEnemySpotted(m_go, m_go->targetEnemy, m_go->teamID));
}
void StateScoutReporting::Update(double dt) {
	m_go->target = m_go->homeBase;
	if ((m_go->pos - m_go->homeBase).LengthSquared() < 5.f) { m_go->targetEnemy = nullptr; m_go->sm->SetNextState("Patrolling"); }
	if (m_go->health < m_go->maxHealth * 0.5f) m_go->sm->SetNextState("Hiding");
}
void StateScoutReporting::Exit() {}

void StateScoutHiding::Enter() {
	m_go->moveSpeed = m_go->baseSpeed; timer = 0.f;
	float max = SceneData::GetInstance()->GetGridSize() * SceneData::GetInstance()->GetNumGrid();
	if (m_go->teamID == 0) m_go->target.Set(0, max, 0); else m_go->target.Set(max, 0, 0);
}
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