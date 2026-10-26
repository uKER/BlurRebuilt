#ifndef TEST_SECTION_PARTICLES_H
#define TEST_SECTION_PARTICLES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "TestDynamic/GameFunctionality.h"
#include "Display/Utils/Matrix.h"
#include "Render/PostProcess/Manager.h"
#include "Render/Sky/SkyDome.h"
#ifndef PLATFORM_PS3
#include "Render/Effects/Perlin/PerlinCPU.h"
#endif
#include "Render/Effects/Perlin/Perlin.h"
#include "Dynamic/Node.h"

// jobqueue
#include <system/scheduler/jobqueue.h>

namespace DynamicEffect { class C_Renderer; }
namespace DynamicEffect { class C_SampleClothRenderer; }
namespace DynamicEffect { namespace Particle { class C_ParticleManager; } }
namespace DynamicEffect { namespace Particle { class I_ParticleEffectBase; } }
namespace DynamicEffect { namespace Particle { class C_Emitter; } }
namespace DynamicEffect { namespace Particle { class C_Force; } }
namespace DynamicEffect { namespace Strip { class C_Manager; } }
namespace DynamicEffect { namespace Strip { class C_Beam; } }
namespace DynamicEffect { class C_Minimal; }

namespace PostProcess { class C_GaussianBlur; }
namespace PostProcess { class C_Greyscale; }
namespace PostProcess { class C_Static; }
namespace PostProcess { class C_GaussianScale; }
namespace PostProcess { class C_BoxBlur; }
namespace PostProcess { class C_HighDynamicRange; }
namespace PostProcess { class C_ShowDepth; }
namespace PostProcess { class C_Tint; }
namespace PostProcess { class C_MotionBlur; }

namespace ReducedRender { class C_Manager; }

namespace Render { namespace Debug { class C_DebugRenderSys; } }
namespace Render { namespace Chart { class C_ChartRenderer; } }
namespace Render { namespace Chart { class C_LineChart; } }

namespace SceneGraph { class C_SceneGraph; }
namespace SceneGraph { class C_View; }
namespace SceneGraph { class C_BackFrontSorter; }

class C_CustomHDR;

class C_TestSectionParticles : public Framework::C_TestSection
{
public:

	C_TestSectionParticles(const Util::C_String& name);
	~C_TestSectionParticles();

	bool OnCreate();
	bool OnCreateVolatile();

	void OnDestroyVolatile();
	void OnDestroy();

	void OnResize();
	
	void OnUpdate();
	void OnPreRender();
	
	void OnBeginPostEffects();
	void OnEndPostEffects();
	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender3DViewWithoutAA(const S32 viewIndex);

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
#ifdef PLATFORM_XENON
	DynamicEffect::C_SampleClothRenderer*	m_pClothRenderer;
#endif
	Display::C_DynamicVB*				m_pDynamicVB;

	Render::Sky::C_SkyDome				m_SkyDome;

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
#ifdef PLATFORM_XENON
	PostProcess::C_MotionBlur*			m_pMotionBlur;
#endif
	Display::C_Matrix					m_Matrices[512];
	Display::C_Matrix					m_Matrices2[8];
	Display::C_Texture2D					m_FlagTextures[3];

	ProjectileList						m_Projectiles;
	ProjectilePool						m_ProjectilePool;

	SceneGraph::C_SceneGraph*			m_pSceneGraph;
	SceneGraph::C_View*					m_pSceneGraphView[4];
	SceneGraph::C_BackFrontSorter*		m_pBackFrontSorter;

	Scheduler::PolicyHandle				m_JobQueuePolicyHandle;
	Scheduler::Policy					m_JobQueuePolicy[8];

};

#endif // TEST_SECTION_PARTICLES_H
