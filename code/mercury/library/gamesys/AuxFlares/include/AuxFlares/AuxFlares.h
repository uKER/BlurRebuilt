#ifndef AUXFLARES_H
#define AUXFLARES_H

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

namespace AuxFlares
{
	// single flare instance, gets inserted into visible node list
	class C_AuxFlareInstance : public SceneGraph::I_RenderNode
	{
		Maths::HVec3		m_Position;
		Maths::HVec3		m_Direction;
		Maths::C_RGBA_F32	m_Colour;
		F32					m_Intensity;
		F32					m_Size;
		U32					m_ViewFlags;

		friend class C_AuxFlares;

	public:
		C_AuxFlareInstance() : SceneGraph::I_RenderNode()
		{
			m_ViewFlags = 0xFFFFFFFF;
		}
		void SetViewFlags(U32 viewFlags) { m_ViewFlags = viewFlags;}
		void UpdatePosition(const Maths::HVec3& position) {m_Position = position;}
		void UpdateDirection(const Maths::HVec3& direction) {m_Direction = direction;}
		void UpdateColour(const Maths::C_RGBA_F32 colour) { m_Colour = colour;}
		void UpdateSize(const F32 size) { m_Size = size;}
		void UpdateIntensity(const F32 intensity) { m_Intensity = intensity;}
	};

	struct ViewSettings
	{
		ViewSettings() : sizeX(1.0f), sizeY(1.0f), nearDistanceFade(100.f), farDistanceFade(150.f) {}
		~ViewSettings() {}

		F32					sizeX;
		F32					sizeY;
		Maths::C_RGBA_F32	colour;
		F32					nearDistanceFade;
		F32					farDistanceFade;
	};

	// we integrate this into the scenegraph using both a gather node and a renderer
	class C_AuxFlares : public Display::I_Renderer, public SceneGraph::I_GatherNode, public SceneGraph::I_Renderer, public Resource::C_Listener
	{
		//List of aux flares
		typedef Btl::List<C_AuxFlareInstance, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter> >	FlareList;
		
		FlareList								m_AuxFlareList;
		Btl::Vector<ViewSettings>				m_ViewSettings;

		Display::C_VertexShaderParameter		m_FlareSizeParameter;
		Display::C_VertexShaderParameter		m_FlareColourParameter;
		Display::C_VertexShader					m_FlareVertexShader;
		Display::C_PixelShader					m_FlarePixelShader;

		U32										m_RenderPass;
		mutable	U32								m_BufferIndex;
		bool									m_bCreated;
		volatile bool							m_resourcesPending;

		Util::C_String							m_FlareTextureName;
		TextureResource::C_TextureHandle		m_FlareTextureHandle;
		Display::C_SamplerState					m_SamplerState;

		void AddFlare(volatile Display::Vertex_XYZ_HDR_UV_N* pVertices,  const C_AuxFlareInstance& flare, const F32 fadeFactor) const;

	public:
		C_AuxFlares(SceneGraph::C_SceneGraph* pSceneGraph, const char* flareTextureName);
		virtual ~C_AuxFlares();

		C_AuxFlareInstance* AllocAuxFlareInstance();

		S32		GetNumFlares() { return m_AuxFlareList.Size(); }
		U32		GetBufferIndex() const { return m_BufferIndex; }

		bool	OnCreate();
		void	OnDestroy();

		// setup
		void	SetRenderPass(const U32 renderPass);
		void	SetNumViewSettings(const S32 flareSettings);
		void	SetViewSettings(const S32 setting, F32 sizeX, F32 sizeY, const Maths::C_RGBA_F32& colour);
		void	SetViewNearFarSettings(const S32 setting, F32 nearDistanceFade, F32 farDistanceFade);
		
		// scene graph I_GatherNode implementation
		void	Accept(SceneGraph::I_Visitor& visitor) const {}
		void	OnGather(const SceneGraph::C_View& view) const;
		
		// scene graph I_Renderer implementation
		void	Render(const SceneGraph::C_View& view, SceneGraph::C_Context& context, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const;

		bool	IsCreated() { return m_bCreated; }

	// inherited from Resource::C_Listener
	public:
		virtual void ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData);
	};

	INLINE void C_AuxFlares::SetRenderPass(const U32 renderPass)
	{
		m_RenderPass = renderPass;
	}

	INLINE void C_AuxFlares::SetNumViewSettings(const S32 flareSettings)
	{
		 m_ViewSettings.Resize(flareSettings);
	}

	INLINE void C_AuxFlares::SetViewSettings(const S32 setting, F32 sizeX, F32 sizeY, const Maths::C_RGBA_F32& colour)
	{
		m_ViewSettings[setting].sizeX = sizeX;
		m_ViewSettings[setting].sizeY = sizeY;
		m_ViewSettings[setting].colour = colour;
	}

	INLINE void	C_AuxFlares::SetViewNearFarSettings(const S32 setting, F32 nearDistanceFade, F32 farDistanceFade)
	{
		m_ViewSettings[setting].nearDistanceFade = nearDistanceFade;
		m_ViewSettings[setting].farDistanceFade = farDistanceFade;
	}
}

#endif
