#ifndef EDITOR_PROPS_RENDER_SPHERE_PROP_NODE_H
#define EDITOR_PROPS_RENDER_SPHERE_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class C_RenderSpherePropNode : public I_PropertyNode
	{
	public:
		C_RenderSpherePropNode(
			C_ActorScene* pScene, 
			const EditorLib::C_DgXtPropNodeBase* pDgNode);
		virtual							~C_RenderSpherePropNode();

		static const char*				GetStaticTypeName();

		static const char*				GetStaticDisplayName();

		virtual const char*				GetTypeName() const;

		static EditorLib::E_ObjType		GetStaticType();

		static void*					Create();
		static void						SimpleRendererRegister();

		virtual void					OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode);
		virtual void					UpdateCache() const;

		virtual void					AttachToRenderer(
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);

	private:
	};
}

#endif
