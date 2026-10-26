#ifndef AFTK_RENDER_SCENE_GRAPH_NODE_H
#define AFTK_RENDER_SCENE_GRAPH_NODE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	extern Debug::Logger& g_Log;

	class C_Scene;
	class C_Renderer;
	class C_RenderContext;


	enum E_SceneGraphNodeType
	{
		E_SceneGraphNode_Base,
		E_SceneGraphNode_NativeMesh,
		E_SceneGraphNode_NativeSkinnedMesh,

		E_SceneGraphNode_Aftk,
		E_SceneGraphNode_AftkMesh,
		E_SceneGraphNode_AftkSkinnedMesh,

		E_SceneGraphNode_Custom
	};


	class C_SceneGraphNode
	{
	public:

		struct Flags
		{
			union
			{
				struct
				{
					U32 hidden	: 4;	//err...different degrees of visibility, e.g node, layer, etc. Non-zero => hidden
				};
				U32 value;
			};

			Flags()
			{
				hidden = 0x0;
			}
		};


		C_SceneGraphNode(C_Scene* pRenderScene, const char* pName, S32 preferredId=-1);
		virtual ~C_SceneGraphNode();

		void SetScene(C_Scene *pScene);
		C_Scene* GetScene();
		const C_Scene* GetScene() const;

		void SetName(const char* pName);
		const Util::C_String& GetName() const;

		void SetId(S32 id);
		S32 GetId() const;

		C_SceneGraphNode* GetParentPtr();
		const C_SceneGraphNode* GetParentPtr() const;

		C_SceneGraphNode* GetPreviousSiblingPtr();
		const C_SceneGraphNode* GetPreviousSiblingPtr() const;

		C_SceneGraphNode* GetNextSiblingPtr();
		const C_SceneGraphNode* GetNextSiblingPtr() const;

		U32 GetNumChildren() const;
		C_SceneGraphNode* GetChildPtr(U32 index=0);
		const C_SceneGraphNode* GetChildPtr(U32 index=0) const;

		void AddSibling(C_SceneGraphNode *pSibling);
		void AddChild(C_SceneGraphNode *pChild);

		void Detach();
		void DetachChild(C_SceneGraphNode *pChild);

		void SetLocalMatrix(const Maths::HMat44& mat);

		const Maths::HMat44& GetLocalMatrix() const;

		void SetWorldMatrix(const Maths::HMat44& mat);
		const Maths::HMat44& GetWorldMatrix() const;

		void SetWorldAABBox(const Maths::C_AxisBox& box);
		const Maths::C_AxisBox& GetWorldAABBox() const;

		virtual void UpdateCache() const;

		virtual E_SceneGraphNodeType GetType() const
		{
			return E_SceneGraphNode_Base;
		}

		virtual bool IsA(E_SceneGraphNodeType type) const
		{
			return (type == E_SceneGraphNode_Base);
		}

		void SetCustomTypeData(S32 data)
		{
			m_CustomTypeData = data;
		}

		S32 GetCustomTypeData() const
		{
			return m_CustomTypeData;
		}

		const Flags& GetFlags() const
		{
			return m_Flags;
		}

		void SetFlags(const Flags& flags)
		{
			m_Flags = flags;
			m_pScene->InvalidateCache();
		}

		virtual void OnRenderContextCreation(C_RenderContext* pRrc);
		virtual void OnRenderContextDestruction(C_RenderContext* pRc);

		virtual void AttachToRenderer(U8 lod=0, E_RendererPass pass=E_RendererPass_Normal, bool recurse=true);

	protected:

		Util::C_String		m_Name;
		S32					m_Id;

		S32					m_CustomTypeData;
		Flags				m_Flags;

		Maths::HMat44		m_LocalMatrix;

		mutable Maths::HMat44		m_WorldMatrix;
		mutable Maths::C_AxisBox	m_WorldAABBox;

		C_Scene				*m_pScene;
		C_SceneGraphNode	*m_pParent;
		C_SceneGraphNode	*m_pChild;
		C_SceneGraphNode	*m_pPrevious;
		C_SceneGraphNode	*m_pNext;

		friend class C_Scene;
	};

	#include "SceneGraphNode.inl"
}

#endif