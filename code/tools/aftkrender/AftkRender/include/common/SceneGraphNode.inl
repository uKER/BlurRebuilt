
inline C_SceneGraphNode::C_SceneGraphNode(C_Scene *pRenderScene, const char* pName, S32 preferredId)
: m_pScene(pRenderScene)
, m_pParent(0)
, m_pPrevious(0)
, m_pNext(0)
, m_pChild(0)
, m_Name(pName)
, m_CustomTypeData(0)
{
	if (pRenderScene)
	{
		m_Id = m_pScene->OnCreateSceneGraphNode(this, preferredId);
		if (preferredId!=-1 && m_Id!=preferredId)
			LOGWARN(g_Log, "Reassigning scenegraph node %s id(preferred %d, actual %d)", pName, preferredId, m_Id);
	}
	else
		m_Id = -1;
}

inline C_SceneGraphNode::~C_SceneGraphNode()
{
	m_pScene->OnDestroySceneGraphNode(this);
}

inline void C_SceneGraphNode::SetScene(C_Scene *pScene)
{
	if (!m_pScene)
		m_Id = pScene->OnCreateSceneGraphNode(this, -1);

	m_pScene = pScene;
}

inline C_Scene* C_SceneGraphNode::GetScene()
{
	return m_pScene;
}

inline const C_Scene* C_SceneGraphNode::GetScene() const
{
	return m_pScene;
}

inline void C_SceneGraphNode::SetName(const char* pName)
{
	m_Name = pName;
}

inline const Util::C_String&  C_SceneGraphNode::GetName() const
{
	return m_Name;
}

inline void C_SceneGraphNode::SetId(S32 id)
{
	m_Id = id;
}

inline S32 C_SceneGraphNode::GetId() const
{
	return m_Id;
}

inline C_SceneGraphNode* C_SceneGraphNode::GetParentPtr()
{
	return m_pParent;
}

inline const C_SceneGraphNode* C_SceneGraphNode::GetParentPtr() const
{
	return m_pParent;
}

inline C_SceneGraphNode* C_SceneGraphNode::GetPreviousSiblingPtr()
{
	return m_pPrevious;
}

inline const C_SceneGraphNode* C_SceneGraphNode::GetPreviousSiblingPtr() const
{
	return m_pPrevious;
}

inline C_SceneGraphNode* C_SceneGraphNode::GetNextSiblingPtr()
{
	return m_pNext;
}

inline const C_SceneGraphNode* C_SceneGraphNode::GetNextSiblingPtr() const
{
	return m_pNext;
}

inline U32 C_SceneGraphNode::GetNumChildren() const
{
	U32 count = 0;
	const C_SceneGraphNode* pChild = GetChildPtr(0);
	while (pChild)
	{
		pChild = pChild->m_pNext;
		++count;
	}
	return count;
}

inline C_SceneGraphNode* C_SceneGraphNode::GetChildPtr(U32 index)
{
	C_SceneGraphNode* pChild = m_pChild;
	while (index && pChild)
	{
		pChild = pChild->m_pNext;
		--index;
	}
	return pChild;
}

inline const C_SceneGraphNode* C_SceneGraphNode::GetChildPtr(U32 index) const
{
	C_SceneGraphNode* pChild = m_pChild;
	while (index && pChild)
	{
		pChild = pChild->GetNextSiblingPtr();
		--index;
	}
	return pChild;
}

inline void C_SceneGraphNode::AddSibling(C_SceneGraphNode* pSibling)
{
//	assert(pSibling->m_pParent==0);
//	assert(m_pScene->GetRootSceneGraphNodePtr() != this);
//	assert(pSibling != this);

	C_SceneGraphNode *pCurrentSibling = this;
	while (pCurrentSibling->m_pNext)
		pCurrentSibling = pCurrentSibling->m_pNext;

	pCurrentSibling->m_pNext = pSibling;
	pSibling->m_pPrevious = pCurrentSibling;

	pCurrentSibling = pSibling;
	while (pCurrentSibling)
	{
		pCurrentSibling->m_pParent = m_pParent;
		pCurrentSibling = pCurrentSibling->m_pNext;
	}
}

