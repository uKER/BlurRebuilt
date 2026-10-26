#include "stdafx.h"

#include <Game/GameWorld/View/CubeMap.h>
#include <Game/GameWorld/ViewManager.h>

#define SCENEGRAPH_ALLOW_INCLUDE
#include <SceneGraph/SceneGraphImpl.h>
#include <SceneGraph/View.h>
#include <SceneGraph/MultiView.h>
#include <SceneGraph/RenderNode.h>
#include <Dynamic/RenderNode.h>
#include <Dynamic/Renderer.h>

#include <Fx/Fx.h>

#include <Game/Game.h>
#include "Game/GameWorld/GameWorld.h"
#include <Game/GameWorld/RenderPass.h>
#include <Game/GameWorld/View.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/ViewType.h>
#include <GameLogic/GameData/GameData.h>
#include <Vehicle/VehicleModel.h>

#include <AuxFlares/AuxFlares.h>

#include <crowd_system/Effects/CrowdFlags.h>

#include <Graphics/GpuMarkers.h>

extern Debug::Bool			g_bRenderEverythingInCubemap;
Debug::Bool					g_bRenderFakeEnvironmentLight("Amax.FakeEnvironmentLight.Enable", true);
Debug::Bool					g_bDebugRenderFakeEnvironmentLight("Amax.FakeEnvironmentLight.RenderDebug", false);
Debug::Float				g_SunFakeEnvironmentOccusionFactor("Amax.FakeEnvironmentLight.SunOccusionFactor", 2.f, Debug::Range(0.f, 5.f));

namespace Game
{

extern GameWorldFxParams	g_FxParams;

void C_CubeMapView::Create()
{
	// create a multiview for the cubemap faces
	m_pMultiView = new SceneGraph::C_MultiView(m_pViewManager->GetSceneGraph());
	//Cubemap views
	for (U32 i = 0; i < 6; i++)
	{
		m_pView[i] = m_pViewManager->GetSceneGraph()->AddView();
		m_pView[i]->SetSubtechniqueHandle(m_pViewManager->GetLODSubtechniqueHandle() | m_pViewManager->GetEnvironmentSubtechniqueHandle());

		if (g_bRenderEverythingInCubemap)
		{
			m_pView[i]->EnableRenderPass(		SceneGraph::Flag<E_RenderPass_CharacterSolid>() |
												SceneGraph::Flag<E_RenderPass_CharacterPunch>() |
												SceneGraph::Flag<E_RenderPass_Solid>() |
												SceneGraph::Flag<E_RenderPass_DynamicObjectSolid>() |
												SceneGraph::Flag<E_RenderPass_AnimatedObjectSolid>() |
												SceneGraph::Flag<E_RenderPass_AnimatedObjectPunch>() |
												SceneGraph::Flag<E_RenderPass_Punch>() |
												SceneGraph::Flag<E_RenderPass_Sky>() |
												SceneGraph::Flag<E_RenderPass_Decal>() |
												SceneGraph::Flag<E_RenderPass_Racification>() |
												SceneGraph::Flag<E_RenderPass_Alpha>()
												);
		}
		else
		{
			m_pView[i]->EnableRenderPass(SceneGraph::Flag<E_RenderPass_Solid>() |
										SceneGraph::Flag<E_RenderPass_Sky>() |
										SceneGraph::Flag<E_RenderPass_Racification>() |
										SceneGraph::Flag<E_RenderPass_Alpha>()
										);
		}

		// disable everything other than primitives
		m_pView[i]->DisableRenderNode(SceneGraph::Flag<SceneGraph::E_RenderNodeType_StaticInstance>());
		m_pView[i]->DisableRenderNode(SceneGraph::Flag<SceneGraph::E_RenderNodeType_DynamicLight>());
		m_pView[i]->DisableRenderNode(m_pViewManager->m_pDynamicEffectRenderer->GetNodeFlag());
		m_pView[i]->DisableRenderNode(m_pViewManager->m_pAuxFlares->GetNodeFlag());
		if (m_pViewManager->m_pCrowdFlagRenderer)
			m_pView[i]->DisableRenderNode(m_pViewManager->m_pCrowdFlagRenderer->GetNodeFlag());
		
		m_pView[i]->SetType(E_ViewType_CubeMap);
		m_pView[i]->DisableCullMethod(SceneGraph::E_CullMethod_Portal);
		m_pView[i]->Initialise();
		m_pView[i]->ReserveGroups(1);
		m_pView[i]->GetGroup(0)->SetName("CubeMapFace");
		
		// add cubemap face to the multiview
		m_pMultiView->AddView(m_pView[i]);
	}

	

		//Create render views
#if defined(PLATFORM_PS3)
	Display::E_Multisample cubemapMultisample = Display::E_Multisample_None;
#else
	Display::E_Multisample cubemapMultisample = Display::E_Multisample_None;
#endif

	U32 environmentResolution = 256;
	m_FarClip = 100.f;
	if (AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mCity])
	{
		switch (pCity->environmentResolution)
		{
		case EnvironmentResolution_High_256:
			environmentResolution = 256;
			break;
		case EnvironmentResolution_Medium_128:
			environmentResolution = 128;
			break;
		case EnvironmentResolution_Low_64:
			environmentResolution = 64;
			break;
		}

		m_FarClip = Maths::Clamp(pCity->environmentFarClip, 0.f, 500.f);
		if (m_FarClip == 0.f)
			m_FarClip = 100.f;
	}

