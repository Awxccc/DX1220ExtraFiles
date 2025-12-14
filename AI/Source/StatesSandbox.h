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
class StateAntSoldierPatrolling : public State
{
	GameObject* m_go;
	float patrolTimer;
	Vector3 patrolTarget;
public:
	StateAntSoldierPatrolling(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntSoldierPatrolling();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntSoldierAttacking : public State
{
	GameObject* m_go;
	float attackCooldown;
public:
	StateAntSoldierAttacking(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntSoldierAttacking();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntSoldierDefending : public State
{
	GameObject* m_go;
public:
	StateAntSoldierDefending(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntSoldierDefending();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAntSoldierRetreating : public State
{
	GameObject* m_go;
public:
	StateAntSoldierRetreating(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateAntSoldierRetreating();
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
class StateBeetleWorkerIdle : public State
{
	GameObject* m_go;
public:
	StateBeetleWorkerIdle(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleWorkerIdle();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleWorkerForaging : public State
{
	GameObject* m_go;
public:
	StateBeetleWorkerForaging(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleWorkerForaging();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleWorkerCollecting : public State
{
	GameObject* m_go;
public:
	StateBeetleWorkerCollecting(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleWorkerCollecting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleWorkerEscaping : public State
{
	GameObject* m_go;
public:
	StateBeetleWorkerEscaping(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleWorkerEscaping();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// ============= BEETLE WARRIOR STATES =============
class StateBeetleWarriorHunting : public State
{
	GameObject* m_go;
	Vector3 huntTarget;
public:
	StateBeetleWarriorHunting(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleWarriorHunting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleWarriorCombat : public State
{
	GameObject* m_go;
	float attackTimer;
public:
	StateBeetleWarriorCombat(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleWarriorCombat();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleWarriorResting : public State
{
	GameObject* m_go;
	float restTimer;
public:
	StateBeetleWarriorResting(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleWarriorResting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateBeetleWarriorWithdrawing : public State
{
	GameObject* m_go;
public:
	StateBeetleWarriorWithdrawing(const std::string& stateID, GameObject* go = NULL);
	virtual ~StateBeetleWarriorWithdrawing();
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