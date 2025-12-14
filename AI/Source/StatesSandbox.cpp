#include "StatesSandbox.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"
#include "SceneData.h"
#include "MyMath.h"

// ============= ANT WORKER STATES IMPLEMENTATION =============
StateAntWorkerIdle::StateAntWorkerIdle(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}

StateAntWorkerIdle::~StateAntWorkerIdle() {}

void StateAntWorkerIdle::Enter()
{
	m_go->moveSpeed = 0.f;
	m_go->target = m_go->pos;
}

void StateAntWorkerIdle::Update(double dt)
{
	// Transition to searching after brief idle
	static float idleTimer = 0.f;
	idleTimer += static_cast<float>(dt);
	if (idleTimer > 1.f)
	{
		idleTimer = 0.f;
		m_go->sm->SetNextState("Searching");
	}
}

void StateAntWorkerIdle::Exit() {}

StateAntWorkerSearching::StateAntWorkerSearching(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}

StateAntWorkerSearching::~StateAntWorkerSearching() {}

void StateAntWorkerSearching::Enter()
{
	m_go->moveSpeed = 3.f;
	m_go->targetResource.SetZero();
}

void StateAntWorkerSearching::Update(double dt)
{
	// Search for food resources
	if (m_go->targetResource.IsZero())
	{
		// Random wandering while searching
		if ((m_go->pos - m_go->target).LengthSquared() < 0.1f)
		{
			float gridSize = SceneData::GetInstance()->GetGridSize();
			int gridNum = SceneData::GetInstance()->GetNumGrid();
			m_go->target.Set(
				Math::RandIntMinMax(1, gridNum - 2) * gridSize + gridSize * 0.5f,
				Math::RandIntMinMax(1, gridNum - 2) * gridSize + gridSize * 0.5f,
				0
			);
		}
	}
	else
	{
		// Found resource, transition to gathering
		m_go->sm->SetNextState("Gathering");
	}

	// Check for nearby enemies
	if (m_go->targetEnemy != nullptr)
	{
		m_go->sm->SetNextState("Fleeing");
	}
}

void StateAntWorkerSearching::Exit() {}

StateAntWorkerGathering::StateAntWorkerGathering(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}

StateAntWorkerGathering::~StateAntWorkerGathering() {}

void StateAntWorkerGathering::Enter()
{
	m_go->moveSpeed = 2.f;
	m_go->gatherTimer = 0.f;
	m_go->isCarryingResource = false;
}

void StateAntWorkerGathering::Update(double dt)
{
	if (m_go->targetEnemy != nullptr)
	{
		m_go->sm->SetNextState("Fleeing");
		return;
	}

	if (!m_go->isCarryingResource)
	{
		// Move to resource
		if (!m_go->targetResource.IsZero())
		{
			m_go->target = m_go->targetResource;
			if ((m_go->pos - m_go->targetResource).LengthSquared() < 1.f)
			{
				m_go->gatherTimer += static_cast<float>(dt);
				if (m_go->gatherTimer > 2.f)
				{
					m_go->isCarryingResource = true;
					m_go->carriedResources = 1;
					m_go->gatherTimer = 0.f;
					// Notify team
					PostOffice::GetInstance()->Send("Scene",
						new MessageResourceFound(m_go, m_go->targetResource, m_go->teamID));
				}
			}
		}
		else
		{
			m_go->sm->SetNextState("Searching");
		}
	}
	else
	{
		// Return to base
		m_go->target = m_go->homeBase;
		if ((m_go->pos - m_go->homeBase).LengthSquared() < 1.f)
		{
			// Deliver resources
			PostOffice::GetInstance()->Send("Scene",
				new MessageResourceDelivered(m_go, m_go->carriedResources, m_go->teamID));
			m_go->isCarryingResource = false;
			m_go->carriedResources = 0;
			m_go->targetResource.SetZero();
			m_go->sm->SetNextState("Idle");
		}
	}
}

void StateAntWorkerGathering::Exit() {}

StateAntWorkerFleeing::StateAntWorkerFleeing(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}

StateAntWorkerFleeing::~StateAntWorkerFleeing() {}

