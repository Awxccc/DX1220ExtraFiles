#ifndef STATES_FISHFOOD_H
#define STATES_FISHFOOD_H

#include "State.h"
#include "GameObject.h"

//week 5
//these 2 states are meant for fishfood

class StateEvolve : public State
{
	GameObject* m_go;
public:
	StateEvolve(const std::string& stateID, GameObject* go = NULL);
	~StateEvolve() {};

	void Enter();
	void Update(double dt);
	void Exit();
};

class StateGrow : public State
{
	GameObject* m_go;
public:
	StateGrow(const std::string& stateID, GameObject* go = NULL);
	~StateGrow() {};

	void Enter();
	void Update(double dt);
	void Exit();
};

#endif