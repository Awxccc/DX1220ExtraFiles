#ifndef SCENE_DATA
#define SCENE_DATA

#include "SingletonTemplate.h"

class SceneData : public Singleton<SceneData>
{
	friend Singleton<SceneData>;
public:
	int GetObjectCount() const;
	void SetObjectCount(const int objectCount);
	int GetFishCount() const;
	void SetFishCount(const int fishCount);
	int GetNumGrid() const;
	void SetNumGrid(const int numGrid);
	float GetGridSize() const;
	void SetGridSize(const float gridSize);
	float GetGridOffset() const;
	void SetGridOffset(const float gridOffset);

private:
	SceneData();
	~SceneData();
	int m_objectCount;
	int m_fishCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;
};

#endif
