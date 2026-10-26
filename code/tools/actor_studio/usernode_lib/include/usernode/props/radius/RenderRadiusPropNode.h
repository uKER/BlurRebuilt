#ifndef EDITOR_PROPS_RENDER_RADIUS_PROP_NODE_H
#define EDITOR_PROPS_RENDER_RADIUS_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class C_RenderRadiusPropNode : public I_PropertyNode
	{
	public:
		C_RenderRadiusPropNode(
			C_ActorScene* pScene, 
			const EditorLib::C_DgXtPropNodeBase* pDgNode);
		virtual							~C_RenderRadiusPropNode();

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

		//manipulator context update
		void ScaleRadii(F32 scale, const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds);

	private:
		void RenderRing(F32 innerRadius, F32 outerRadius, const Maths::C_RGBA_U8& colour);

		mutable Btl::Vector<Maths::HMat44> m_matrixCache;
		mutable Btl::Vector<F32> m_scaleCache;

	};
}

#endif
