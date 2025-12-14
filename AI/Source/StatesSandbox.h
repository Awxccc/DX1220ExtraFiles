#pragma once

#include "State.h"
#include "GameObject.h"

// ============= ANT WORKER STATES =============
class StateAntWorkerIdle : public State
{
	GameObject* m_go;
public:
	StateAntWorkerIdle(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntWorkerIdle();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntWorkerSearching : public State
{
	GameObject* m_go;
public:
	StateAntWorkerSearching(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntWorkerSearching();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntWorkerGathering : public State
{
	GameObject* m_go;
public:
	StateAntWorkerGathering(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntWorkerGathering();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntWorkerFleeing : public State
{
	GameObject* m_go;
public:
	StateAntWorkerFleeing(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntWorkerFleeing();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// ============= ANT SOLDIER STATES =============
class StateSpeedyAntSoldierPatrolling : public State
{
	GameObject* m_go;
	float patrolTimer;
	Vector3 patrolTarget;
public:
	StateSpeedyAntSoldierPatrolling(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateSpeedyAntSoldierPatrolling();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateSpeedyAntSoldierAttacking : public State
{
	GameObject* m_go;
	float attackCooldown;
public:
	StateSpeedyAntSoldierAttacking(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateSpeedyAntSoldierAttacking();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateSpeedyAntSoldierDefending : public State
{
	GameObject* m_go;
public:
	StateSpeedyAntSoldierDefending(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateSpeedyAntSoldierDefending();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateSpeedyAntSoldierRetreating : public State
{
	GameObject* m_go;
public:
	StateSpeedyAntSoldierRetreating(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateSpeedyAntSoldierRetreating();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// ============= ANT QUEEN STATES =============
class StateAntQueenSpawning : public State
{
	GameObject* m_go;
public:
	StateAntQueenSpawning(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntQueenSpawning();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntQueenEmergency : public State
{
	GameObject* m_go;
public:
	StateAntQueenEmergency(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntQueenEmergency();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntQueenCooldown : public State
{
	GameObject* m_go;
	float cooldownTimer;
public:
	StateAntQueenCooldown(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntQueenCooldown();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntQueenBoosting : public State
{
	GameObject* m_go;
	int boostCount;
public:
	StateAntQueenBoosting(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntQueenBoosting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// ============= BEETLE WORKER STATES =============
class StateStrongAntWorkerIdle : public State
{
	GameObject* m_go;
public:
	StateStrongAntWorkerIdle(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateStrongAntWorkerIdle();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateStrongAntWorkerForaging : public State
{
	GameObject* m_go;
public:
	StateStrongAntWorkerForaging(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateStrongAntWorkerForaging();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateStrongAntWorkerCollecting : public State
{
	GameObject* m_go;
public:
	StateStrongAntWorkerCollecting(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateStrongAntWorkerCollecting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateStrongAntWorkerEscaping : public State
{
	GameObject* m_go;
public:
	StateStrongAntWorkerEscaping(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateStrongAntWorkerEscaping();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// ============= BEETLE WARRIOR STATES =============
class StateStrongAntSoldierHunting : public State
{
	GameObject* m_go;
	Vector3 huntTarget;
public:
	StateStrongAntSoldierHunting(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateStrongAntSoldierHunting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateStrongAntSoldierCombat : public State
{
	GameObject* m_go;
	float attackTimer;
public:
	StateStrongAntSoldierCombat(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateStrongAntSoldierCombat();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateStrongAntSoldierResting : public State
{
	GameObject* m_go;
	float restTimer;
public:
	StateStrongAntSoldierResting(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateStrongAntSoldierResting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateStrongAntSoldierWithdrawing : public State
{
	GameObject* m_go;
public:
	StateStrongAntSoldierWithdrawing(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateStrongAntSoldierWithdrawing();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// ============= BEETLE QUEEN STATES =============
class StateBeetleQueenProducing : public State
{
	GameObject* m_go;
public:
	StateBeetleQueenProducing(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleQueenProducing();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleQueenAlert : public State
{
	GameObject* m_go;
public:
	StateBeetleQueenAlert(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleQueenAlert();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleQueenWaiting : public State
{
	GameObject* m_go;
	float waitTimer;
public:
	StateBeetleQueenWaiting(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleQueenWaiting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleQueenWarmode : public State
{
	GameObject* m_go;
	int warCount;
public:
	StateBeetleQueenWarmode(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleQueenWarmode();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};