void StateAntWorkerFleeing::Enter()
{
	m_go->moveSpeed = 5.f;
	// Send help request
	PostOffice::GetInstance()->Send("Scene",
		new MessageRequestHelp(m_go, m_go->pos, m_go->teamID));
}

void StateAntWorkerFleeing::Update(double dt)
{
	if (m_go->targetEnemy != nullptr && m_go->targetEnemy->active)
	{
		// Flee away from enemy
		Vector3 fleeDir = m_go->pos - m_go->targetEnemy->pos;
		if (fleeDir.LengthSquared() > 0.1f)
		{
			fleeDir.Normalize();
			m_go->target = m_go->pos + fleeDir * SceneData::GetInstance()->GetGridSize() * 3.f;
		}
		else
		{
			m_go->target = m_go->homeBase;
		}

		// Check if safe
		if ((m_go->pos - m_go->targetEnemy->pos).LengthSquared() >
			m_go->detectionRange * m_go->detectionRange * 4.f)
		{
			m_go->targetEnemy = nullptr;
			m_go->sm->SetNextState("Idle");
		}
	}
	else
	{
		m_go->targetEnemy = nullptr;
		m_go->sm->SetNextState("Idle");
	}
}

void StateAntWorkerFleeing::Exit() {}

// ============= ANT SOLDIER STATES IMPLEMENTATION =============
StateAntSoldierPatrolling::StateAntSoldierPatrolling(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go), patrolTimer(0.f) {
}
StateAntSoldierPatrolling::~StateAntSoldierPatrolling() {}
void StateAntSoldierPatrolling::Enter()
{
	m_go->moveSpeed = 4.f;
	patrolTimer = 0.f;
	patrolTarget.SetZero();
}
void StateAntSoldierPatrolling::Update(double dt)
{
	patrolTimer += static_cast<float>(dt);
	// Check for enemies
	if (m_go->targetEnemy != nullptr && m_go->targetEnemy->active)
	{
		PostOffice::GetInstance()->Send("Scene",
			new MessageEnemySpotted(m_go, m_go->targetEnemy, m_go->teamID));
		m_go->sm->SetNextState("Attacking");
		return;
	}

	// Patrol around home base
	if (patrolTimer > 3.f || patrolTarget.IsZero() ||
		(m_go->pos - m_go->target).LengthSquared() < 0.5f)
	{
		patrolTimer = 0.f;
		float gridSize = SceneData::GetInstance()->GetGridSize();
		float range = 5.f;
		patrolTarget = m_go->homeBase + Vector3(
			Math::RandFloatMinMax(-range, range) * gridSize,
			Math::RandFloatMinMax(-range, range) * gridSize,
			0
		);
		m_go->target = patrolTarget;
	}
}

void StateAntSoldierPatrolling::Exit() {}
StateAntSoldierAttacking::StateAntSoldierAttacking(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go), attackCooldown(0.f) {
}
StateAntSoldierAttacking::~StateAntSoldierAttacking() {}
void StateAntSoldierAttacking::Enter()
{
	m_go->moveSpeed = 5.f;
	attackCooldown = 0.f;
}

void StateAntSoldierAttacking::Update(double dt)
{
	attackCooldown += static_cast<float>(dt);
	if (m_go->targetEnemy == nullptr || !m_go->targetEnemy->active)
	{
		m_go->targetEnemy = nullptr;
		m_go->sm->SetNextState("Defending");
		return;
	}

	// Chase enemy
	m_go->target = m_go->targetEnemy->pos;
	float distSq = (m_go->pos - m_go->targetEnemy->pos).LengthSquared();

	// Attack if in range
	if (distSq < m_go->attackRange * m_go->attackRange)
	{
		if (attackCooldown > 0.5f)
		{
			m_go->targetEnemy->health -= m_go->attackPower;
			attackCooldown = 0.f;

			if (m_go->targetEnemy->health <= 0.f)
			{
				PostOffice::GetInstance()->Send("Scene",
					new MessageUnitDied(m_go->targetEnemy, m_go->targetEnemy->teamID,
						m_go->targetEnemy->type));
				m_go->targetEnemy->active = false;
				m_go->targetEnemy = nullptr;
				m_go->sm->SetNextState("Defending");
			}
		}
	}

	// Retreat if low health
	if (m_go->health < m_go->maxHealth * 0.3f)
	{
		m_go->sm->SetNextState("Retreating");
	}
}

