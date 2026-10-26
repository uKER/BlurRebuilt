#ifndef EDITOR_PROPS_RENDER_DISTANCE_PROP_NODE_H
#define EDITOR_PROPS_RENDER_DISTANCE_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class C_RenderDistancePropNode : public I_PropertyNode
	{
	public:
		C_RenderDistancePropNode(
			C_ActorScene* pScene, 
			const EditorLib::C_DgXtPropNodeBase* pDgNode);
		virtual							~C_RenderDistancePropNode();

		static const char*				GetStaticTypeName();

		static const char*				GetStaticDisplayName();

		virtual const char*				GetTypeName() const;

		static EditorLib::E_ObjType		GetStaticType();

		static void*					Create();
		static void						SimpleRendererRegister();

		virtual void					OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode);
		virtual void					UpdateCache() const;

		virtual bool					GetVisible(const RECT& region, C_DgNodeSelection *pResult);
		virtual void					AttachToRenderer(
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);

	private:
		//we have to link to the render scene nodes as the backend dgnodes
		//may be out of sync, eg in the middle of a manipulator operation
		const AftkRender::C_SceneGraphNode* m_linkedNodes[2];

		void Render() const;
		void RenderText() const;
	};
}

#endif