	m_RenderGroupSettings.SetNumWorkers(1);
	m_RenderGroupSettings.SetMultisample(cubemapMultisample);

	SetFormat(environmentResolution, cubemapMultisample);
	SetFarClip(m_FarClip);

	GameView::C_CubeMapView::Create();
}

void C_CubeMapView::Destroy()
{
	for (U32 i = 0; i < 6; i++)
	{
		m_pViewManager->GetSceneGraph()->RemoveView(m_pView[i]);
	}
	delete m_pMultiView;
}

#ifdef PLATFORM_PC
Debug::Bool g_CubeMapEnable("Amax.Views.CubeMapEnable", false );
#else
Debug::Bool g_CubeMapEnable("Amax.Views.CubeMapEnable", true);
#endif
Debug::Float g_CubeMapInstanceDrawDistanceScale("Amax.Views.CubeMapInstanceDrawDistanceScale", 0.25f, Debug::Range(0.0f, 1.0f));
Debug::Float g_CubeMapInstanceDrawDistanceScaleLOD("Amax.Views.CubeMapInstanceDrawDistanceScaleLOD", 0.10f, Debug::Range(0.0f, 1.0f));
Debug::Float g_CubeMapDrawDistanceFactorLOD("Amax.Views.CubeMapDrawDistanceFactorLOD", 0.25f, Debug::Range(0.0f, 1.0f));
Debug::Bool g_CaptureCubeMap("Game.CaptureOneFrameCubeMap", false );
Debug::Float g_CaptureCubeMapFarDistance("Game.CaptureCubeMapFarDistance", 5000.f);
Debug::String g_CubeMapFileName("Game.CubeMapFileName", "GAME:\\cubemap" );

void C_CubeMapView::Gather()
{
#if PLATFORM_PC
	if(g_CubeMapEnable && m_Enabled && m_GraphicalDetailEnabled)	// [Blur2PC] retail 0x00425c30/0x00425d50/0x00426e50
#else
	if(g_CubeMapEnable && m_Enabled)
#endif
	{
		if (g_CaptureCubeMap)
		{
			SetFarClip(g_CaptureCubeMapFarDistance);
		}
		PERF_EVENT("Render.GatherCubeMap");
		GameView::C_CubeMapView::Gather();
		m_pMultiView->BeginGather();
	}
}

void C_CubeMapView::GatherFace(const U32 face)
{
	m_pView[face]->SetViewFrustum(m_View[face].GetViewFrustum());
	m_pView[face]->SetViewData(m_View[face].GetViewSettings3D());
	m_pView[face]->SetDrawDistanceScale(Maths::Lerp(g_CubeMapInstanceDrawDistanceScaleLOD, g_CubeMapInstanceDrawDistanceScale, m_DetailFactor));
	if (!g_CaptureCubeMap)
	{
		m_pView[face]->SetFarClip(Maths::Lerp(m_FarClip * g_CubeMapDrawDistanceFactorLOD, m_FarClip, m_DetailFactor));
	}
	else
	{
		m_pView[face]->SetFarClip(g_CaptureCubeMapFarDistance);
	}
}

