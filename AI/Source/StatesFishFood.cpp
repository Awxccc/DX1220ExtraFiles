#include "StatesFishFood.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

StateEvolve::StateEvolve(const std::string& stateID, GameObject* go)
	: State(stateID)
	, m_go(go)
{
}

void StateEvolve::Enter()
{
	PostOffice::GetInstance()->Send("Scene", new MessageEvolve(m_go));
}

void StateEvolve::Update(double dt)
{
}

void StateEvolve::Exit()
{
}

StateGrow::StateGrow(const std::string& stateID, GameObject* go)
	: State(stateID)
	, m_go(go)
{
}

void StateGrow::Enter()
{
	m_go->countDown = 0;
}

void StateGrow::Update(double dt)
{
	m_go->countDown += static_cast<float>(dt);
	if (m_go->countDown >= 15.f) //after 15 seconds, make fishfood evolve
	{
		m_go->sm->SetNextState("Evolve");
	}
}

void StateGrow::Exit()
{
}