void StateAntSoldierAttacking::Exit() {}
StateAntSoldierDefending::StateAntSoldierDefending(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateAntSoldierDefending::~StateAntSoldierDefending() {}
void StateAntSoldierDefending::Enter()
{
	m_go->moveSpeed = 4.f;
	m_go->target = m_go->homeBase;
}
void StateAntSoldierDefending::Update(double dt)
{
	// Return to home base
	if ((m_go->pos - m_go->homeBase).LengthSquared() > 1.f)
	{
		m_go->target = m_go->homeBase;
	}
	// Check for new enemies
	if (m_go->targetEnemy != nullptr && m_go->targetEnemy->active)
	{
		m_go->sm->SetNextState("Attacking");
		return;
	}

	// Slowly heal at home
	if ((m_go->pos - m_go->homeBase).LengthSquared() < 4.f)
	{
		m_go->health = Math::Min(m_go->maxHealth, m_go->health + static_cast<float>(dt) * 0.5f);
	}

	// Resume patrol if healthy
	if (m_go->health >= m_go->maxHealth * 0.8f)
	{
		m_go->sm->SetNextState("Patrolling");
	}
}

void StateAntSoldierDefending::Exit() {}
StateAntSoldierRetreating::StateAntSoldierRetreating(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateAntSoldierRetreating::~StateAntSoldierRetreating() {}
void StateAntSoldierRetreating::Enter()
{
	m_go->moveSpeed = 6.f;
	PostOffice::GetInstance()->Send("Scene",
		new MessageRequestHelp(m_go, m_go->pos, m_go->teamID));
}
void StateAntSoldierRetreating::Update(double dt)
{
	// Flee to home base
	m_go->target = m_go->homeBase;
	// Heal when safe
	if ((m_go->pos - m_go->homeBase).LengthSquared() < 2.f)
	{
		m_go->health = Math::Min(m_go->maxHealth, m_go->health + static_cast<float>(dt) * 1.f);

		if (m_go->health >= m_go->maxHealth * 0.7f)
		{
			m_go->sm->SetNextState("Defending");
		}
	}
}

void StateAntSoldierRetreating::Exit() {}
// ============= ANT QUEEN STATES IMPLEMENTATION =============
StateAntQueenSpawning::StateAntQueenSpawning(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateAntQueenSpawning::~StateAntQueenSpawning() {}
void StateAntQueenSpawning::Enter()
{
	m_go->moveSpeed = 0.f;
	m_go->spawnCooldown = 0.f;
}
void StateAntQueenSpawning::Update(double dt)
{
	m_go->spawnCooldown += static_cast<float>(dt);
	// Check for threats
	if (m_go->targetEnemy != nullptr && m_go->targetEnemy->active)
	{
		PostOffice::GetInstance()->Send("Scene",
			new MessageQueenThreat(m_go, m_go->teamID));
		m_go->sm->SetNextState("Emergency");
		return;
	}

	// Spawn units periodically
	if (m_go->spawnCooldown > 5.f)
	{
		m_go->spawnCooldown = 0.f;

		// Alternate between workers and soldiers
		MessageSpawnUnit::UNIT_TYPE unitType = (m_go->unitsSpawned % 3 == 0) ?
			MessageSpawnUnit::UNIT_ANT_SOLDIER : MessageSpawnUnit::UNIT_ANT_WORKER;

		PostOffice::GetInstance()->Send("Scene",
			new MessageSpawnUnit(m_go, unitType, m_go->pos));

		m_go->unitsSpawned++;
		m_go->sm->SetNextState("Cooldown");
	}
}

void StateAntQueenSpawning::Exit() {}
StateAntQueenEmergency::StateAntQueenEmergency(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateAntQueenEmergency::~StateAntQueenEmergency() {}
void StateAntQueenEmergency::Enter()
{
	m_go->moveSpeed = 0.f;
	// Emergency spawn soldiers
	for (int i = 0; i < 3; ++i)
	{
		PostOffice::GetInstance()->Send("Scene",
			new MessageSpawnUnit(m_go, MessageSpawnUnit::UNIT_ANT_SOLDIER, m_go->pos));
	}
}
void StateAntQueenEmergency::Update(double dt)
{
	// Check if threat is gone
	if (m_go->targetEnemy == nullptr || !m_go->targetEnemy->active)
	{
		m_go->targetEnemy = nullptr;
		m_go->sm->SetNextState("Cooldown");
	}
}

void StateAntQueenEmergency::Exit() {}
StateAntQueenCooldown::StateAntQueenCooldown(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go), cooldownTimer(0.f) {
}
StateAntQueenCooldown::~StateAntQueenCooldown() {}
void StateAntQueenCooldown::Enter()
{
	cooldownTimer = 0.f;
}
void StateAntQueenCooldown::Update(double dt)
{
	cooldownTimer += static_cast<float>(dt);
	if (cooldownTimer > 3.f)
	{
		// Check population
		int teamUnits = SceneData::GetInstance()->GetObjectCount(); // Simplified
		if (teamUnits < 10)
		{
			m_go->sm->SetNextState("Boosting");
		}
		else
		{
			m_go->sm->SetNextState("Spawning");
		}
	}
}

void StateAntQueenCooldown::Exit() {}
StateAntQueenBoosting::StateAntQueenBoosting(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go), boostCount(0) {
}
StateAntQueenBoosting::~StateAntQueenBoosting() {}
void StateAntQueenBoosting::Enter()
{
	boostCount = 0;
}
void StateAntQueenBoosting::Update(double dt)
{
	m_go->spawnCooldown += static_cast<float>(dt);
	if (m_go->spawnCooldown > 2.f && boostCount < 3)
	{
		m_go->spawnCooldown = 0.f;
		PostOffice::GetInstance()->Send("Scene",
			new MessageSpawnUnit(m_go, MessageSpawnUnit::UNIT_ANT_WORKER, m_go->pos));
		boostCount++;
	}

	if (boostCount >= 3)
	{
		m_go->sm->SetNextState("Cooldown");
	}
}

void StateAntQueenBoosting::Exit() {}
// ============= BEETLE WORKER STATES IMPLEMENTATION =============
StateBeetleWorkerIdle::StateBeetleWorkerIdle(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateBeetleWorkerIdle::~StateBeetleWorkerIdle() {}
void StateBeetleWorkerIdle::Enter()
{
	m_go->moveSpeed = 0.f;
}
void StateBeetleWorkerIdle::Update(double dt)
{
	static float idleTimer = 0.f;
	idleTimer += static_cast<float>(dt);
	if (idleTimer > 1.5f)
	{
		idleTimer = 0.f;
		m_go->sm->SetNextState("Foraging");
	}
}
void StateBeetleWorkerIdle::Exit() {}
StateBeetleWorkerForaging::StateBeetleWorkerForaging(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateBeetleWorkerForaging::~StateBeetleWorkerForaging() {}
void StateBeetleWorkerForaging::Enter()
{
	m_go->moveSpeed = 2.5f;
	m_go->targetResource.SetZero();
}

void StateBeetleWorkerForaging::Update(double dt)
{
	if (m_go->targetResource.IsZero())
	{
		if ((m_go->pos - m_go->target).LengthSquared() < 0.1f)
		{
			float gridSize = SceneData::GetInstance()->GetGridSize();
			int gridNum = SceneData::GetInstance()->GetNumGrid();
			m_go->target.Set(
				Math::RandIntMinMax(1, gridNum - 2) * gridSize + gridSize * 0.5f,
				Math::RandIntMinMax(1, gridNum - 2) * gridSize + gridSize * 0.5f,
				0
			);
		}
	}
	else
	{
		m_go->sm->SetNextState("Collecting");
	}
	if (m_go->targetEnemy != nullptr)
	{
		m_go->sm->SetNextState("Escaping");
	}
}

void StateBeetleWorkerForaging::Exit() {}
StateBeetleWorkerCollecting::StateBeetleWorkerCollecting(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateBeetleWorkerCollecting::~StateBeetleWorkerCollecting() {}
void StateBeetleWorkerCollecting::Enter()
{
	m_go->moveSpeed = 2.f;
	m_go->gatherTimer = 0.f;
	m_go->isCarryingResource = false;
}
void StateBeetleWorkerCollecting::Update(double dt)
{
	if (m_go->targetEnemy != nullptr)
	{
		m_go->sm->SetNextState("Escaping");
		return;
	}
	if (!m_go->isCarryingResource)
	{
		if (!m_go->targetResource.IsZero())
		{
			m_go->target = m_go->targetResource;
			if ((m_go->pos - m_go->targetResource).LengthSquared() < 1.f)
			{
				m_go->gatherTimer += static_cast<float>(dt);
				if (m_go->gatherTimer > 2.5f)
				{
					m_go->isCarryingResource = true;
					m_go->carriedResources = 1;
					PostOffice::GetInstance()->Send("Scene",
						new MessageResourceFound(m_go, m_go->targetResource, m_go->teamID));
				}
			}
		}
		else
		{
			m_go->sm->SetNextState("Foraging");
		}
	}
	else
	{
		m_go->target = m_go->homeBase;
		if ((m_go->pos - m_go->homeBase).LengthSquared() < 1.f)
		{
			PostOffice::GetInstance()->Send("Scene",
				new MessageResourceDelivered(m_go, m_go->carriedResources, m_go->teamID));
			m_go->isCarryingResource = false;
			m_go->carriedResources = 0;
			m_go->targetResource.SetZero();
			m_go->sm->SetNextState("Idle");
		}
	}
}

void StateBeetleWorkerCollecting::Exit() {}
StateBeetleWorkerEscaping::StateBeetleWorkerEscaping(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateBeetleWorkerEscaping::~StateBeetleWorkerEscaping() {}
void StateBeetleWorkerEscaping::Enter()
{
	m_go->moveSpeed = 4.5f;
	PostOffice::GetInstance()->Send("Scene",
		new MessageRequestHelp(m_go, m_go->pos, m_go->teamID));
}
void StateBeetleWorkerEscaping::Update(double dt)
{
	if (m_go->targetEnemy != nullptr && m_go->targetEnemy->active)
	{
		Vector3 escapeDir = m_go->pos - m_go->targetEnemy->pos;
		if (escapeDir.LengthSquared() > 0.1f)
		{
			escapeDir.Normalize();
			m_go->target = m_go->pos + escapeDir * SceneData::GetInstance()->GetGridSize() * 3.f;
		}
		if ((m_go->pos - m_go->targetEnemy->pos).LengthSquared() >
			m_go->detectionRange * m_go->detectionRange * 4.f)
		{
			m_go->targetEnemy = nullptr;
			m_go->sm->SetNextState("Idle");
		}
	}
	else
	{
		m_go->targetEnemy = nullptr;
		m_go->sm->SetNextState("Idle");
	}
}

void StateBeetleWorkerEscaping::Exit() {}
// ============= BEETLE WARRIOR STATES IMPLEMENTATION =============
StateBeetleWarriorHunting::StateBeetleWarriorHunting(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateBeetleWarriorHunting::~StateBeetleWarriorHunting() {}
void StateBeetleWarriorHunting::Enter()
{
	m_go->moveSpeed = 5.f;
	huntTarget.SetZero();
}
void StateBeetleWarriorHunting::Update(double dt)
{
	if (m_go->targetEnemy != nullptr && m_go->targetEnemy->active)
	{
		PostOffice::GetInstance()->Send("Scene",
			new MessageEnemySpotted(m_go, m_go->targetEnemy, m_go->teamID));
		m_go->sm->SetNextState("Combat");
		return;
	}
	// Aggressive patrol towards enemy territory
	if (huntTarget.IsZero() || (m_go->pos - m_go->target).LengthSquared() < 0.5f)
	{
		float gridSize = SceneData::GetInstance()->GetGridSize();
		int gridNum = SceneData::GetInstance()->GetNumGrid();
		// Patrol towards lower-left (ant territory)
		huntTarget.Set(
			Math::RandIntMinMax(0, gridNum / 2) * gridSize + gridSize * 0.5f,
			Math::RandIntMinMax(0, gridNum / 2) * gridSize + gridSize * 0.5f,
			0
		);
		m_go->target = huntTarget;
	}
}

void StateBeetleWarriorHunting::Exit() {}
StateBeetleWarriorCombat::StateBeetleWarriorCombat(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go), attackTimer(0.f) {
}
StateBeetleWarriorCombat::~StateBeetleWarriorCombat() {}
void StateBeetleWarriorCombat::Enter()
{
	m_go->moveSpeed = 6.f;
	attackTimer = 0.f;
}
void StateBeetleWarriorCombat::Update(double dt)
{
	attackTimer += static_cast<float>(dt);
	if (m_go->targetEnemy == nullptr || !m_go->targetEnemy->active)
	{
		m_go->targetEnemy = nullptr;
		m_go->sm->SetNextState("Resting");
		return;
	}

	m_go->target = m_go->targetEnemy->pos;
	float distSq = (m_go->pos - m_go->targetEnemy->pos).LengthSquared();

	if (distSq < m_go->attackRange * m_go->attackRange)
	{
		if (attackTimer > 0.4f)
		{
			m_go->targetEnemy->health -= m_go->attackPower;
			attackTimer = 0.f;

			if (m_go->targetEnemy->health <= 0.f)
			{
				PostOffice::GetInstance()->Send("Scene",
					new MessageUnitDied(m_go->targetEnemy, m_go->targetEnemy->teamID,
						m_go->targetEnemy->type));
				m_go->targetEnemy->active = false;
				m_go->targetEnemy = nullptr;
				m_go->sm->SetNextState("Resting");
			}
		}
	}

	if (m_go->health < m_go->maxHealth * 0.25f)
	{
		m_go->sm->SetNextState("Withdrawing");
	}
}

void StateBeetleWarriorCombat::Exit() {}
StateBeetleWarriorResting::StateBeetleWarriorResting(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go), restTimer(0.f) {
}
StateBeetleWarriorResting::~StateBeetleWarriorResting() {}
void StateBeetleWarriorResting::Enter()
{
	m_go->moveSpeed = 2.f;
	restTimer = 0.f;
	m_go->target = m_go->homeBase;
}
void StateBeetleWarriorResting::Update(double dt)
{
	restTimer += static_cast<float>(dt);
	// Move towards home
	if ((m_go->pos - m_go->homeBase).LengthSquared() > 2.f)
	{
		m_go->target = m_go->homeBase;
	}

	// Heal slowly
	m_go->health = Math::Min(m_go->maxHealth, m_go->health + static_cast<float>(dt) * 0.3f);

	// Resume hunting
	if (restTimer > 4.f && m_go->health > m_go->maxHealth * 0.6f)
	{
		m_go->sm->SetNextState("Hunting");
	}

	// Respond to new threats
	if (m_go->targetEnemy != nullptr && m_go->targetEnemy->active)
	{
		m_go->sm->SetNextState("Combat");
	}
}

void StateBeetleWarriorResting::Exit() {}
StateBeetleWarriorWithdrawing::StateBeetleWarriorWithdrawing(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateBeetleWarriorWithdrawing::~StateBeetleWarriorWithdrawing() {}
void StateBeetleWarriorWithdrawing::Enter()
{
	m_go->moveSpeed = 7.f;
	PostOffice::GetInstance()->Send("Scene",
		new MessageRequestHelp(m_go, m_go->pos, m_go->teamID));
}
void StateBeetleWarriorWithdrawing::Update(double dt)
{
	m_go->target = m_go->homeBase;
	if ((m_go->pos - m_go->homeBase).LengthSquared() < 3.f)
	{
		m_go->health = Math::Min(m_go->maxHealth, m_go->health + static_cast<float>(dt) * 0.8f);

		if (m_go->health >= m_go->maxHealth * 0.6f)
		{
			m_go->sm->SetNextState("Resting");
		}
	}
}

void StateBeetleWarriorWithdrawing::Exit() {}
// ============= BEETLE QUEEN STATES IMPLEMENTATION =============
StateBeetleQueenProducing::StateBeetleQueenProducing(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateBeetleQueenProducing::~StateBeetleQueenProducing() {}
void StateBeetleQueenProducing::Enter()
{
	m_go->moveSpeed = 0.f;
	m_go->spawnCooldown = 0.f;
}
void StateBeetleQueenProducing::Update(double dt)
{
	m_go->spawnCooldown += static_cast<float>(dt);
	if (m_go->targetEnemy != nullptr && m_go->targetEnemy->active)
	{
		PostOffice::GetInstance()->Send("Scene",
			new MessageQueenThreat(m_go, m_go->teamID));
		m_go->sm->SetNextState("Alert");
		return;
	}

	if (m_go->spawnCooldown > 4.5f)
	{
		m_go->spawnCooldown = 0.f;

		MessageSpawnUnit::UNIT_TYPE unitType = (m_go->unitsSpawned % 2 == 0) ?
			MessageSpawnUnit::UNIT_BEETLE_WARRIOR : MessageSpawnUnit::UNIT_BEETLE_WORKER;

		PostOffice::GetInstance()->Send("Scene",
			new MessageSpawnUnit(m_go, unitType, m_go->pos));

		m_go->unitsSpawned++;
		m_go->sm->SetNextState("Waiting");
	}
}

void StateBeetleQueenProducing::Exit() {}
StateBeetleQueenAlert::StateBeetleQueenAlert(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go) {
}
StateBeetleQueenAlert::~StateBeetleQueenAlert() {}
void StateBeetleQueenAlert::Enter()
{
	m_go->moveSpeed = 0.f;
	// Emergency warriors
	for (int i = 0; i < 2; ++i)
	{
		PostOffice::GetInstance()->Send("Scene",
			new MessageSpawnUnit(m_go, MessageSpawnUnit::UNIT_BEETLE_WARRIOR, m_go->pos));
	}
}
void StateBeetleQueenAlert::Update(double dt)
{
	if (m_go->targetEnemy == nullptr || !m_go->targetEnemy->active)
	{
		m_go->targetEnemy = nullptr;
		m_go->sm->SetNextState("Waiting");
	}
}

void StateBeetleQueenAlert::Exit() {}
StateBeetleQueenWaiting::StateBeetleQueenWaiting(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go), waitTimer(0.f) {
}
StateBeetleQueenWaiting::~StateBeetleQueenWaiting() {}
void StateBeetleQueenWaiting::Enter()
{
	waitTimer = 0.f;
}
void StateBeetleQueenWaiting::Update(double dt)
{
	waitTimer += static_cast<float>(dt);
	if (waitTimer > 2.5f)
	{
		int teamUnits = SceneData::GetInstance()->GetObjectCount();
		if (teamUnits < 15)
		{
			m_go->sm->SetNextState("Warmode");
		}
		else
		{
			m_go->sm->SetNextState("Producing");
		}
	}
}

void StateBeetleQueenWaiting::Exit() {}
StateBeetleQueenWarmode::StateBeetleQueenWarmode(const std::string& stateID, GameObject* go)
	: State(stateID), m_go(go), warCount(0) {
}
StateBeetleQueenWarmode::~StateBeetleQueenWarmode() {}
void StateBeetleQueenWarmode::Enter()
{
	warCount = 0;
}
void StateBeetleQueenWarmode::Update(double dt)
{
	m_go->spawnCooldown += static_cast<float>(dt);
	if (m_go->spawnCooldown > 1.5f && warCount < 4)
	{
		m_go->spawnCooldown = 0.f;
		PostOffice::GetInstance()->Send("Scene",
			new MessageSpawnUnit(m_go, MessageSpawnUnit::UNIT_BEETLE_WARRIOR, m_go->pos));
		warCount++;
	}

	if (warCount >= 4)
	{
		m_go->sm->SetNextState("Waiting");
	}
}

void StateBeetleQueenWarmode::Exit() {}