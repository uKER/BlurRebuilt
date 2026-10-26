#ifndef EDITOR_PROPS_RENDER_LINK_PROP_NODE_H
#define EDITOR_PROPS_RENDER_LINK_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class C_RenderLinkPropNode : public I_PropertyNode
	{
	public:
		C_RenderLinkPropNode(
			C_ActorScene* pScene, 
			const EditorLib::C_DgXtPropNodeBase* pDgNode);
		virtual							~C_RenderLinkPropNode();

		static const char*				GetStaticTypeName();

		static const char*				GetStaticDisplayName();

		virtual const char*				GetTypeName() const;

		static EditorLib::E_ObjType		GetStaticType();

		static void*					Create();
		static void						SimpleRendererRegister();

		virtual void					OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode);
		virtual void					OnNodeResolved(const EditorLib::I_DgNode* pDgNode);
		virtual void					UpdateCache() const;

		virtual bool					GetVisible(const RECT& region, C_DgNodeSelection *pResult);
		virtual void					AttachToRenderer(
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);

	private:
		//we have to link to the render scene nodes as the backend dgnodes
		//may be out of sync, eg in the middle of a manipulator operation
		const AftkRender::C_SceneGraphNode* m_pLinkedNode;

		void Render() const;

		U32 m_style;
		U32 m_colourStart, m_colourEnd;

	};
}

#endif
