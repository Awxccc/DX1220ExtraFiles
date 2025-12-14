#pragma once

#include "State.h"
#include "GameObject.h"
#include "Vector3.h"

// ================= WORKER STATES =================
class StateWorkerIdle : public State
{
	GameObject* m_go;
public:
	StateWorkerIdle(const std::string& stateID, GameObject* go);
	virtual ~StateWorkerIdle();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateWorkerSearching : public State
{
	GameObject* m_go;
public:
	StateWorkerSearching(const std::string& stateID, GameObject* go);
	virtual ~StateWorkerSearching();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateWorkerGathering : public State
{
	GameObject* m_go;
public:
	StateWorkerGathering(const std::string& stateID, GameObject* go);
	virtual ~StateWorkerGathering();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateWorkerFleeing : public State
{
	GameObject* m_go;
public:
	StateWorkerFleeing(const std::string& stateID, GameObject* go);
	virtual ~StateWorkerFleeing();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// ================= SOLDIER STATES =================
class StateSoldierPatrolling : public State
{
	GameObject* m_go;
	float patrolTimer;
	Vector3 patrolTarget;
public:
	StateSoldierPatrolling(const std::string& stateID, GameObject* go);
	virtual ~StateSoldierPatrolling();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateSoldierAttacking : public State
{
	GameObject* m_go;
	float attackCooldown;
public:
	StateSoldierAttacking(const std::string& stateID, GameObject* go);
	virtual ~StateSoldierAttacking();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateSoldierResting : public State // Merged Defending/Resting
{
	GameObject* m_go;
	float restTimer;
public:
	StateSoldierResting(const std::string& stateID, GameObject* go);
	virtual ~StateSoldierResting();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateSoldierRetreating : public State
{
	GameObject* m_go;
public:
	StateSoldierRetreating(const std::string& stateID, GameObject* go);
	virtual ~StateSoldierRetreating();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// ================= QUEEN STATES =================
class StateQueenSpawning : public State
{
	GameObject* m_go;
public:
	StateQueenSpawning(const std::string& stateID, GameObject* go);
	virtual ~StateQueenSpawning();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateQueenEmergency : public State
{
	GameObject* m_go;
public:
	StateQueenEmergency(const std::string& stateID, GameObject* go);
	virtual ~StateQueenEmergency();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateQueenCooldown : public State
{
	GameObject* m_go;
	float timer;
public:
	StateQueenCooldown(const std::string& stateID, GameObject* go);
	virtual ~StateQueenCooldown();
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};