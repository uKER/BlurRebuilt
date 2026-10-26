#ifndef GAME_WORLD_VIEW_TYPE_H
#define GAME_WORLD_VIEW_TYPE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

enum E_ViewType
{
	E_ViewType_CubeMap,
	E_ViewType_Mirror,
	E_ViewType_Prepass,
	E_ViewType_Reflection,
	E_ViewType_Main1,
	E_ViewType_Main2,
	E_ViewType_Main3,
	E_ViewType_Main4,
	E_ViewType_LightTrails,
	E_ViewType_Count
};

enum E_ViewTypeFlag
{
	E_ViewTypeFlag_CubeMap		= 1 << E_ViewType_CubeMap,
	E_ViewTypeFlag_Mirror		= 1 << E_ViewType_Mirror,
	E_ViewTypeFlag_Prepass		= 1 << E_ViewType_Prepass,
	E_ViewTypeFlag_Reflection	= 1 << E_ViewType_Reflection,
	E_ViewTypeFlag_Main1		= 1 << E_ViewType_Main1,
	E_ViewTypeFlag_Main2		= 1 << E_ViewType_Main2,
	E_ViewTypeFlag_Main3		= 1 << E_ViewType_Main3,
	E_ViewTypeFlag_Main4		= 1 << E_ViewType_Main4,
	E_ViewTypeFlag_Main			= E_ViewTypeFlag_Main1 | E_ViewTypeFlag_Main2 | E_ViewTypeFlag_Main3 | E_ViewTypeFlag_Main4,
	E_ViewTypeFlag_LightTrails	= 1 << E_ViewType_LightTrails,
	E_ViewTypeFlag_All			= E_ViewTypeFlag_CubeMap | E_ViewTypeFlag_Reflection | E_ViewTypeFlag_Mirror | E_ViewTypeFlag_Main | E_ViewTypeFlag_LightTrails
};

const U32 g_MainViewFlags[4] = { E_ViewTypeFlag_Main1, E_ViewTypeFlag_Main2, E_ViewTypeFlag_Main3, E_ViewTypeFlag_Main4 };

} // namespace Game

#endif // GAME_WORLD_VIEW_TYPE_H
