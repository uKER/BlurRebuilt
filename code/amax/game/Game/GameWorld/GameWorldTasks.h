#ifndef GAME_WORLD_TASKS_H
#define GAME_WORLD_TASKS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace WorldRender
{
	class C_LightList;
}

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace Game
{
	class C_View;
	class C_ViewManager;
	class C_GameWorld;

	struct C_ShadowCaster
	{
		Maths::HVec3		m_Position;
		F32					m_Intensity;
		//U32					m_Index;

		C_ShadowCaster() : m_Position(0.f, 0.f, 0.f), m_Intensity(0.f)
		{
		}
	};

	struct C_RimGenerator
	{
		Maths::HVec3		m_Position;
		Maths::HVec3		m_Direction;
		Maths::HVec3		m_Colour;
		Maths::HF32			m_Radius;
	};

	struct C_LightCubeMapInfo
	{
#ifdef PLATFORM_SPU
		void* m_pTextureCube;
#else
		Display::C_TextureCube* m_pTextureCube;
#endif
		Maths::HVec3 m_Position;
		void*	m_MemoryLocked[6];
		U32		m_Pitch;
		bool m_FloatRGBM; // [Blur2PC][HDR] Eight-byte RGBA16F texels, same RGBM decoding.
	};

#ifndef PLATFORM_SPU
	struct C_VehicleTaskInfo
	{
		Maths::HVec3	m_Position;
		S32				m_ViewIndex;
		U32				m_NeedUpdateDynamicSH;
		F32				m_DynamicSphericalHarmonicLight[16];
		Maths::HVec3	m_HeadLightPosition;
		F32				m_HeadLightRadius;
		F32				m_HeadLightFadeIntensity;
		U32				m_LightFlags;
	};

	struct C_DynamicPointLight
	{
		Maths::HVec3 m_Position;
		Maths::HVec3 m_Colour;
		F32			 m_Radius;
		U32			 m_Flags;
	};
	struct C_DynamicSpotLight
	{
		Maths::HVec3 m_Position;
		Maths::HVec3 m_Direction;
		Maths::HVec3 m_Colour;
		F32			 m_Radius;
		Maths::Mat44 m_WorldLight;
		U16			 m_RenderFlags;
		F32			 m_HorizontalFoV;
		F32			 m_VerticalFoV;
		U32			 m_Flags;
	};

	struct C_CarDynamicEffectsTaskInit
	{
		Game::C_GameWorld*					m_pGameWorld;
		Game::C_ViewManager*				m_pViewManager;
		U32									m_NumViews;
		Game::C_View*						m_pView[4];
		SceneGraph::C_SceneGraph*			m_pSceneGraph;
		const WorldRender::C_LightList*		m_pLightList;
		C_DynamicPointLight*				m_DynamicPointLightVector[4];
		U32									m_DynamicPointLightCount[4];
		C_DynamicSpotLight*					m_DynamicSpotLightVector[4];
		U32									m_DynamicSpotLightCount[4];
		U32									m_NumVehicles;
		C_VehicleTaskInfo					m_VehicleInfo[20];
		Maths::HVec4						m_FakeLightColour;
		Maths::HVec3						m_FakeLightPosition;
		C_CarDynamicEffectsTaskInit()
		{
			for (U32 i = 0; i < 20; i++)
				m_VehicleInfo[i].m_ViewIndex = 0;
		}
	};
#endif

	struct ALIGN16 C_CarUpdateLightCubeMapTaskInit
	{
		C_CarUpdateLightCubeMapTaskInit()
		{
			m_NumLightCubeMaps = 0;
		}

		Game::C_RimGenerator*				m_RimGeneratorVector;
		C_LightCubeMapInfo					m_LightCubeMapInfo[20];
		U32									m_NumLightCubeMaps;
	};

	const U32 GameWorld_MaxShadowCasters = 15;
	const U32 GameWorld_MaxRimGenerators = 15;
	const U32 GameWorld_LightCubeMapSize = 8;
}

#endif // GAME_WORLD_H
