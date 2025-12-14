#ifndef SCENE_MOVEMENT_WEEK05_H
#define SCENE_MOVEMENT_WEEK05_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "ObjectBase.h"

class SceneMovement_Week05 : public SceneBase, public ObjectBase
{
public:
	SceneMovement_Week05();
	~SceneMovement_Week05();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);
	bool Handle(Message* message);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
protected:

	std::vector<GameObject *> m_goList;
	std::vector<GameObject*> m_goList_Add;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ghost;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;
	float m_hourOfTheDay;
	int m_numGO[GameObject::GO_TOTAL];
	float zOffset;
};

#endif