inline void C_SceneGraphNode::AddChild(C_SceneGraphNode* pChild)
{
	assert(pChild->m_pParent==0);
	assert(pChild->m_pPrevious==0);

	if (m_pChild)
		m_pChild->AddSibling(pChild);
	else
	{
		pChild->m_pParent = this;
		m_pChild = pChild;

		C_SceneGraphNode *pCurrentChild = m_pChild->m_pNext;
		while (pCurrentChild)
		{
			pCurrentChild->m_pParent = m_pParent;
			pCurrentChild = pCurrentChild->m_pNext;
		}
	}
}

inline void C_SceneGraphNode::Detach()
{
	if (m_pPrevious)
		m_pPrevious->m_pNext = m_pNext;

	if (m_pNext)
		m_pNext->m_pPrevious = m_pPrevious;
	
	if (m_pParent->m_pChild == this)
		m_pParent->m_pChild = m_pNext;

	m_pPrevious = 0;
	m_pNext = 0;
	m_pParent = 0;
}

inline void C_SceneGraphNode::DetachChild(C_SceneGraphNode *pChild)
{
	assert(pChild != this);

	C_SceneGraphNode *pCurrentChild = m_pChild;

	//find the child node
	while (pCurrentChild && pChild!=pCurrentChild)
		pCurrentChild = pCurrentChild->m_pNext;

	if (!pCurrentChild)
		return;

	if (pCurrentChild->m_pPrevious)
		pCurrentChild->m_pPrevious->m_pNext = pCurrentChild->m_pNext;

	if (pCurrentChild->m_pNext)
		pCurrentChild->m_pNext->m_pPrevious = pCurrentChild->m_pPrevious;

	if (m_pChild == pCurrentChild)
	{
		assert(pCurrentChild->m_pPrevious == 0);
		m_pChild = pCurrentChild->m_pNext;
	}

	pCurrentChild->m_pNext = 0;
	pCurrentChild->m_pPrevious = 0;
	pCurrentChild->m_pParent = 0;

}

inline void C_SceneGraphNode::SetLocalMatrix(const Maths::HMat44& mat)
{
	m_LocalMatrix = mat;
	UpdateCache();
}

inline const Maths::HMat44& C_SceneGraphNode::GetLocalMatrix() const
{
	return m_LocalMatrix;
}

inline void C_SceneGraphNode::SetWorldMatrix(const Maths::HMat44& mat)
{
	if (m_pParent)
		m_LocalMatrix = mat * m_pParent->GetWorldMatrix().Inverse();
	else
		m_LocalMatrix = mat;

	m_WorldMatrix = mat;
	UpdateCache();
}

inline const Maths::HMat44& C_SceneGraphNode::GetWorldMatrix() const
{
	return m_WorldMatrix;
}

inline void C_SceneGraphNode::SetWorldAABBox(const Maths::C_AxisBox& box)
{
	m_WorldAABBox = box;
}

inline const Maths::C_AxisBox& C_SceneGraphNode::GetWorldAABBox() const
{
	return m_WorldAABBox;
}

inline void C_SceneGraphNode::UpdateCache() const
{
	if (m_pParent)
		m_WorldMatrix = m_LocalMatrix * m_pParent->GetWorldMatrix();
	else
		m_WorldMatrix = m_LocalMatrix;

	C_SceneGraphNode *pCurrentChild = m_pChild;
	while (pCurrentChild)
	{
		pCurrentChild->UpdateCache();
		pCurrentChild = pCurrentChild->m_pNext;
	}
	m_pScene->InvalidateCache();
}

inline void C_SceneGraphNode::OnRenderContextCreation(C_RenderContext* pRc)
{
	C_SceneGraphNode *pChild = m_pChild;
	while (pChild)
	{
		pChild->OnRenderContextCreation(pRc);
		pChild = pChild->m_pNext;
	}
}

inline void C_SceneGraphNode::OnRenderContextDestruction(C_RenderContext* pRc)
{
	C_SceneGraphNode *pChild = m_pChild;
	while (pChild)
	{
		pChild->OnRenderContextDestruction(pRc);
		pChild = pChild->m_pNext;
	}
}

inline void C_SceneGraphNode::AttachToRenderer(U8 lod, E_RendererPass pass, bool recurse)
{
	if (recurse)
	{
		C_SceneGraphNode *pChild = m_pChild;
		while (pChild)
		{
			pChild->AttachToRenderer(lod, pass);
			pChild = pChild->m_pNext;
		}
	}
}

