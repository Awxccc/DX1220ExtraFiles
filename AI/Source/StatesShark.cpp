#include "StatesShark.h"
#include "SceneData.h"

static const float CRAZY_SPEED = 16.f;
static const float NAUGHTY_SPEED = 12.f;
static const float HAPPY_SPEED = 8.f;

StateCrazy::StateCrazy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateCrazy::~StateCrazy()
{
}

void StateCrazy::Enter()
{
	m_go->moveSpeed = CRAZY_SPEED;
	m_go->nearest = NULL;
}

void StateCrazy::Update(double dt)
{
	if (SceneData::GetInstance()->GetFishCount() < 12)
		m_go->sm->SetNextState("Naughty");
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

void StateCrazy::Exit()
{
}

StateNaughty::StateNaughty(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateNaughty::~StateNaughty()
{
}

void StateNaughty::Enter()
{
	m_go->moveSpeed = NAUGHTY_SPEED;
	m_go->nearest = NULL;
}

void StateNaughty::Update(double dt)
{
	if (SceneData::GetInstance()->GetFishCount() > 10)
		m_go->sm->SetNextState("Crazy");
	else if(SceneData::GetInstance()->GetFishCount() < 6)
		m_go->sm->SetNextState("Happy");
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

void StateNaughty::Exit()
{
}

StateHappy::StateHappy(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateHappy::~StateHappy()
{
}

void StateHappy::Enter()
{
	m_go->moveSpeed = HAPPY_SPEED;
	m_go->moveLeft = m_go->moveRight = m_go->moveUp = m_go->moveDown = true;
}

void StateHappy::Update(double dt)
{
	if (SceneData::GetInstance()->GetFishCount() > 4)
		m_go->sm->SetNextState("Naughty");
}

void StateHappy::Exit()
{
}
