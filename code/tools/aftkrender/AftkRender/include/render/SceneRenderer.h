#ifndef AFTK_RENDERER_SCENE_RENDERER_H
#define AFTK_RENDERER_SCENE_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "common/Renderer.h"

namespace AftkRender
{
	class C_SceneGraphNode;
	class C_GeometryNode;

	class C_SceneRenderer : public C_Renderer
	{
	public:

		struct Flags
		{
			union
			{
				struct
				{
					U32  fillmode	:3;	//Display::E_FillMode
					U32  cullmode	:3; //Display::E_CullMode
					U32  xray		:1;
					U32  textured	:1;
					U32  lit		:1;
				};
				U32 value;
			};

			Flags()
			{
				value = 0;
				fillmode = Display::E_FillMode_Default;
				cullmode = Display::E_CullMode_Default;
				xray = false;
				textured = false;
				lit = true;
			}
		};

		C_SceneRenderer();
		virtual ~C_SceneRenderer();

		void Clear();
		virtual void Update(const Maths::C_TimeS& timeStep, E_RendererPass pass=E_RendererPass_Normal);
		void Render();
		bool IsVisible(const RECT &region, const C_SceneGraphNode* pNode);

		bool LoadDefaultShaders();

		void AttachSceneGraphNode(C_SceneGraphNode* pSgNode);
		void DetachSceneGraphNode(const C_SceneGraphNode* pSgNode);
		virtual void AddRenderContext(C_RenderContext& rc, const C_SceneGraphNode* pNode);

		const Flags& GetFlags() const;
		void SetFlags(const Flags& flags);

	protected:

		void SortNodes();
		static bool Compare(const Ut::Pair<const C_SceneGraphNode*, C_RenderContext>& l,
							const Ut::Pair<const C_SceneGraphNode*, C_RenderContext>& r);

		Ut::UtVector<Ut::DynamicVector<
			Ut::Pair<const C_SceneGraphNode*, C_RenderContext> > >	m_RenderContextArray;

		Ut::UtVector<Ut::DynamicVector<C_SceneGraphNode*> >	m_SceneGraphNodeArray;

		Flags	m_Flags;
		
	};

	C_SceneRenderer* GetSceneRenderer();
	void PushSceneRenderer(C_SceneRenderer* pRenderer);
	C_SceneRenderer* PopSceneRenderer();

}

#endif