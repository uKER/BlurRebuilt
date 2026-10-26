#ifndef EDITOR_ACTOR_PROPERTY_NODE_H
#define EDITOR_ACTOR_PROPERTY_NODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgNodeBase.h>
#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace Editor
{
	class I_PropertyNode : public C_DgNodeBase
	{
	public:
		I_PropertyNode (
			C_ActorScene *pScene, 
			const EditorLib::C_DgXtPropNodeBase* pProp) :
		C_DgNodeBase(pScene, pProp) {}

		virtual ~I_PropertyNode () {}

		virtual AftkRender::E_SceneGraphNodeType GetType() const
		{
			return AftkRender::E_SceneGraphNode_Custom;
		}

		virtual bool IsA(AftkRender::E_SceneGraphNodeType type) const
		{
			return (type==
				AftkRender::E_SceneGraphNode_Custom || 
				AftkRender::C_SceneGraphNode::IsA(type));
		}

		virtual void UpdateCache() const = 0;

		virtual void AttachToRenderer(
			AftkRender::E_RendererPass pass = AftkRender::E_RendererPass_Normal) = 0;

	protected:
		void AttachToRenderer(
			U8 lod=0, 
			AftkRender::E_RendererPass pass = AftkRender::E_RendererPass_Normal,
			bool recurse=true)
		{ 
			return AttachToRenderer(pass);
		}

	};
}

#endif
