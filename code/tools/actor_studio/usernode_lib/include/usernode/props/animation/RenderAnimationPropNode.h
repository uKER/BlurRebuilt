#ifndef EDITOR_PROPS_RENDER_ANIMATION_PROP_NODE_H
#define EDITOR_PROPS_RENDER_ANIMATION_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class ComplexObject;

	class C_RenderAnimationPropNode : public I_PropertyNode, public I_ComplexObjectCallback
	{
	public:
		C_RenderAnimationPropNode(
			C_ActorScene* pScene, 
			const EditorLib::C_DgXtPropNodeBase* pDgNode);
		virtual							~C_RenderAnimationPropNode();

		static const char*				GetStaticTypeName();

		static const char*				GetStaticDisplayName();

		virtual const char*				GetTypeName() const;

		static EditorLib::E_ObjType		GetStaticType();

		static void*					Create();
		static void						SimpleRendererRegister();

		virtual void					OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode);
		virtual void					OnNodeResolved(const EditorLib::I_DgNode* pDgNode);
		virtual void					UpdateCache() const;
		void							UpdateAnimation(const Ut::UtVector< Ut::DynamicVector<U32> >* componentIds=0, const Maths::HVec3* offset=0, const F32* tension=0);
		void							UpdateVisualisationDef();

		Maths::HVec3					GetPosition(EditorLib::C_DgAnimationPropNode::Object* object);
		Maths::HVec3					GetRotation(EditorLib::C_DgAnimationPropNode::Object* object);

		void							RegisterPositionCallback(const Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>& pHandler);
		void							RegisterRotationCallback(const Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>& pHandler);
		void							UnregisterCallbacks();

		virtual void					ComplexRendererCallback(ComplexObject* object);

		virtual void					AttachToRenderer(
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);

		virtual Maths::C_AxisBox		GetSelectionAABBox() const;

		Maths::HMat44&					GetLocalTransform(int object);
		void							ApplyTranslate(const Ut::UtVector< Ut::DynamicVector<U32> >* componentIds, const Maths::HVec3& offset);
		void							ApplyRotate(const Ut::UtVector< Ut::DynamicVector<U32> >* componentIds, const Maths::C_Quaternion& quat);

		void							SetAbsoluteTransform(EditorLib::C_DgAnimationPropNode::AnimationCurve* curve, float value);

		void SetTimelineValue(double timeline);
		double GetTimelineValue();

	private:
		ComplexObject*	m_pVertexBuffer;
		int m_VertexCount;
		
		Btl::Vector<Btl::String> m_Visualisations;

		Btl::Vector<Maths::HMat44> m_LocalAnimationTransforms;
		Btl::Vector<Maths::HMat44> m_WorldAnimationTransforms;

		Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>* m_pPositionCallback;
		Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>* m_pRotationCallback;

		double m_Timeline;

		void FillVertexBuffer();
	};
}

#endif
