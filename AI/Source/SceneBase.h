#ifndef SCENE_BASE_H
#define SCENE_BASE_H

#include "Scene.h"
#include "Mtx44.h"
#include "Camera.h"
#include "Mesh.h"
#include "MatrixStack.h"
#include "Light.h"
#include "GameObject.h"
#include <vector>

class SceneBase : public Scene
{
	enum UNIFORM_TYPE
	{
		U_MVP = 0,
		U_MODELVIEW,
		U_MODELVIEW_INVERSE_TRANSPOSE,
		U_MATERIAL_AMBIENT,
		U_MATERIAL_DIFFUSE,
		U_MATERIAL_SPECULAR,
		U_MATERIAL_SHININESS,
		U_LIGHTENABLED,
		U_NUMLIGHTS,
		U_LIGHT0_TYPE,
		U_LIGHT0_POSITION,
		U_LIGHT0_COLOR,
		U_LIGHT0_POWER,
		U_LIGHT0_KC,
		U_LIGHT0_KL,
		U_LIGHT0_KQ,
		U_LIGHT0_SPOTDIRECTION,
		U_LIGHT0_COSCUTOFF,
		U_LIGHT0_COSINNER,
		U_LIGHT0_EXPONENT,
		U_COLOR_TEXTURE_ENABLED,
		U_COLOR_TEXTURE,
		U_TEXT_ENABLED,
		U_TEXT_COLOR,
		U_TOTAL,
	};
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_TEXT,
		GEO_BALL,
		GEO_CUBE,
		GEO_LINE,
		GEO_BG,
		GEO_TICTACTOE,
		GEO_CROSS,
		GEO_CIRCLE,
		GEO_REVERSI,
		GEO_BLACK,
		GEO_WHITE,
		GEO_TOOFULL,
		GEO_FULL,
		GEO_HUNGRY,
		GEO_DEAD,
		GEO_SHARK,
		GEO_FISHFOOD,
		GEO_CRAZY,
		GEO_HAPPY,
		GEO_CHESSBOARD,
		GEO_QUEEN,
		GEO_KNIGHT,
		GEO_WHITEQUAD,
		GEO_WALL,
		GEO_FLOOR,
		GEO_AGENT,
		GEO_WAYPOINT,
		GEO_WHITEHEX,
		GEO_REVERSIBOARD4x4,
		GEO_REVERSIBOARD,
		GEO_REVERSIWHITE,
		GEO_REVERSIBLACK,
		GEO_FLAPPYBG,
		GEO_SIDEBAR,
		GEO_CHARACTER,
		GEO_PIPE,

		//Assignment 1
		GEO_SPEEDY_ANT_WORKER,
		GEO_SPEEDY_ANT_SOLDIER,
		GEO_SPEEDY_ANT_QUEEN,
		GEO_STRONG_ANT_WORKER,
		GEO_STRONG_ANT_SOLDIER,
		GEO_STRONG_ANT_QUEEN,
		GEO_FOOD,
		GEO_NEST,
		GEO_GRASS,
		GEO_TERRITORYRED,
		GEO_TERRITORYBLUE,
		GEO_SANDBOXBG,

		NUM_GEOMETRY,
	};
public:
	SceneBase();
	~SceneBase();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderText(Mesh* mesh, std::string text, Color color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y);
	void RenderMesh(Mesh *mesh, bool enableLight);
protected:
	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];
	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	Camera camera;

	MS modelStack;
	MS viewStack;
	MS projectionStack;

	Light lights[1];

	bool bLightEnabled;

	float fps;
};

#endif