void C_CubeMapView::Schedule()
{
#if PLATFORM_PC
	if(g_CubeMapEnable && m_Enabled && m_GraphicalDetailEnabled)	// [Blur2PC] retail 0x00425c30/0x00425d50/0x00426e50
#else
	if(g_CubeMapEnable && m_Enabled)
#endif
	{
		PERF_EVENT("Render.ScheduleCubeMap");
		{

			PERF_EVENT("Render.SetFxParams");
			
			const C_LightFlareSettings& levelSettings = m_pViewManager->m_pGameWorld->GetLevelLightFlareSettings();
			const C_LightFlareSettings& carSettings = m_pViewManager->m_pGameWorld->GetCarLightFlareSettings();

			FX::SetParameter(g_FxParams.globalLightFlareSize, Display::C_PSConstant(levelSettings.m_GlobalSize, levelSettings.m_EnvironmentSizeX, levelSettings.m_EnvironmentSizeY, 0.0f));
			FX::SetParameter(g_FxParams.globalLightFlareColour, Display::C_PSConstant(levelSettings.m_EnvironmentColour.R(), levelSettings.m_EnvironmentColour.G(), levelSettings.m_EnvironmentColour.B(), levelSettings.m_EnvironmentColour.A()));
			FX::SetParameter(g_FxParams.carGlobalLightFlareSize, Display::C_PSConstant(carSettings.m_GlobalSize, carSettings.m_EnvironmentSizeX, carSettings.m_EnvironmentSizeY, 0.0f));
			FX::SetParameter(g_FxParams.carGlobalLightFlareColour, Display::C_PSConstant(carSettings.m_ReflectionColour.R(), carSettings.m_ReflectionColour.G(), carSettings.m_ReflectionColour.B(), carSettings.m_ReflectionColour.A()));
			FX::SetParameter(g_FxParams.nearFarLightFlare, Display::C_PSConstant(levelSettings.m_EnvironmentNearFadeDistant, levelSettings.m_EnvironmentFarFadeDistant, levelSettings.m_SizeDistantFactor / 100.f, levelSettings.m_IntensityDistantFactor / 100.f));
		}

		m_pMultiView->EndGather();

		GameView::C_CubeMapView::Schedule();
	}
}

void C_CubeMapView::ScheduleFace(const U32 face)
{
	m_pView[face]->BeginSchedule(0, m_View[face], &m_RenderGroupSettings);

	if (g_bRenderEverythingInCubemap)
	{
	m_pView[face]->SchedulePass(E_RenderPass_CharacterSolid);
	m_pView[face]->SchedulePass(E_RenderPass_CharacterPunch);
	m_pView[face]->SchedulePass(E_RenderPass_Solid);
	m_pView[face]->SchedulePass(E_RenderPass_AnimatedObjectSolid);
	m_pView[face]->SchedulePass(E_RenderPass_AnimatedObjectPunch);
	m_pView[face]->SchedulePass(E_RenderPass_DynamicObjectSolid);
	m_pView[face]->SchedulePass(E_RenderPass_Punch);
	m_pView[face]->SchedulePass(E_RenderPass_Sky);
	m_pView[face]->SchedulePass(E_RenderPass_Decal);
	m_pView[face]->SchedulePass(E_RenderPass_Racification);
	m_pView[face]->SchedulePass(E_RenderPass_Alpha);
	}
	else
	{
	m_pView[face]->SchedulePass(E_RenderPass_Solid);
	m_pView[face]->SchedulePass(E_RenderPass_Sky);
	m_pView[face]->SchedulePass(E_RenderPass_Racification);
	m_pView[face]->SchedulePass(E_RenderPass_Alpha);
	}
	m_pView[face]->EndSchedule();
}

void C_CubeMapView::Render()
{
#if PLATFORM_PC
	if(g_CubeMapEnable && m_Enabled && m_GraphicalDetailEnabled)	// [Blur2PC] retail 0x00425c30/0x00425d50/0x00426e50
#else
	if (g_CubeMapEnable && m_Enabled)
#endif
	{
		CPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "CPU.RenderCubeMap");
		GPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "GPU.RenderCubeMap");

		if (g_CaptureCubeMap)
		{
			GameView::C_CubeMapView::Render(true, (const char*)g_CubeMapFileName);
			g_CaptureCubeMap = false;
		}
		else
		{
			GameView::C_CubeMapView::Render(false, Util::C_String());
		}
	}
}

