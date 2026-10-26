#ifndef AFTK_RENDER_SCENE_NODE_INCLUDE
#define AFTK_RENDER_SCENE_NODE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_SceneNode;
}

namespace AftkRender
{

	enum E_RenderNodeType
	{
		E_RenderNodeType_Node=0,
		E_RenderNodeType_Node_Instance,
		E_RenderNodeType_Mesh,
		E_RenderNodeType_Mesh_Remappable,
		E_RenderNodeType_Mesh_Interface,
		E_RenderNodeType_Mesh_Instance,
		E_RenderNodeType_Mesh_Boned_Instance,

		E_RenderNodeType_Count
	};


	class C_SceneRenderer;

	class C_RenderNode
	{
	public:
		struct C_Flags
		{
			unsigned isBuilt:1;
			unsigned isStatic:1;
			unsigned isSkinned:1;
			unsigned isSkinAnimated:1;
		};

		C_RenderNode(
			Aftk::C_SceneNode* pAftkSceneNode,
			C_SceneRenderer *pRenderer);

		virtual ~C_RenderNode();

		virtual E_RenderNodeType GetType() const;

		virtual bool HasType(
			E_RenderNodeType type) const;

		const Aftk::C_SceneNode* GetAftkSceneNodePtr() const;

		Aftk::C_SceneNode* GetAftkSceneNodePtr();

		U32 GetNumChildren() const;

		const C_RenderNode* GetChildPtr(
			U32 child) const;

		void AddChild(
			C_RenderNode* pNode);

		C_SceneRenderer* GetRenderer() const;

		const Maths::HMat44& GetLocalMatrix();

		const Maths::HMat44& GetWorldMatrix();

		const Maths::HMat44& GetWorldInverseTransposeMatrix();

		const Maths::C_AxisBox& GetWorldAABBox();

		bool IsSkinned() const;
		
		bool IsSkinAnimated() const;

	protected:
		C_RenderNode();

		C_RenderNode(
			const C_RenderNode& node);

		const C_RenderNode& operator=(
			const C_RenderNode& rhs);
		
		C_SceneRenderer *m_pRenderer;

		Aftk::C_SceneNode* m_pAftkSceneNode;

		C_RenderNode* m_pParent;

		Ut::UtVector< Ut::DynamicVector<C_RenderNode*> > m_Children;

		Maths::HMat44	m_LocalMatrix;
		Maths::HMat44	m_WorldMatrix;
		Maths::HMat44	m_WorldInverseTransposeMatrix;

		Maths::C_AxisBox m_WorldAABBox;
		C_Flags m_RenderNodeFlags;
	};

	inline const Maths::HMat44& C_RenderNode::GetLocalMatrix()
	{
		return m_LocalMatrix;
	}

	inline const Maths::HMat44& C_RenderNode::GetWorldMatrix()
	{
		return m_WorldMatrix;
	}

	inline const Maths::HMat44& C_RenderNode::GetWorldInverseTransposeMatrix()
	{
		return m_WorldInverseTransposeMatrix;
	}

	inline const Maths::C_AxisBox& C_RenderNode::GetWorldAABBox()
	{
		return m_WorldAABBox;
	}
}

#endif