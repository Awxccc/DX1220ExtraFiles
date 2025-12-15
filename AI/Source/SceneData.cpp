#include "SceneData.h"

int SceneData::GetObjectCount() const
{
	return m_objectCount;
}

void SceneData::SetObjectCount(const int objectCount)
{
	m_objectCount = objectCount;
}

int SceneData::GetFishCount() const
{
	return m_fishCount;
}

void SceneData::SetFishCount(const int fishCount)
{
	m_fishCount = fishCount;
}

SceneData::SceneData()
	: m_objectCount(0)
	, m_fishCount(0)
	, m_noGrid(0)
	, m_gridSize(0.f)
	, m_gridOffset(0.f)
{
}

SceneData::~SceneData()
{
}

int SceneData::GetNumGrid() const
{
	return m_noGrid;
}

void SceneData::SetNumGrid(const int numGrid)
{
	m_noGrid = numGrid;
}

float SceneData::GetGridSize() const
{
	return m_gridSize;
}

void SceneData::SetGridSize(const float gridSize)
{
	m_gridSize = gridSize;
}

float SceneData::GetGridOffset() const
{
	return m_gridOffset;
}

void SceneData::SetGridOffset(const float gridOffset)
{
	m_gridOffset = gridOffset;
}