void C_CubeMapView::RenderFace(const U32 face)
{
	m_pView[face]->BeginRender();
	m_pView[face]->RenderGroup();
	m_pView[face]->EndRender();

	if (g_bRenderFakeEnvironmentLight)
	{
		Display::Device()->InvalidateState();
		Display::Device()->DefaultStates();

		RenderFakeLighting();
	}
}

void C_CubeMapView::UpdateView(C_View& gameView)
{
	for(S32 i = 0; i < 6; ++i)
		m_pView[i]->SetPvsCell(gameView.GetMainView().GetPvsCell());
	m_pMultiView->SetPvsCell(gameView.GetMainView().GetPvsCell());
}

void C_CubeMapView::SetDynamicDetail(F32 detail)
{
	// if detail is less than 0.66 then make cubemap update at half rate
	if(detail < 0.66f)
	{
		m_Enabled = !m_Enabled;
	}
	else
		m_Enabled = true;

	m_DetailFactor = detail;
}

void C_CubeMapView::RenderFakeLighting()
{
	U32 vertexCount = 0;

	for (U32 i = 0; i < 3; i++)
	{
		if (m_FakeLightingVector[i].m_Enabled)
			vertexCount += 4;
	}
	if (vertexCount > 0)
	{

		//Setup states
		Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);
		Display::Device()->SetCullMode(Display::E_CullMode_None);
		Display::Device()->SetDepthEnable(Display::E_Bool_False);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);	

		Display::Device()->SetVertexShader(&m_pViewManager->GetFakeLightingVertexShader());
		Display::Device()->SetPixelShader(&m_pViewManager->GetFakeLightingPixelShader());

		// knock out until we have a begin vertices method for PC
	#ifndef PLATFORM_PC

	//	m_FakeLightingVector[0].m_Position = Maths::HVec3(1.f, 1.f, 1.f) * m_pViewManager->GetView(0)->GetMainView().GetViewSettings3D().GetCameraToWorld();
	//	m_FakeLightingVector[0].m_Size	= 15.f;
	//	m_FakeLightingVector[0].m_Colour	= Maths::C_RGBA_F32(0.4f, 0.4f, 1.2f, 1.f);
	//	m_FakeLightingVector[0].m_Enabled = true;

		Display::Vertex_XYZ_HDR_UV* pVertices = (Display::Vertex_XYZ_HDR_UV*)Display::Device()->BeginVertices(Display::E_PrimitiveType_QuadList, vertexCount, sizeof(Display::Vertex_XYZ_HDR_UV), Display::Vertex_XYZ_HDR_UV::m_pDeclaration);

		if(pVertices)
		{
			U32 count = 0;
			for (U32 i = 0; i < 3; i++)
			{
				if (m_FakeLightingVector[i].m_Enabled)
				{
					Maths::HVec3 position;
					Maths::HVec4 colour(m_FakeLightingVector[i].m_Colour.R(), m_FakeLightingVector[i].m_Colour.G(), m_FakeLightingVector[i].m_Colour.B(), 0);
					
					if (i == 0 || i == 1)
					{
						//Camera related
						position = m_FakeLightingVector[i].m_Position * m_pViewManager->GetView(0)->GetMainView().GetViewSettings3D().GetCameraToWorld();
					}
					else
					{
						//World position, calculate direction vector
						Maths::HVec3 dirVector = m_FakeLightingVector[i].m_Position - m_Position;	
						dirVector.Normalise();
						position = m_Position + dirVector;

						//Use direction to attenuate the light base of SH light
						Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
						if (pRLC && (pRLC->GetNumVehicles() > 0))
						{
							S8 focusVehicleIndex = pRLC->GetViewportFocusVehicleIndex(0);
							if (focusVehicleIndex >= 0)
							{
								Game::Actors::C_VehicleActor* pVehicleActor = pRLC->GetVehicleActor(focusVehicleIndex);
								const WorldRender::C_LightSampleData& lightSampleData = pVehicleActor->GetVehicleModel()->GetLightSampler()->GetLightSampleData();

								//Calculate intensity from direction
								Maths::HVec4 redLightSH(lightSampleData.m_SampleData[0], lightSampleData.m_SampleData[1], lightSampleData.m_SampleData[2], lightSampleData.m_SampleData[3]);
								Maths::HVec4 greenLightSH(lightSampleData.m_SampleData[4], lightSampleData.m_SampleData[5], lightSampleData.m_SampleData[6], lightSampleData.m_SampleData[7]);
								Maths::HVec4 blueLightSH(lightSampleData.m_SampleData[8], lightSampleData.m_SampleData[9], lightSampleData.m_SampleData[10], lightSampleData.m_SampleData[11]);
								Maths::HVec4 normal = dirVector || Maths::HVecW(1.f);
								F32 light = (normal | redLightSH) + (normal | greenLightSH) + (normal | blueLightSH);
								light = Maths::Min(light * g_SunFakeEnvironmentOccusionFactor, 1.f);
								colour = colour * light;
							}
							
						}

					}
					
					if (g_bDebugRenderFakeEnvironmentLight)
					{
						Maths::HVec3 dirVector = position - m_Position;
						dirVector.Normalise();
						Render::Debug::Deferred()->SetDepthEnable(FALSE);
						Render::Debug::Deferred()->Draw3dArrow(m_Position, dirVector, 3.f, 1.f, Maths::RGBA(m_FakeLightingVector[i].m_Colour.R(), m_FakeLightingVector[i].m_Colour.G(), m_FakeLightingVector[i].m_Colour.B() , 1.f));
						Render::Debug::Deferred()->SetDepthEnable(TRUE);
					}

					colour.W(m_FakeLightingVector[i].m_Size);

					Maths::Store(position, (MathsStorage::Float3*)&pVertices[count * 4 + 0].x);
					Maths::Store(colour, (MathsStorage::Float4*)&pVertices[count * 4 + 0].r);
					pVertices[count * 4 + 0].u = -1.f;
					pVertices[count * 4 + 0].v = 1.f;

					Maths::Store(position, (MathsStorage::Float3*)&pVertices[count * 4 + 1].x);
					Maths::Store(colour, (MathsStorage::Float4*)&pVertices[count * 4 + 1].r);
					pVertices[count * 4 + 1].u = 1.f;
					pVertices[count * 4 + 1].v = 1.f;

					Maths::Store(position, (MathsStorage::Float3*)&pVertices[count * 4 + 2].x);
					Maths::Store(colour, (MathsStorage::Float4*)&pVertices[count * 4 + 2].r);
					pVertices[count * 4 + 2].u = 1.f;
					pVertices[count * 4 + 2].v = -1.f;

					Maths::Store(position, (MathsStorage::Float3*)&pVertices[count * 4 + 3].x);
					Maths::Store(colour, (MathsStorage::Float4*)&pVertices[count * 4 + 3].r);
					pVertices[count * 4 + 3].u = -1.f;
					pVertices[count * 4 + 3].v = -1.f;

					count++;
				}
			}
			Display::Device()->EndVertices();
		}

	#endif

		Display::Device()->SetVertexShader(NULL);
		Display::Device()->SetPixelShader(NULL);

		Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
		Display::Device()->SetCullMode(Display::E_CullMode_Default);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	}
}

void C_CubeMapView::UpdateFakeLight( const U32& index, const Maths::C_RGBA_F32& colour, const Maths::HVec3& position,const F32& size )
{
	BIZ_ASSERT(index < 3);
	if (size > 0.01f)
	{
		m_FakeLightingVector[index].m_Enabled = true;
		m_FakeLightingVector[index].m_Colour = colour;
		m_FakeLightingVector[index].m_Position = position;
		m_FakeLightingVector[index].m_Size = size;
	}
	else
	{
		m_FakeLightingVector[index].m_Enabled = false;
	}

}

void C_CubeMapView::GetFakeLight( const U32& index, bool& enable, Maths::C_RGBA_F32& colour, Maths::HVec3& position, F32& size )
{
	BIZ_ASSERT(index < 3);
	enable = m_FakeLightingVector[index].m_Enabled;
	colour = m_FakeLightingVector[index].m_Colour;
	position = m_FakeLightingVector[index].m_Position;
	size = m_FakeLightingVector[index].m_Size;
}
} // namespace Game
