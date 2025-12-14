#include "StatesFish.h"
// Exercise Week 05
#include "PostOffice.h"
#include "ConcreteMessages.h"

static const float ENERGY_DROP_RATE = 0.2f;
static const float FULL_SPEED = 8.f;
static const float HUNGRY_SPEED = 4.f;

StateTooFull::StateTooFull(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateTooFull::~StateTooFull()
{
}

void StateTooFull::Enter()
{
	m_go->moveSpeed = 0;
}

void StateTooFull::Update(double dt)
{
	m_go->energy -= ENERGY_DROP_RATE * static_cast<float>(dt);
	if (m_go->energy < 10.f)
		m_go->sm->SetNextState("Full");
}

void StateTooFull::Exit()
{
}

StateFull::StateFull(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateFull::~StateFull()
{
}

void StateFull::Enter()
{
	m_go->moveSpeed = FULL_SPEED;
	m_go->nearest = NULL;
}

void StateFull::Update(double dt)
{
	m_go->energy -= ENERGY_DROP_RATE * static_cast<float>(dt);
	if (m_go->energy >= 10.f)
		m_go->sm->SetNextState("TooFull");
	else if (m_go->energy < 5.f)
		m_go->sm->SetNextState("Hungry");
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = true;
	if (m_go->nearest)
	{
		if (m_go->nearest->pos.x > m_go->pos.x)
			m_go->moveRight = false;
		else
			m_go->moveLeft = false;
		if (m_go->nearest->pos.y > m_go->pos.y)
			m_go->moveUp = false;
		else
			m_go->moveDown = false;
	}
}

void StateFull::Exit()
{
}

StateHungry::StateHungry(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateHungry::~StateHungry()
{
}

void StateHungry::Enter()
{
	m_go->moveSpeed = HUNGRY_SPEED;
	m_go->nearest = NULL;

	// Exercise Week 05
	int range[2] = { -3, 3 };
	PostOffice::GetInstance()->Send("Scene", new MessageSpawnFood(m_go, GameObject::GO_FISHFOOD, 2, range));
}

void StateHungry::Update(double dt)
{
	m_go->energy -= ENERGY_DROP_RATE * static_cast<float>(dt);
	if (m_go->energy >= 5.f)
		m_go->sm->SetNextState("Full");
	else if (m_go->energy < 0.f)
	{
		m_go->sm->SetNextState("Dead");
	}
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = true;
	if (m_go->nearest)
	{
		if (m_go->nearest->pos.x > m_go->pos.x)
			m_go->moveLeft = false;
		else
			m_go->moveRight = false;
		if (m_go->nearest->pos.y > m_go->pos.y)
			m_go->moveDown = false;
		else
			m_go->moveUp = false;
	}
}

void StateHungry::Exit()
{
}

StateDead::StateDead(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateDead::~StateDead()
{
}

void StateDead::Enter()
{
	m_go->countDown = 3.f;
	m_go->moveSpeed = 0;
}

void StateDead::Update(double dt)
{
	m_go->countDown -= static_cast<float>(dt);
	if (m_go->countDown < 0)
	{
		m_go->active = false;
	}
}

void StateDead::Exit()
{
}
