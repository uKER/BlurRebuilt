#ifndef SECTIONPARTICLES_H
#define SECTIONPARTICLES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Samples/GameFunctionality.h"
#include "Display/Utils/Matrix.h"
#include "Render/PostProcess/Manager.h"
#include "Render/Sky/SkyDome.h"
#ifndef PLATFORM_PS3
#include "Render/Effects/Perlin/PerlinCPU.h"
#endif
#include "Render/Effects/Perlin/Perlin.h"
#include "Dynamic/Node.h"

namespace DynamicEffect
{
	class C_Renderer;
	class C_SampleClothRenderer;

namespace Particle
{
	class C_ParticleManager;
	class I_ParticleEffectBase;
	class C_Emitter;
	class C_Force;
}

namespace Strip
{
	class C_Manager;
	class C_Beam;
}

	class C_Minimal;

	//class C_Sprite2D;
}

namespace PostProcess
{
	class C_GaussianBlur;
	class C_Greyscale;
	class C_Static;
	class C_GaussianScale;
	class C_BoxBlur;
	class C_HighDynamicRange;
	class C_ShowDepth;
	class C_Tint;
}

namespace ReducedRender
{
	class C_Manager;
}

namespace Render
{
namespace Debug
{
	class C_DebugRenderSys;
}
namespace Chart
{
	class C_ChartRenderer;
	class C_LineChart;
}
}

class C_CustomHDR;

namespace Samples
{

class C_SectionParticles : public C_Section
{
public:

	C_SectionParticles(const Util::C_String& name);

	~C_SectionParticles();

	bool OnCreate();
	bool OnCreateVolatile();

	void OnDestroyVolatile();
	void OnDestroy();

	void OnResize();
	
	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	
	void OnBeginRenderSplitscreenWithoutAA();
	void OnEndRenderSplitscreenWithoutAA();
	void OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer);
	void OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer);

private:

	F32									m_fTime;
	F32									m_fCycle;
	U32									m_iCount;

	Display::C_Texture2D				m_DepthTexture;
	Display::C_Texture2D				m_OverlapTexture0;
	Display::C_Texture2D				m_OverlapTexture1;

	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;
	DynamicEffect::C_Renderer*			m_pDynamicEffectRenderer;

	// strips
	DynamicEffect::Strip::C_Manager*	m_pStripManager;
	DynamicEffect::Strip::C_Beam*		m_pBeam;
	
	// particles
	DynamicEffect::Particle::C_ParticleManager*	m_pParticleManager;
	
	// minimal
	DynamicEffect::C_Minimal*			m_pMinimalDynamicEffect;

	// cloth
	Display::C_DynamicVB*				m_pDynamicVB;

	Render::Sky::C_SkyDome				m_SkyDome;

	//DynamicEffect::C_Sprite2D*			m_pSprite2D;

	Render::Chart::C_ChartRenderer*		m_pChartRenderer;
	Render::Chart::C_LineChart*			m_pFpsChart;

#ifndef PLATFORM_PS3
	PerlinCPU::C_Perlin*				m_pPerlinCPU;
#endif
	Perlin::C_Perlin*					m_pPerlin;

	ReducedRender::C_Manager*			m_pReducedRender;

	PostProcess::C_Manager*				m_pPostProcessing;
	PostProcess::C_GaussianBlur*		m_pBlur;
	PostProcess::C_BoxBlur*				m_pBoxBlur;
	PostProcess::C_Greyscale*			m_pGreyscale;
	PostProcess::C_GaussianScale*		m_pGaussianScale;
	PostProcess::C_Static*				m_pStatic;
	PostProcess::C_HighDynamicRange*	m_pHighDynamicRange;
	PostProcess::C_ShowDepth*			m_pShowDepth;
	PostProcess::C_Tint*				m_pTint;
	Display::C_Matrix					m_Matrices[512];
	Display::C_Matrix					m_Matrices2[8];
	//DynamicEffect::ClothInstance			m_Instances[512];
	//DynamicEffect::ClothInstance			m_Instances2[512];
	Display::C_Texture2D					m_FlagTextures[3];

	ProjectileList						m_Projectiles;
	ProjectilePool						m_ProjectilePool;
};

} // namespace Samples

#endif // SECTIONPARTICLES_H
