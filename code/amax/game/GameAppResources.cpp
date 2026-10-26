#include "StdAfx.h"

#include <system/time/scopedtimer.h>

#include "GameApp.h"
#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>
#include <game_resource/ModelResource.h>
#include <TextureResource/TextureResource.h>

#include <game_resource/EffectVariations.h>
#include <algorithm>
#include <fx/Fx.h>
#include <fx/RenderMutex.h>

#include <dynamic/renderer.h>
#include <dynamic/generic/XtInterface.h>
#include <ui/Game/UILocaliser.h>

#include <SpecialFX/SpecialFX.h>
#include <SplashCamera/SplashCamera.h>
#include <Game/GameUtil/LoadingPerf.h>
#include <Game/GameUtil/ResourceBag.h>
#include <GameLogic/GameData/GameXtLoader.h>

#include <Dynamic/Particle/UserParticleLibrary.h>

namespace Framework
{
	extern Debug::Logger& g_BootLog;
}

//#define USE_RESOURCE_THREADS

//#define PROFILE_FXPRELOAD

#ifdef PROFILE_FXPRELOAD
#include "tracerecording.h"
#pragma comment( lib, "tracerecording.lib" )
#pragma comment( lib, "xbdm.lib" )
#endif

static bool StringSort(const char* a, const char* b) { return stricmp(a,b)<0; };

typedef void(*VoidFunc)();

MT::Thread* g_pBootCacheThread = NULL;
MT::Thread* g_pPostBootCacheAnimationsThread = NULL;

class C_RunFuncAsync : public MT::Thread
{
public:
	C_RunFuncAsync(VoidFunc pFunc) : MT::Thread(), m_pFunc(pFunc)
	{
		BIZ_ASSERT_SPEED(m_pFunc);
	}

	virtual ~C_RunFuncAsync()
	{
	}

	virtual void ThreadMain()
	{
		m_pFunc();
	}

private:
	VoidFunc m_pFunc;
};


