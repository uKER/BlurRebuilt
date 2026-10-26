#ifndef LIGHTTRAILS_H
#define LIGHTTRAILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <display/utils/Renderer.h>
#include <display/buffer/VertexDeclarations.h>
#include <TextureResource/TextureResource.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/GatherNode.h>
#include <SceneGraph/RendererView.h>
#include <SceneGraph/RendererInterface.h>
#include <Display/context/SamplerState.h>

namespace SceneGraph
{
	class C_SceneGraph;
}

#define LIGHTTRAILS_SIZE 6

namespace LightTrails
{
	// LightTrail instance
	class C_LightTrailInstance : public SceneGraph::I_RenderNode
	{
		U32					m_ViewFlags;

		//Trail
		Maths::HVec3		m_Position[LIGHTTRAILS_SIZE];
		Maths::HVec3		m_VectorX[LIGHTTRAILS_SIZE];
		Maths::HVec3		m_VectorY[LIGHTTRAILS_SIZE];
		F32					m_Intensity[LIGHTTRAILS_SIZE];
		
		U32					m_RingBufferIndex;
		U32					m_LastActivateIndex;

		Maths::C_RGBA_F32	m_Colour;
		F32					m_SizeX;
		F32					m_SizeY;
		F32					m_CameraFadeFactor;

		friend class C_LightTrails;

		const Maths::HVec3& GetLastPosition() const { return m_Position[(m_RingBufferIndex - 1 + LIGHTTRAILS_SIZE) % LIGHTTRAILS_SIZE];};
		const Maths::HVec3& GetFirstPosition() const { return m_Position[(m_RingBufferIndex + 1 + LIGHTTRAILS_SIZE) % LIGHTTRAILS_SIZE];};

	public:
		C_LightTrailInstance() : SceneGraph::I_RenderNode()
		{
			m_ViewFlags = 0xFFFFFFFF;
			m_RingBufferIndex = 0;
			m_CameraFadeFactor = 0.5f;
			m_LastActivateIndex = 0;
		}

		void SetViewFlags(U32 viewFlags) { m_ViewFlags = viewFlags;}
		void SetColour(const Maths::C_RGBA_F32 colour) { m_Colour = colour;}
		void SetSize(const F32 sizeX, const F32 sizeY) { m_SizeX = sizeX; m_SizeY = sizeY;}
		void SetCameraFadeFactor(const F32 cameraFadeFactor) { m_CameraFadeFactor = cameraFadeFactor;}

		void Reset(const Maths::HVec3& position);
		void Update(const Maths::HVec3& position, const Maths::HVec3& vectorX, const Maths::HVec3& vectorY, const F32& intensity);
	};

	// we integrate this into the scenegraph using both a gather node and a renderer
	class C_LightTrails : public Display::I_Renderer, public SceneGraph::I_GatherNode, public SceneGraph::I_Renderer
	{
		//List of LightTrails
		typedef Btl::List<C_LightTrailInstance, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter> >	LightTrailList;
		
		LightTrailList							m_LightTrailList;

		Display::C_VertexShader					m_LightTrailVertexShader;
		Display::C_PixelShader					m_LightTrailPixelShader;

		Display::C_VertexShader					m_ParticleLightTrailVertexShader;
		Display::C_PixelShader					m_ParticleLightTrailPixelShader;

		F32										m_IntensityVector[LIGHTTRAILS_SIZE];

		U32										m_RenderPass;
		U32										m_ViewFlags;
		bool									m_bCreated;

		F32										m_NearDistanceFade;
		F32										m_FarDistanceFade;

		void AddLightTrail(volatile Display::Vertex_XYZ_HDR_N2* pVertices,  const C_LightTrailInstance& lightTrail, const F32 fadeFactor ) const;
		void AddParticleLightTrail(volatile Display::Vertex_XYZ_HDR_N2* pVertices,  const C_LightTrailInstance& lightTrail, const F32 fadeFactor ) const;

	public:
		C_LightTrails(SceneGraph::C_SceneGraph* pSceneGraph);
		virtual ~C_LightTrails();

		C_LightTrailInstance* AllocLightTrailInstance();
		S32		GetNumLightTrails() { return m_LightTrailList.Size(); }
		void	Update();
		void	SetIntensityVector(U32 index, F32 value) { m_IntensityVector[index] = value;};

		bool	OnCreate();
		void	OnDestroy();

		// setup
		void	SetRenderPass(const U32 renderPass);
		void	SetViewFlags(U32 viewFlags);
		void	SetNearFarDistanceFade(const F32& nearDistanceFade, const F32& farDistanceFade) { m_NearDistanceFade = nearDistanceFade; m_FarDistanceFade = farDistanceFade;};
		
		// scene graph I_GatherNode implementation
		void	Accept(SceneGraph::I_Visitor& visitor) const {}
		void	OnGather(const SceneGraph::C_View& view) const;
		
		// scene graph I_Renderer implementation
		void	Render(const SceneGraph::C_View& view, SceneGraph::C_Context& context, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const;

		bool	IsCreated() { return m_bCreated; }
	};

	INLINE void C_LightTrails::SetRenderPass(const U32 renderPass)
	{
		m_RenderPass = renderPass;
	}
	INLINE void C_LightTrails::SetViewFlags(U32 viewFlags)
	{
		m_ViewFlags = viewFlags;
	}
}

#endif
