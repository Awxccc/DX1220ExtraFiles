#ifndef CONCRETE_MESSAGE_H
#define CONCRETE_MESSAGE_H

#include "Message.h"
#include "GameObject.h"

struct MessageWRU : public Message
{
	enum SEARCH_TYPE
	{
		SEARCH_NONE = 0,
		NEAREST_SHARK,
		NEAREST_FISHFOOD,
		NEAREST_FULLFISH,
		HIGHEST_ENERGYFISH,
	};
	MessageWRU(GameObject *goValue, SEARCH_TYPE typeValue, float thresholdValue) : go(goValue), type(typeValue), threshold(thresholdValue) {}
	virtual ~MessageWRU() {}

	GameObject *go;
	SEARCH_TYPE type;
	float threshold;
};

struct MessageCheckActive : public Message
{
	MessageCheckActive() {}
};

struct MessageCheckFish : public Message
{
	MessageCheckFish() {}
};

struct MessageCheckFood : public Message
{
	MessageCheckFood() {}
};

struct MessageCheckShark : public Message
{
	MessageCheckShark() {}
};

//week 5
//this message asks the scene to spawn an object
struct MessageSpawn : public Message
{
	// owner of msg, what to spawn, # to spawn, # tiles(x & y) from owner
	// passing range array by reference to avoid array decay (to int*) - that way we can force users to only pass an array of size 2(no other sizes will be accepted)
	// alternatively, look into std::array(c++11 onwards)?
	MessageSpawn(GameObject* goVal, int typeVal, int countVal, int (&range)[2]) : go(goVal), type(typeVal), count(countVal)
	{
		distRange[0] = range[0];
		distRange[1] = range[1];
	}

	int distRange[2];
	int type;
	int count;
	GameObject* go;
};

struct MessageSpawnFood : public Message
{
	// owner of msg, what to spawn, # to spawn, # tiles(x & y) from owner
	// passing range array by reference to avoid array decay (to int*) - that way we can force users to only pass an array of size 2(no other sizes will be accepted)
	// alternatively, look into std::array(c++11 onwards)?
	MessageSpawnFood(GameObject* goVal, int typeVal, int countVal, int(&range)[2]) : go(goVal), type(typeVal), count(countVal)
	{
		distRange[0] = range[0];
		distRange[1] = range[1];
	}

	int distRange[2];
	int type;
	int count;
	GameObject* go;
};

struct MessageStop : public Message
{
	MessageStop() {}
};

//this message is meant to turn food into fish
struct MessageEvolve : public Message
{
	MessageEvolve(GameObject* goVal) : go(goVal) {}

	GameObject* go;
};

//Assignment 1 stuff

struct MessageSpawnUnit : public Message
{
	enum UNIT_TYPE
	{
		UNIT_SPEEDY_ANT_WORKER,
		UNIT_SPEEDY_ANT_SOLDIER,
		UNIT_STRONG_ANT_WORKER,
		UNIT_STRONG_ANT_SOLDIER,
		UNIT_HEALER,
        UNIT_SCOUT,
        UNIT_TANK,
		UNIT_PHEROMONE
	};
	MessageSpawnUnit(GameObject* goValue, UNIT_TYPE unitType, Vector3 spawnPos)
		: spawner(goValue), type(unitType), position(spawnPos) {
	}
	virtual ~MessageSpawnUnit() {}

	GameObject* spawner;
	UNIT_TYPE type;
	Vector3 position;
};

struct MessageResourceFound : public Message
{
	MessageResourceFound(GameObject* finder, Vector3 resourcePos, int team)
		: discoverer(finder), position(resourcePos), teamID(team) {
	}
	virtual ~MessageResourceFound() {}

	GameObject* discoverer;
	Vector3 position;
	int teamID;
};

struct MessageEnemySpotted : public Message
{
	MessageEnemySpotted(GameObject* spotter, GameObject* target, int team)
		: scout(spotter), enemy(target), teamID(team) {
	}
	virtual ~MessageEnemySpotted() {}

	GameObject* scout;
	GameObject* enemy;
	int teamID;
};

struct MessageRequestHelp : public Message
{
	MessageRequestHelp(GameObject* caller, Vector3 pos, int team)
		: requester(caller), position(pos), teamID(team) {
	}
	virtual ~MessageRequestHelp() {}

	GameObject* requester;
	Vector3 position;
	int teamID;
};

struct MessageResourceDelivered : public Message
{
	MessageResourceDelivered(GameObject* deliverer, int amount, int team)
		: worker(deliverer), resourceAmount(amount), teamID(team) {
	}
	virtual ~MessageResourceDelivered() {}

	GameObject* worker;
	int resourceAmount;
	int teamID;
};

struct MessageUnitDied : public Message
{
	MessageUnitDied(GameObject* deceased, int team, GameObject::GAMEOBJECT_TYPE unitType)
		: unit(deceased), teamID(team), type(unitType) {
	}
	virtual ~MessageUnitDied() {}

	GameObject* unit;
	int teamID;
	GameObject::GAMEOBJECT_TYPE type;
};

struct MessageTerritoryClaimed : public Message
{
	MessageTerritoryClaimed(int team, Vector3 pos)
		: teamID(team), position(pos) {
	}
	virtual ~MessageTerritoryClaimed() {}

	int teamID;
	Vector3 position;
};

struct MessageQueenThreat : public Message
{
	MessageQueenThreat(GameObject* queenUnit, int team)
		: queen(queenUnit), teamID(team) {
	}
	virtual ~MessageQueenThreat() {}

	GameObject* queen;
	int teamID;
};

#endif