void CacheBootGlobalResourcesFunc()
{
	SCOPED_TIMER("C_GameApp::CacheBootGlobalResourcesFunc()");

	DynamicEffect::Textures*	pParticleTextures;
	Btl::Vector<FX::I_Effect*>* pEffectPointers;
	C_GameApp::GetInstance()->LockCacheBootGlobalResources(&pParticleTextures, &pEffectPointers);

	const char *g_EffectsPreloadList[] = 
	{
		"Advert_LN_NV.fx",
		"AdvertTwoLayer_LN_NV.fx",
		"AnimatedNeon_LN.fx",
		"AnimatedNeon_LN_NL.fx",
		"AnimatedNeon_LN_NL_NV.fx",
		"AnimatedNeon_LN_NV.fx",
		"BillBoard_LN.fx",
		"CarLightFlare_LN.fx",
		"DecalFade_LN.fx",
		"default_LN.fx",
		"default_LN_NB_NL.fx",
		"default_LN_NB_NL_NM.fx",
		"default_LN_NB_NL_NM_NV.fx",
		"default_LN_NB_NL_NV.fx",
		"default_LN_NB_NM_NV.fx",
		"default_LN_NB_NV.fx",
		"default_LN_NL.fx",
		"default_LN_NL_NM.fx",
		"default_LN_NL_NM_NV.fx",
		"default_LN_NL_NV.fx",
		"default_LN_NM_NV.fx",
		"default_LN_NV.fx",
		"default_skin_LN_NB_NL_NM_NV.fx",
		"default_skin_LN_NB_NL_NV.fx",
		"default_skin_LN_NV.fx",
		"DetailColour_LN.fx",
		"DetailColourTwoMtl_LN.fx",
		"DetailColourTwoMtl_LN_NL.fx",
		"FlashingLightFlare_LN.fx",
		"FlashingNeon_LN_NV.fx",
		"FlickeringNeon_LN_NV.fx",
		"Foliage_LN_NL.fx",
		"Foliage_LN_NV.fx",
		"FoliageBillboard_LN.fx",
		"HighVis_LN_NB_NL.fx",
		"HighVis_LN_NB_NL_NV.fx",
		"HighVis_LN_NB_NV.fx",
		"HighVis_skin_LN_NB_NL_NV.fx",
		"HighVisBarrier_LN_NL_NV.fx",
		"HighVisBarrier_LN_NV.fx",
		"LightFlare_LN.fx",
		"Material_LN_NB_NM_NV.fx",
		"Material_LN_NL_NV.fx",
		"Material_LN_NV.fx",
		"MeshEffect_LN.fx",
		"MeshEffectCull_LN.fx",
		"MeshEffectCullNoDepth.fx",
		"MeshEffectCullNoDepth_LN.fx",
		"MeshEffectDLayer.fx",
		"MeshEffectDLayer_LN.fx",
		"MeshEffectDLayerCull_LN.fx",
		"MeshEffectDLayerCullNDepth_LN.fx",
		"MeshEffectDLayerUVDist.fx",
		"MeshEffectDLayerUVDist_LN.fx",
		"MeshEffectDLayerUVDistCull.fx",
		"MeshEffectDLayerUVDistCull_LN.fx",
		"MeshEffectDLayerUVDistCullNDepth_LN.fx",
		"MeshEffectDLayerUVDistNDepth_LN.fx",
		"MeshEffectNoDepth_LN.fx",
		"MeshEffectRefract_LN.fx",
		"MeshEffectUVDistort.fx",
		"MeshEffectUVDistort_LN.fx",
		"MeshEffectUVDistortCull_LN.fx",
		"MeshEffectUVDistortCullNoDepth_LN.fx",
		"MeshEffectUVDistortNoDepth.fx",
		"MeshEffectUVDistortNoDepth_LN.fx",
		"MeshEffect_skin_LN.fx",
		"MeshEffectCull_skin_LN.fx",
		"MeshEffectCullNoDepth_skin.fx",
		"MeshEffectCullNoDepth_skin_LN.fx",
		"MeshEffectDLayer_skin_LN.fx",
		"MeshEffectDLayerCullNDepth_skin_LN.fx",
		"MeshEffectDLayerUVDist_skin.fx",
		"MeshEffectDLayerUVDist_skin_LN.fx",
		"MeshEffectDLayerUVDistCull_skin.fx",
		"MeshEffectDLayerUVDistCull_skin_LN.fx",
		"MeshEffectDLayerUVDistNDepth_skin_LN.fx",
		"MeshEffectNoDepth_skin_LN.fx",
		"MeshEffectRefract_skin_LN.fx",
		"MeshEffectUVDistort_skin.fx",
		"MeshEffectUVDistort_skin_LN.fx",
		"MeshEffectUVDistortCull_skin_LN.fx",
		"MeshEffectUVDistortCullNoDepth_skin_LN.fx",
		"MeshEffectUVDistortNoDepth_skin.fx",
		"MeshEffectUVDistortNoDepth_skin_LN.fx",
		"Ocean_LN.fx",
		"Ocean_LN_NL.fx",
		"OceanFakeReflect_LN.fx",
		"OceanReflect_LN.fx",
		"Puddle_LN_NL_NV.fx",
		"Puddle_LN_NV.fx",		
		"Racification_LN.fx",
		"RacificationDLayer_LN.fx",
		"Road_LN.fx",
		"Road_LN_ND.fx",
		"Road_LN_ND_NG.fx",
		"Road_LN_ND_NG_NL.fx",
		"Road_LN_ND_NG_NL_NW.fx",
		"Road_LN_ND_NG_NS.fx",
		"Road_LN_ND_NG_NS_NW.fx",
		"Road_LN_ND_NG_NV.fx",
		"Road_LN_ND_NG_NV_NW.fx",
		"Road_LN_ND_NG_NW.fx",
		"Road_LN_NG.fx",
		"Road_LN_NG_NL.fx",
		"Road_LN_NG_NL_NS.fx",
		"Road_LN_NG_NL_NW.fx",
		"Road_LN_NG_NS.fx",
		"Road_LN_NG_NS_NV_NW.fx",
		"Road_LN_NG_NS_NW.fx",
		"Road_LN_NG_NW.fx",
		"Road_LN_NS_NW.fx",
		"Road_LN_NW.fx",
		"RoadDecal_LN.fx",
		"RoadDecal_LN_NS.fx",
		"RoadDecal_LN_NS_NV.fx",
		"RoadDecal_LN_NV.fx",
		"RoadN_LN_ND_NG.fx",
		"RoadN_LN_NG.fx",
		"RoadN_LN_NS.fx",
		"RoadN_LN_NG_NL.fx",
		"RoadN_LN_NG_NS.fx",
		"RoadTwoSpec_LN.fx",
		"RoadTwoSpec_LN_ND.fx",
		"RoadTwoSpec_LN_ND_NG.fx",
		"RoadTwoSpec_LN_ND_NG_NL.fx",
		"RoadTwoSpec_LN_ND_NG_NV.fx",
		"RoadTwoSpec_LN_ND_NG_NV_NW.fx",
		"RoadTwoSpec_LN_ND_NG_NW.fx",
		"RoadTwoSpec_LN_ND_NW.fx",
		"RoadTwoSpec_LN_NG.fx",
		"RoadTwoSpec_LN_NG_NL.fx",
		"RoadTwoSpec_LN_NG_NV.fx",
		"RoadTwoSpec_LN_NG_NV_NW.fx",
		"RoadTwoSpec_LN_NG_NW.fx",
		"RoadTwoSpec_LN_NW.fx",
		"RoadTwoSpecN_LN.fx",
		"RoadTwoSpecN_LN_ND_NG.fx",
		"RoadTwoSpecN_LN_NG.fx",
		"RoadTwoSpecN_LN_NG_NL.fx",
		"RoadTwoSpecN_LN_NG_NV.fx",		
		"ScreenFacingSprite_LN.fx",
		"ScrollFade_LN.fx",
		"ScrollFade_LN_NL.fx",
		"ScrollFade_skin_LN.fx",
		"ScrollFade_skin_LN_NL.fx",
		"ScrollUVNeon_LN_NV.fx",
		"SoftLightFlare_LN.fx",
		"SpeedTree_LN.fx",
		"SpeedTreeLeaf_LN.fx",
		"SyntheticBillboard_LN.fx",
		"veh_Alloy_a_b_f_LN.fx",
		"veh_Alloy_a_f_LN.fx",
		"veh_Alloy_b_f_LN.fx",
		"veh_Alloy_f_LN.fx",
		"veh_Alpha_c_h_LN.fx",
		"veh_Alpha_c_LN.fx",
		"veh_Alpha_LN.fx",
		"veh_Default.fx",
		"veh_Default_a_b.fx",
		"veh_Default_a_b_c.fx",
		"veh_Default_a_b_c_f.fx",
		"veh_Default_a_b_c_f_h.fx",
		"veh_default_a_b_c_f_h_LN.fx",
		"veh_Default_a_b_c_f_LN.fx",
		"veh_Default_a_b_c_h.fx",
		"veh_Default_a_b_c_h_LN.fx",
		"veh_Default_a_b_c_LN.fx",
		"veh_Default_a_b_f.fx",
		"veh_Default_a_b_f_h.fx",
		"veh_Default_a_b_f_h_LN.fx",
		"veh_Default_a_b_f_LN.fx",
		"veh_Default_a_b_h_LN.fx",
		"veh_Default_a_b_LN.fx",
		"veh_Default_a_c_f_h.fx",
		"veh_Default_a_c_f_h_LN.fx",
		"veh_Default_a_c_f_LN.fx",
		"veh_Default_a_c_h_LN.fx",
		"veh_Default_a_c_LN.fx",
		"veh_Default_b_c_f.fx",
		"veh_Default_b_c_f_h.fx",
		"veh_Default_b_c_f_h_LN.fx",
		"veh_Default_b_c_f_LN.fx",
		"veh_Default_b_c_h_LN.fx",
		"veh_Default_b_c_LN.fx",
		"veh_Default_b_f.fx",
		"veh_Default_b_f_h_LN.fx",
		"veh_Default_b_f_LN.fx",
		"veh_Default_b_LN.fx",
		"veh_Default_c_f_h.fx",
		"veh_Default_c_f_h_LN.fx",
		"veh_Default_c_f_LN.fx",
		"veh_Default_c_LN.fx",
		"veh_Default_f_LN.fx",
		"veh_Glass.fx",
		"veh_Glass_a_c.fx",
		"veh_Glass_a_c_d_h_LN.fx",
		"veh_Glass_a_c_d_LN.fx",
		"veh_Glass_a_c_h.fx",
		"veh_Glass_a_c_h_LN.fx",
		"veh_Glass_a_c_LN.fx",
		"veh_Glass_a_d_LN.fx",
		"veh_Glass_c_d_h_LN.fx",
		"veh_Glass_c_d_LN.fx",
		"veh_Glass_c_h.fx",
		"veh_Glass_c_h_LN.fx",
		"veh_Glass_c_LN.fx",
		"veh_Glass_d_LN.fx",
		"veh_Light.fx",
		"veh_Light_c.fx",
		"veh_Light_c_h.fx",
		"veh_Light_c_h_LN.fx",
		"veh_Light_c_LN.fx",
		"veh_Matt_a_b_c_f_h_LN.fx",
		"veh_Matt_a_b_c_f_LN.fx",
		"veh_Matt_a_b_c_h_LN.fx",
		"veh_Matt_a_b_c_LN.fx",
		"veh_Matt_a_b_f_h_LN.fx",
		"veh_Matt_a_b_f_LN.fx",
		"veh_Matt_a_b_LN.fx",
		"veh_Matt_b_c_f_h_LN.fx",
		"veh_Matt_b_c_f_LN.fx",
		"veh_Matt_b_f_h_LN.fx",
		"veh_Matt_b_f_LN.fx",
		"veh_Matt_b_LN.fx",
		"veh_Matt_c_f_h_LN.fx",
		"veh_Matt_c_f_LN.fx",
		"veh_Matt_f_LN.fx",
		"veh_Neon.fx",
		"veh_Neon_c_g_h.fx",
		"veh_Neon_c_g_h_LN.fx",
		"veh_Neon_c_g_LN.fx",
		"veh_Neon_c_h_LN.fx",
		"veh_Neon_c_LN.fx",
		"veh_Paint.fx",
		"veh_Paint_a_b.fx",
		"veh_Paint_a_b_c.fx",
		"veh_Paint_a_b_c_h.fx",
		"veh_Paint_a_b_c_h_LN.fx",
		"veh_Paint_a_b_c_LN.fx",
		"veh_Paint_a_b_h.fx",
		"veh_Paint_a_b_h_LN.fx",
		"veh_Paint_a_b_LN.fx",
		"veh_Paint_a_c_h.fx",
		"veh_Paint_a_c_h_LN.fx",
		"veh_Paint_a_c_LN.fx",
		"veh_Paint_a_LN.fx",
		"veh_Paint_b_c_h_LN.fx",
		"veh_Paint_b_c_LN.fx",
		"veh_Paint_b_h_LN.fx",
		"veh_Paint_b_LN.fx",
		"veh_Paint_c_LN.fx",
		"veh_Paint_LN.fx",
		"veh_Shadow.fx",
		"veh_Shadow_LN.fx",
		"Water_LN_NL_NV.fx",
		"Water_LN_NV.fx",
		"WaterCubeMap_LN.fx",
		"WaterCubeMap_LN_NL.fx",
		"WaterCubeMap_LN_NL_NV.fx",
		"WaterCubeMap_LN_NV.fx",
		"WaterFlow_LN.fx",
		"WaterFlow_LN_NL.fx",
		"WaterFlow_LN_NL_NV.fx",
		"WaterFlow_LN_NV.fx",
		"",
	};

	MEMORY_TRACKER_PUSH_SUB("shaders");

	FX::RegisterSubtechnique("LOD");
#if PLATFORM_PC
	FX::RegisterSubtechnique("NoDamage");	// [Blur2PC] retail order (Blur.exe 1.2 0x004340d7)
#endif
	FX::RegisterSubtechnique("Prepass");
	FX::RegisterSubtechnique("Lights");
	FX::RegisterSubtechnique("LSplitScreen");
	FX::RegisterSubtechnique("Electric");
	FX::RegisterSubtechnique("Solid");
	FX::RegisterSubtechnique("AlphaLuminance");
#if PLATFORM_PC
	FX::RegisterSubtechnique("NoDepth");	// [Blur2PC] retail, used by graphical detail 1
#endif

	GameResource::EffectVariations::Instance().SetVariation("_LN");

#ifdef PROFILE_FXPRELOAD
	XTraceStartRecording("e:\\FXpreload.pix2");
#endif

	// preload a list of effects into the effects system
	FX::LockSerialiseMutex();
	Btl::Vector<const char *> effects;
	Btl::Vector<Btl::String> effectNames;
	for (const char **ppsz = g_EffectsPreloadList; **ppsz; ppsz++)
		effectNames.PushBack(FX::GetShaderPath() + *ppsz + "b");

#ifdef PROFILE_FXPRELOAD
	effectNames.Resize(20); // TEMP TEST
#endif

	effects.Resize(effectNames.Size());
	for (int i=0; i<effectNames.Size(); i++)
		effects[i] = effectNames[i].Chars();
	std::sort(effects.Begin(), effects.End(), StringSort);
	FX::ReloadEffects();
	FX::LoadEffects(effects, *pEffectPointers);	
	FX::ReleaseSerialiseMutex();

#ifdef PROFILE_FXPRELOAD
	XTraceStopRecording();
#endif

	MEMORY_TRACKER_POP_SUB("shaders");

	MEMORY_TRACKER_PUSH_SUB("particles");
	
	const char *g_ParticleTexturesPreloadList[] = 
	{
		"bigflare",
		"damage_parts_02",
		"flakes_add_01",
		"flame",
		"flare",
		"flare_new_glow",
		"flare_new_spike",
		"flare_new_spikeball",
		"flare2",
		"flares_blue_01",
		"flares_blue_02",
		"flares_blue_03",
		"flares_green_01",
		"flares_green_02",
		"flares_green_03",
		"flares_pink_01",
		"flares_pink_03",
		"flares_purple_01",
		"flares_purple_03",
		"flares_red_01",
		"flares_red_02",
		"flares_red_03",
		"flares_yellow_01",
		"flares_yellow_03",
		"flares_white_01",
		"flares_white_03",
		"glassshards01",
		"glow",
		"gravel",
		"lantern01",
		"leaves01",
		"letters01",
		"lightflare",
		"lightflarestretched",
		"litter01",
		"litter02",
		"mud_splash",
		"mud_splash_big",
		"mud_splash_left",
		"mud_splash_right",
		"newspaper01",
		"newspaper02",
		"plate",
		"skid_mark",
		"skid_mark_sand_multiply",
		"smoke",
		"smokevolume",
		"smoke_new_01",
		"spark_pop_01",
		"splash01",
		"splash_left",
		"splash_right",
		"stone",
		"straw",
		"watersplash",
		"waterspray",
		"water_cloud",
		"wood01",
		""
	};

	Util::C_String particlesRoot("graphics\\textures\\particles\\");
	for (const char **ppsz = g_ParticleTexturesPreloadList; **ppsz; ppsz++)
	{
		Display::C_RefTexture* pRefTexture = new Display::C_RefTexture;
		Util::C_String filename(*ppsz);

		if(!Display::CreateTexture2DFromFile(particlesRoot + filename + ".texture", pRefTexture))
		{
			delete pRefTexture;
		}
		else
			pParticleTextures->Insert(DynamicEffect::Textures::DataType(filename, pRefTexture));
	}

	DynamicEffect::Particle::C_UserParticleLibrary::Instance().Open("graphics\\models\\particles\\Amax_Particle_Shapes.particleMesh");
	MEMORY_TRACKER_POP_SUB("particles");

	C_GameApp::GetInstance()->UnlockCacheBootGlobalResources();
}

