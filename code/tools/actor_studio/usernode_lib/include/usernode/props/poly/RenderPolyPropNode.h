#ifndef EDITOR_PROPS_RENDER_Poly_PROP_NODE_H
#define EDITOR_PROPS_RENDER_Poly_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class C_RenderPolyPropNode : public I_PropertyNode, public I_ComplexObjectCallback
	{
	public:
		C_RenderPolyPropNode(
			C_ActorScene* pScene, 
			const EditorLib::C_DgXtPropNodeBase* pDgNode);
		virtual							~C_RenderPolyPropNode();

		static const char*				GetStaticTypeName();

		static const char*				GetStaticDisplayName();

		virtual const char*				GetTypeName() const;

		static EditorLib::E_ObjType		GetStaticType();

		static void*					Create();
		static void						SimpleRendererRegister();

		void							UpdateOffset(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds, const Maths::HVec3& offset);
		virtual void					OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode);
		virtual void					UpdateCache() const;
		void							UpdatePoly(const Ut::UtVector< Ut::DynamicVector<U32> >* componentIds=0, const Maths::HVec3* offset=0);

		virtual void					ComplexRendererCallback(ComplexObject* object);

		virtual bool					GetVisible(const RECT& region, C_DgNodeSelection *pResult);
		virtual void					AttachToRenderer(
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);

		virtual Maths::C_AxisBox		GetSelectionAABBox() const;

	private:
		ComplexObject*	m_pVertexBuffer;
		U32				m_nVertices;
		Maths::HMat44	m_probeTransform;

		void FillVertexBuffer();
	};
}

#endif