void CachePostBootAnimationsFunc()
{
	MEMORY_TRACKER_PUSH("game.cachedresources");

	struct C_ResourceDef
	{
		char* m_Name;
		S32 m_Id;
	};

	SCOPED_TIMER("CachePostBootAnimationsFunc()");

	const C_ResourceDef globalResources[]=
	{
        { "animations\\cameras\\mp_winner.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\pack_am3.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\post_03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\post_04.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\post_05.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\post_06.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\post_07.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\pre_1_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\pre_2_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\pre_3_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\pre_4_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\pre_go_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\rival_front01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\rival_front02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\rival_front03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\rival_rear01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\rival_rear02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\vista_am3.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\cameras\\winner.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\crowdskeleton.skeleton", Animation::C_SkeletonResource::StaticGetResourceTypeId() },
        { "animations\\crowd\\hang\\crwd_hang01_postsit_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang01_postsit_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang02_poststand_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang02_poststand_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang03_fencetop_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang04_fencetop_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang05_swingunder_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang06_postlh_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang06_postlh_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang07_postrh_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\hang\\crwd_hang07_postrh_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean01_ledgef_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean01_ledgef_evade.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean01_ledgef_idle02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean02_ledgef_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean02_ledgef_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean03_ledgef_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean03_ledgef_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean04_ledgeb_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean04_ledgeb_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean05_wallb_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean05_wallb_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean06_carb_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean06_carb_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean07_fencef_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean07_fencef_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean08_postf_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean08_postf_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean09_fenceb_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\lean\\crwd_lean09_fenceb_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit01_chair_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit01_chair_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit02_chair_idle02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit03_hiledge_elated02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit03_hiledge_evade01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit03_hiledge_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit04_hiledge_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit04_hiledge_elated02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit04_hiledge_evade01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit04_hiledge_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit04_hiledge_idle02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit05_floor_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit05_floor_evade.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit05_floor_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit06_floor_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit06_floor_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit07_floor_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit07_floor_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit08_squat_elated_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit08_squat_evade_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit08_squat_idle_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\sit\\crwd_sit09_graffiti_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_deride01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_deride02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_elated02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_evade01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_evade02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_idle02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_strain01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_strain02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand01_skateboard_strain03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_cameraphone.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_deride.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_elated02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_evade01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_evade02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_idle02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand02_handships_straintosee.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_cameraphone.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_deride01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_deride02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_deride03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_elated02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_evade01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_evade02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_idle02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand03_standing_straintosee01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand04_balancing_idle02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand05_balancing_evade.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand05_balancing_idle02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand06_bmx_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand06_bmx_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand07_bmx_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand07_bmx_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand08_bmx_elated01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand08_bmx_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand09_graf_evade.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand09_graf_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\crowd\\stand\\crwd_stand_flagwave_idle.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_accelerate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_base.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_brake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_celebrate01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_celebrate02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_celebrate03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_frustration01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_frustration02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_frustration03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_frustration04.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_gearchange.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_glancebackl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_glancebackr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_glancel.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_glancer.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_handbrake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_impactb1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_impactf1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_impactl1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_impactl2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_impactr1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_impactr2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_leanl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_leanr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_lookl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_lookr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_noisehigh.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_noiselow.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive01_steering.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_accelerate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_base.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_brake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_celebrate01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_celebrate02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_celebrate03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_frustration01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_frustration02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_frustration03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_frustration04.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_frustration05.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_gearchange.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_glancebackl1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_glancebackl2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_glancebackr1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_glancebackr2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_glancel.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_glancer.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_handbrake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_impactb1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_impactf1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_impactf2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_impactl1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_impactl2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_impactr1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_impactr2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_leanl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_leanr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_lookl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_lookr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_noisehigh.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_noiselow.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\aggressive\\aggressive02_steering.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_accelerate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_base.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_brake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_celebrate01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_celebrate02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_celebrate03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_celebrate04.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_celebrate05.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_celebrate06.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_crashposerolledl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_crashposerolledn.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_crashposerolledr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_frustration01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_frustration02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_frustration03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_frustration04.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_frustration05.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_frustration06.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_frustration07.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_gearchange.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_glancebackl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_glancebackr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_glancel.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_glancer.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_handbrake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_impactb1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_impactf1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_impactl1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_impactl2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_impactr1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_impactr2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_leanl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_leanr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_lookl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_lookr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_noisehigh.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_noiselow.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\neutral\\neutral_steering.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_accelerate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_base.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_brake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_celebration01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_celebration02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_celebration03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_frustration_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_frustration_02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_frustration_03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_gearchange.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_glance_l.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_glance_r.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_glanceback_l.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_glanceback_r.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_handbrake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_impactb1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_impactf1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_impactl1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_impactr1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_leanl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_leanr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_lookl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_lookr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_neutral_trans.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_noisehigh.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_noiselow.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed01_steering.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_accelerate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_base.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_brake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_celebration01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_celebration02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_celebration03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_frustration_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_frustration_02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_frustration_03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_gearchange.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_glance_l.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_glance_r.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_glancebackl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_glancebackr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_handbrake.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_idle01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_impactb1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_impactf1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_impactl1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_impactr1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_leanl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_leanr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_lookl.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_lookr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_noisehigh.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_noiselow.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\drivers\\relaxed\\relaxed02_steering.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\igm_driver_ml.skeleton", Animation::C_SkeletonResource::StaticGetResourceTypeId() },
        { "animations\\perks\\bargebuild2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargebuild2_ai.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargebuildbumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\barged1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargeengulflevel2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargepickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargepickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargepickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargewave1left.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargewavebumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargewaveresidue.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bargewavering.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\batteringram.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\batteringramkhan.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blind_build_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blind_projectile_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blindfire_build_rev.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blindfire_engulf_car.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blindfire_hit_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blindfire_pow_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blindfireshotgunbuild.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blindfireshotgunbuildrev.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\blindfireshotgunpow.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\bolt_shotgun_projectile.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\boltlasersightbeam.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\destructionmodecolumn_blue.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\destructionmodecolumn_green.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\destructionmodecolumn_orange.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\destructionmodecolumn_red.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\draftwind1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\draftwindfadein1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\draftwindfadeout1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\drifterspiral.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\garage_engulf_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\garage_mystery_car.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\lastgaspshroud.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\lastgaspsphere.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mastermine01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mastermineballexpand.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mastermineblastresidue.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mastermineboomcircles.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\masterminecore1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mineballexpand.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mineblastimplode.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mineblastresidue.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mineboomcircles.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\minebuild1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\minebuild1reverse.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\minecore1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\minecore2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mined1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\minedtcore1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\mineengulflevel2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\minepickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\minepickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\minepickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitro_carver.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitroengulflevel2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitrolevel1nr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitrolevel2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitrolevel2nr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitrolevel3.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitrolevel3bumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitrolevel3nr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitropickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitropickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitropickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitroreverse01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitroreverseblastrings.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\nitroreversebumper01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\pickups\\fangoldenpickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\pickups\\fangoldenpickup_dimmed.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\pickups\\fanpickup_aggressive.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\pickups\\fanpickup_aggressive_dimmed.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\pickups\\fanpickup_speed.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\pickups\\fanpickup_speed_dimmed.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\repairengulf.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\repairing1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\repairingbumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\repairpickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\repairpickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\repairpickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\repairzone1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\rift2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\rifted.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_barge_norotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_barge_rotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_bolt_norotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_bolt_rotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_mine_norotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_mine_rotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_nitro_norotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_nitro_rotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_repair_norotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_repair_rotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_shield_norotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_shield_rotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_shock_norotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_shock_rotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_shunt_norotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\roadsweeper_shunt_rotate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\safetynet_barge.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\safetynet_blindfire.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\safetynet_mine.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\safetynet_shock.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\safetynet_shunt.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\scramblerbubbleshort.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\scramblerbumperbubbleshort.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldblockbarge1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldblockblindfire1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldblockmine1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldblockshock1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldblockshunt1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldedbaybubble.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldedbaybumperbubble.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldedboosters.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldengulf1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldkhan1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldkhan2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldlevel1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldlevel1bumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldpickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldpickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldpickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldsuper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldsuperbumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldsuperpickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldsuperpickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shieldsuperpickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shocked02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shockengulf2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shockfiredown1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shockfireup1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shockfireup1ai.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shocklevel2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shocklevel2player.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shockpickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shockpickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shockpickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shocksparks02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shocksparks03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shocktargetdown1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shunt_flow_car_02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntbuild2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntbuild2bumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntbuildai.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntbuildrev.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shunted1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntedball1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shunthit1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntlevel1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntpickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntpickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntpickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntpow1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntpow2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntprojectile1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\shuntring1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\silentrunning.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\silentrunningai.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\splashdamagebargebuild.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\splashdamagebargebuildbumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\splashdamagebargewave.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\splashdamagebargewavebumper.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\splashdamagebargewaveresidue.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\splashdamagebargewavering.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\stopwatchpickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\titaniumarmour.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\xppickup.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\xppickupdecoy.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\perks\\xppickupdecoyhit.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\arrow_projection_smoke.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\aug_1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\aug_2.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\aug_3.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\aug_go.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\birds01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\birds01.skeleton", Animation::C_SkeletonResource::StaticGetResourceTypeId() },
        { "animations\\specialfx\\blue_laser_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\brighton_tide1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\corner_arrows.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\direction_ambient_spot_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\dockslight.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\goldenfandemandgate.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\goldenfandemandgate.skeleton", Animation::C_SkeletonResource::StaticGetResourceTypeId() },
        { "animations\\specialfx\\goldenfandemandgate_air.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\goldenfandemandgate_air.skeleton", Animation::C_SkeletonResource::StaticGetResourceTypeId() },
        { "animations\\specialfx\\goldenfandemandgateend_air.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\green_laser_01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\helilight.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\lantern01.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\light_projection_beam.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\police_flashing_lights.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\saus_tide1.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\splitscreencolumn_bottomleft.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\splitscreencolumn_bottomright.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\splitscreencolumn_topleft.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\splitscreencolumn_topright.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\start_finish_line.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\start_line_ambient_spot_02.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\start_line_ambient_spot_03.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		//Load for all locs as tiny( 1 or 2 kb )
		{ "animations\\specialfx\\start_text_checkpoint_de.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\start_text_checkpoint_en.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_checkpoint_es.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_checkpoint_fr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_checkpoint_it.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\start_text_finallap_de.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_finallap_en.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_finallap_es.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_finallap_fr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_finallap_it.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\start_text_finish_de.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_finish_en.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_finish_es.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_finish_fr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_finish_it.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\start_text_start_de.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_start_en.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_start_es.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_start_fr.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
		{ "animations\\specialfx\\start_text_start_it.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\teamcolourcolumn_green.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "animations\\specialfx\\teamcolourcolumn_red.cycle", Animation::C_CycleResource::StaticGetResourceTypeId() },	
        { "graphics\\textures\\flares\\lightflare_diff.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\shadows\\impostershadow.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\arrow.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\checker_flag.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\gracia.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\hackney.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\la_docks.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\la_downtown.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\nyc_dumbo.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\russianhill.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\sausalito.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\shutoko.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\specialfx\\spotlight.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\splash\\crack.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\splash\\cracknormal.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\splash\\dirt.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\splash\\dot.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\splash\\mud.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\splash\\water.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "graphics\\textures\\splash\\waterrefract.texture", TextureResource::C_TextureResource::StaticGetResourceTypeId() },
        { "models\\perks\\bargebuild2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\bargebuild2_ai.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\bargebuildbumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\barged1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\bargeengulflevel2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\bargewave1left.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\bargewavebumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\bargewaveresidue.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\bargewavering.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\batteringram.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\batteringramkhan.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blind_build_01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blind_projectile_01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blindfire_build_rev.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blindfire_engulf_car.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blindfire_hit_01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blindfire_pow_01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blindfireshotgunbuild.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blindfireshotgunbuildrev.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\blindfireshotgunpow.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\bolt_shotgun_projectile.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\boltlasersightbeam.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\destructionmodecolumn_blue.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\destructionmodecolumn_green.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\destructionmodecolumn_orange.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\destructionmodecolumn_red.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\draftwind1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\draftwindfadein1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\draftwindfadeout1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\drifterspiral.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\garage_engulf_01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\garage_mystery_car.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\lastgaspshroud.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\lastgaspsphere.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mastermine01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mastermineballexpand.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mastermineblastresidue.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mastermineboomcircles.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\masterminecore1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mineballexpand.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mineblastimplode.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mineblastresidue.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mineboomcircles.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\minebuild1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\minebuild1reverse.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\minecore1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\minecore2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mined1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\minedtcore1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\mineengulflevel2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitro_carver.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitroengulflevel2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitrolevel1nr.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitrolevel2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitrolevel2nr.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitrolevel3.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitrolevel3bumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitrolevel3nr.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitroreverse01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitroreverseblastrings.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\nitroreversebumper01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\bargepickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\bargepickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\bargepickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\fangoldenpickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\fangoldenpickup_dimmed.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\fanpickup_aggressive.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\fanpickup_aggressive_dimmed.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\fanpickup_speed.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\fanpickup_speed_dimmed.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\minepickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\minepickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\minepickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\nitropickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\nitropickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\nitropickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\repairpickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\repairpickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\repairpickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shieldpickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shieldpickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shieldpickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shieldsuperpickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shieldsuperpickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shieldsuperpickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shockpickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shockpickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shockpickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shuntpickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shuntpickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\shuntpickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\stopwatchpickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\xppickup.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\xppickupdecoy.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\pickups\\xppickupdecoyhit.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\repairengulf.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\repairing1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\repairingbumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\repairzone1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\rift2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\rifted.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_barge_norotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_barge_rotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_bolt_norotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_bolt_rotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_mine_norotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_mine_rotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_nitro_norotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_nitro_rotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_repair_norotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_repair_rotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_shield_norotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_shield_rotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_shock_norotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_shock_rotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_shunt_norotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\roadsweeper_shunt_rotate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\safetynet_barge.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\safetynet_blindfire.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\safetynet_mine.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\safetynet_shock.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\safetynet_shunt.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\scramblerbubbleshort.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\scramblerbumperbubbleshort.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldblockbarge1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldblockblindfire1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldblockmine1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldblockshock1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldblockshunt1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldedbaybubble.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldedbaybumperbubble.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldedboosters.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldengulf1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldkhan1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldkhan2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldlevel1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldlevel1bumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldsuper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shieldsuperbumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shocked02.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shockengulf2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shockfiredown1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shockfireup1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shockfireup1ai.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shocklevel2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shocklevel2player.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shocksparks02.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shocksparks03.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shocktargetdown1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shunt_flow_car_02.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntbuild2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntbuild2bumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntbuildai.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntbuildrev.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shunted1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntedball1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shunthit1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntlevel1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntpow1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntpow2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntprojectile1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\shuntring1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\silentrunning.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\silentrunningai.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\splashdamagebargebuild.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\splashdamagebargebuildbumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\splashdamagebargewave.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\splashdamagebargewavebumper.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\splashdamagebargewaveresidue.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\splashdamagebargewavering.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\perks\\titaniumarmour.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\arrow_projection_smoke.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\arrow_projection_smoke_medium.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\aug_1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\aug_2.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\aug_3.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\aug_go.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\birds01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\blue_laser_01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\brighton_tide1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\corner_arrows.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\direction_ambient_spot_01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\dockslight.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\goldenfandemandgate.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\goldenfandemandgate_air.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\goldenfandemandgateend_air.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\green_laser_01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\helilight.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\lantern01.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\light_projection_beam.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\police_flashing_lights.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\saus_tide1.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\splitscreencolumn_bottomleft.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\splitscreencolumn_bottomright.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\splitscreencolumn_topleft.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\splitscreencolumn_topright.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\start_finish_line.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\start_line_ambient_spot_02.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\start_line_ambient_spot_03.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\teamcolourcolumn_green.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	
        { "models\\specialfx\\teamcolourcolumn_red.model|specialfx", GameResource::C_ModelResource::StaticGetResourceTypeId() },	

		{ "", -1 }
	};

	{
		Resource::C_HandleList* pList;
		C_GameApp::GetInstance()->LockCachePostBootAnimations(&pList);
		Resource::C_HandleList& globalCachedResourceHandleList = *pList;

		S32 resourceDefIndex = 0;
		while(globalResources[resourceDefIndex].m_Id != -1)
		{		
			globalCachedResourceHandleList.PushBack();
			Resource::C_HandleBase& handle = globalCachedResourceHandleList[resourceDefIndex];
			handle.Load( globalResources[resourceDefIndex].m_Name, globalResources[resourceDefIndex].m_Id );

			resourceDefIndex++;
		}

		C_GameApp::GetInstance()->UnlockCachePostBootAnimations();
	}
	MEMORY_TRACKER_POP("game.cachedresources");
	C_GameApp::GetInstance()->SetCachePostBootAnimationsStarted();
}

namespace Game
{
	XtPack* OpenSpecialFxPack();
}

void C_GameApp::CachePostBootFxResourceBag()
{
	if (m_cachePostBootFxResourceBagStarted)
		return;

	//HACK: Set language code so we can ignore certain files.
	SpecialFX::SfxSetLanguageCode(UI::C_UILocaliser::Instance().GetLanguageCode());

	#define NUM_SPECIALFX_PACKS 3
	XtPack* specialFxPacks[NUM_SPECIALFX_PACKS] = {0};

	//Precache special fx, passing thru pointers to any xt packs using special fx.
	{
		specialFxPacks[0] = Game::OpenSpecialFxPack();
		BIZ_ASSERT_SPEED(specialFxPacks[0]);

		{
			Game::GameXtPack* pGameXtPack = Game::GameXtLoader::Instance().GetPack(Game::E_XTPACK_PICKUPS);
			BIZ_ASSERT_SPEED(pGameXtPack);
			specialFxPacks[1] = pGameXtPack->mXtAmaxPack;
			BIZ_ASSERT_SPEED(specialFxPacks[1]);
		}

		{
			Game::GameXtPack* pGameXtPack = Game::GameXtLoader::Instance().GetPack(Game::E_XTPACK_RACELOGICDATABASE);
			BIZ_ASSERT_SPEED(pGameXtPack);
			specialFxPacks[2] = pGameXtPack->mXtAmaxPack;
			BIZ_ASSERT_SPEED(specialFxPacks[2]);
		}
	}

	//Ensure shaders are loaded before we continue.
	if (g_pBootCacheThread)
	{
		g_pBootCacheThread->Join();
		SAFE_DELETE_PTR(g_pBootCacheThread);
	}

	SCOPED_TIMER("CachePostBootFxResourceBag");

	{
		MEMORY_TRACKER_PUSH_SUB("fxresources");
		LOADING_PERF("CreateFxResources");

		const XtType* pXtType = XtGetType<SpecialFX::C_SpecialFX>();
		const XtType* pSplashType = XtGetType<SplashCamera::C_SplashType>();

		for (U32 itPack = 0; itPack < NUM_SPECIALFX_PACKS; ++itPack)
		{
			XtPack* pPack = specialFxPacks[itPack];

			for(size_t i = 0; i < pPack->count; ++i)
			{
				XtRecord* pRecord = pPack->records[i].record;
				if ((pRecord != NULL) && (pRecord->type == pXtType))
				{
					XtHandle<SpecialFX::C_SpecialFX>(pRecord)->AddToResourceBag(*C_GameApp::GetInstance()->GetSpecialFXResourceBag());
				}
				if ((pRecord != NULL) && (pRecord->type == pSplashType))
				{
					XtHandle<SplashCamera::C_SplashType>(pRecord)->AddToResourceBag(*C_GameApp::GetInstance()->GetSpecialFXResourceBag());
				}
			}
		}

		MEMORY_TRACKER_POP_SUB("fxresources");
	}
	C_GameApp::GetInstance()->SetCachePostBootFxResourceBagStarted();
}

void C_GameApp::SetCachePostBootFxResourceBagStarted()
{
	m_cachePostBootFxResourceBagStarted = true;
}

void C_GameApp::CachePostBootAnimations()
{
	if (m_cachePostBootAnimationsStarted)
		return;

#ifdef USE_RESOURCE_THREADS
	g_pPostBootCacheAnimationsThread = new C_RunFuncAsync(&CachePostBootAnimationsFunc);
	g_pPostBootCacheAnimationsThread->Start();
#else
	CachePostBootAnimationsFunc();
#endif
}

void C_GameApp::SetCachePostBootAnimationsStarted()
{
	m_cachePostBootAnimationsStarted = true;
}

bool C_GameApp::CachePostBootFxResourceBagReady()
{
	if (!m_cachePostBootFxResourceBagStarted)
		return false;
	
	if (GetSpecialFXResourceBag()->IsWaitingForLoad())
		return false;

	return true;
}

void C_GameApp::WaitForBootResources()
{
	SCOPED_TIMER("C_GameWorld::WaitForPostUIResources");
	
	// Wait for specialfx resource bag to have loaded it's resources
	{
		//g_SpecialFXResourceBag.DebugOutputBagContents();
		while( GetSpecialFXResourceBag()->IsWaitingForLoad() )
		{
			Resource::C_Manager::GetInstance()->PartialUpdate();
			#ifdef PLATFORM_PS3
			// without this 
			// we get "lv2(2): System Warning : busy loop detected" spam + game locks up for 
			// a couple seconds at a time off a blu-ray(tm).
			sys_timer_usleep( 50 );
			#endif
		}
	}

	if (g_pPostBootCacheAnimationsThread)
	{
		g_pPostBootCacheAnimationsThread->Join();
		SAFE_DELETE_PTR(g_pPostBootCacheAnimationsThread);
	}
}

void C_GameApp::LockCachePostBootAnimations(Resource::C_HandleList** ppResourceList)
{
	m_globalCachedResourceLock.Enter();
	*ppResourceList = &m_GlobalCachedResourceHandleList;
}

void C_GameApp::UnlockCachePostBootAnimations()
{
	m_globalCachedResourceLock.Exit();
}

void C_GameApp::FinishCacheBootGlobalResources()
{
	if (g_pBootCacheThread)
	{
		g_pBootCacheThread->Join();
		SAFE_DELETE_PTR(g_pBootCacheThread);
	}
}

void C_GameApp::CacheBootGlobalResources()
{
#ifdef USE_RESOURCE_THREADS
	g_pBootCacheThread = new C_RunFuncAsync(&CacheBootGlobalResourcesFunc);
	g_pBootCacheThread->Start();
#else
	CacheBootGlobalResourcesFunc();
#endif
}

void C_GameApp::LockCacheBootGlobalResources(
	DynamicEffect::Textures**	ppParticleTextures,
	Btl::Vector<FX::I_Effect*>** ppEffectPointers)
{
	m_bootGlobalResourcesLock.Enter();
	*ppParticleTextures = m_pParticleTextures;
	*ppEffectPointers = &m_EffectPointers;
}

void C_GameApp::UnlockCacheBootGlobalResources()
{
	m_bootGlobalResourcesLock.Exit();
}

void C_GameApp::ReleaseAllGlobalResources()
{
	if (g_pBootCacheThread)
	{
		g_pBootCacheThread->Join();
		SAFE_DELETE_PTR(g_pBootCacheThread);
	}
	if (g_pPostBootCacheAnimationsThread)
	{
		g_pPostBootCacheAnimationsThread->Join();
		SAFE_DELETE_PTR(g_pPostBootCacheAnimationsThread);
	}

	Resource::C_HandleList* pGlobalCachedResourceHandleList;
	C_GameApp::GetInstance()->LockCachePostBootAnimations(&pGlobalCachedResourceHandleList);
	pGlobalCachedResourceHandleList->Resize(0);
	C_GameApp::GetInstance()->UnlockCachePostBootAnimations();

	for (int i=0; i<m_EffectPointers.Size(); i++)
	{
		if (m_EffectPointers[i])
			FX::ReleaseEffect(m_EffectPointers[i]);
	}

	for(DynamicEffect::Textures::Iterator pIterator = m_pParticleTextures->Begin(); pIterator != m_pParticleTextures->End(); )
	{
		DynamicEffect::Textures::Iterator pNext = pIterator;
#if PLATFORM_PC
		++pNext;	// [Blur2PC] retail (Blur.exe 1.2 0x00433380) walks to the next texture; the snapshot never advanced and destroyed the first one twice
#endif
		Display::C_RefTexture* pRefTexture = pIterator->GetValue();
		Display::Device()->Destroy(pRefTexture);

		delete pRefTexture;
#if PLATFORM_PC
		pIterator = pNext;	// [Blur2PC]
#endif
	}
	m_pParticleTextures->Clear();
}

#ifndef CFG_GOLD
Debug::Bool g_FakeStillBootCaching( "GameSys.FakeStillBootCaching", false );
#endif

bool C_GameApp::IsBootPreCachingComplete()
{
#ifndef CFG_GOLD
	// Fake 
	if( g_FakeStillBootCaching )
		return false;
#endif

	return m_setBootPreCachingComplete;
}


void C_GameApp::SetBootPreCachingComplete()
{
	m_setBootPreCachingComplete = true;
}

void C_GameApp::EnableInstallerScreen(bool enable)
{
	m_installerScreenEnabled = enable;
}

void C_GameApp::EnableLoadingSeg(bool enable)
{
	m_loadingSegEnabled = enable;
}

bool C_GameApp::IsInstallerScreenEnabled() const
{
	return m_installerScreenEnabled;
}

bool C_GameApp::IsLoadingSegEnabled() const
{
	return m_loadingSegEnabled;
}
