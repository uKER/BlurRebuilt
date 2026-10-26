#include <stdafx.h>
#include <Aftk/AftkEuler.h>
#include "AftkInterpolation.h"

//#pragma optimize("", off)

using namespace Aftk;

#define AFTK_SCENE_NODE_STATIC_PLUG_TRANSLATE 0
#define AFTK_SCENE_NODE_STATIC_PLUG_ROTATE 1
#define AFTK_SCENE_NODE_STATIC_PLUG_SCALE 2
#define AFTK_SCENE_NODE_STATIC_PLUG_SHEAR 3
#define AFTK_SCENE_NODE_NUM_STATIC_TRANSFORM_PLUGS 4
#define AFTK_SCENE_NODE_STATIC_PLUG_WORLD_MATRIX 4
#define AFTK_SCENE_NODE_STATIC_PLUG_VISIBILITY 5
#define AFTK_SCENE_NODE_NUM_STATIC_PLUGS 6

C_SceneNode::C_SceneNode(C_Scene* pScene)
:
C_Node(pScene),
m_translatePlug(OnTransformPlugChanged),
m_rotatePlug(OnTransformPlugChanged),
m_scalePlug(OnTransformPlugChanged),
m_shearPlug(OnTransformPlugChanged),
m_worldMatrixProxyPlug(E_GenericAttributeType_Matrix44F32, OnWorldMatrixProxyQuery),
m_visibilityPlug(E_GenericAttributeType_Boolean, 0)
{
	OneTimeInit(pScene);
}

C_SceneNode::C_SceneNode(const C_SceneNode& rhs)
:
C_Node(0),
m_translatePlug(OnTransformPlugChanged),
m_rotatePlug(OnTransformPlugChanged),
m_scalePlug(OnTransformPlugChanged),
m_shearPlug(OnTransformPlugChanged),
m_worldMatrixProxyPlug(E_GenericAttributeType_Matrix44F32, OnWorldMatrixProxyQuery),
m_visibilityPlug(E_GenericAttributeType_Boolean, 0)
{
	OneTimeInit(0);
	*this = rhs;
}

C_Node* C_SceneNode::Clone(C_Scene* pScene) const
{
	C_SceneNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneNode(pScene);
		(C_SceneNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_SceneNode::~C_SceneNode()
{
	//Remove this node from its lod group, if any.
	if (m_pParent)
	{
		if (m_pParent->GetType() == E_ObjType_SceneLodGroup)
		{
			C_SceneLodGroupNode* pLodGroup = (C_SceneLodGroupNode*)m_pParent;
			pLodGroup->DeleteLodLevelInfo(this);
		}
	}

	//Make a backup of our instance set.
	Ut::UtVector<Ut::DynamicVector<C_SceneNode*> > instancesOfThisNode = m_instancesOfThisNode;
	Ut::UtVector<Ut::DynamicVector<C_SceneNode*> >::Iterator it;

	for ( it = instancesOfThisNode.Begin() ; it != instancesOfThisNode.End(); it++)
	{
		C_SceneNode* pSceneNode = *it;
		pSceneNode->DeleteInstanceSource(this);
	}

	//There should be no more instances of this node remaining.
	AFTK_ASSERT(m_instancesOfThisNode.Size() == 0);

	ClearThis();

	m_pScene->OnDestroySceneNode(this);
}

E_ObjType C_SceneNode::GetType() const
{
	return E_ObjType_SceneNode;
}

bool C_SceneNode::HasType(E_ObjType type) const
{
	if (C_Node::HasType(type))
		return true;
	if (type == C_SceneNode::GetType()) 
		return true;

	return false;
}

C_InfluenceClusterNode* C_SceneNode::GetInfluenceClusterPtr(U32 instance)
{
	return m_influenceClusters[instance];
}

const C_InfluenceClusterNode*	C_SceneNode::GetInfluenceClusterPtr(U32 instance) const
{
	return m_influenceClusters[instance];
}

U32 C_SceneNode::GetNumInfluenceClusters() const
{
	return m_influenceClusters.Size();
}

void C_SceneNode::AddChildPtr(C_SceneNode* pNode)
{
	if (pNode->m_pParent != 0)
	{
		AFTK_ASSERT(pNode->m_pParent != this);
	}
	AFTK_ASSERT(pNode != this);

	m_pScene->ExtractSceneNodeAndChildren(pNode);

	if (m_pChild == 0)
	{
		SetChildPtr(pNode);
	}
	else
	{
		m_pChild->AddNextPtr(pNode);
	}
	AFTK_ASSERT(pNode->GetParentPtr() == this);
	
	//Dirty matrix cache.
	pNode->DirtyAllTransformDependantCaches();
	//Dirty siblings.
	while (pNode->m_pNext)
	{
		pNode->DirtyAllTransformDependantCaches();
		pNode = pNode->m_pNext;
	}		
}

void C_SceneNode::AddChildPtrAfter(C_SceneNode* afterThis, C_SceneNode* pChild)
{
	m_pScene->ExtractSceneNodeAndChildren(pChild);
	afterThis->InsertNextPtr(pChild);
}

void C_SceneNode::AddChildPtrAfter(U32 afterThisChild, C_SceneNode* pChild)
{
	C_SceneNode* pAfterThisChild = GetChildPtr(afterThisChild);
	AFTK_ASSERT(pChild);
	pChild->AddChildPtrAfter(pAfterThisChild, pChild);
}

void C_SceneNode::AddChildPtrBefore(C_SceneNode* beforeThis, C_SceneNode* pChild)
{
	m_pScene->ExtractSceneNodeAndChildren(pChild);
	beforeThis->InsertPreviousPtr(pChild);
}

void C_SceneNode::AddChildPtrBefore(U32 beforeThisChild, C_SceneNode* pChild)
{
	C_SceneNode* pBeforeThisChild = GetChildPtr(beforeThisChild);
	AFTK_ASSERT(pChild);
	pChild->AddChildPtrAfter(pBeforeThisChild, pChild);
}

void C_SceneNode::InsertNextPtr(C_SceneNode* pNode)
{
	if (!pNode) 
		return;

	if (pNode->m_pParent)
	{
		//Dirty parent's nChildren cache.
		pNode->m_pParent->m_nChildrenCache = -1;
	}
	if (m_pParent)
	{
		//Dirty parent's nChildren cache.
		m_pParent->m_nChildrenCache = -1;
	}


	C_SceneNode* pOldNext = m_pNext;
	m_pNext = pNode;
	AFTK_ASSERT(m_pNext != this);
	if (pOldNext != 0)
	{
		pOldNext->m_pPrevious = pNode;
	}

	//Find the end element of the siblings to be added
	pNode->m_pNext = pOldNext;
	pNode->m_pPrevious = this;
	pNode->m_pParent = m_pParent;
	pNode->RecalculateStaticMatrixCache();
}

void C_SceneNode::InsertPreviousPtr(C_SceneNode* pNode)
{
	if (!pNode) 
		return;

	if (pNode->m_pParent)
	{
		//Dirty parent's nChildren cache.
		pNode->m_pParent->m_nChildrenCache = -1;
	}
	if (m_pParent)
	{
		//Dirty parent's nChildren cache.
		m_pParent->m_nChildrenCache = -1;
	}

	C_SceneNode* pOldPrev = m_pPrevious;
	m_pPrevious = pNode;
	if (pOldPrev != 0)
	{
		pOldPrev->m_pNext = pNode;
	}

	//Find the first element of the siblings to be added
	pNode->m_pPrevious = pOldPrev;
	pNode->m_pNext = this;
	pNode->m_pParent = m_pParent;
	if (!pOldPrev)
	{
		//We are the first child now
		m_pParent->m_pChild = pNode;
	}

	pNode->RecalculateStaticMatrixCache();
}

void C_SceneNode::DeleteChildPtr(C_SceneNode* pNode)
{
	//Dirty our nChildren cache.
	m_nChildrenCache = -1;

	AFTK_ASSERT(pNode != this);
	//Find an end location to insert into sibling list
	C_SceneNode* pChildToDelete = m_pChild;

	//Loop until null, or we find our node.
	while ((pNode != pChildToDelete) && (pChildToDelete))
	{
		pChildToDelete = pChildToDelete->m_pNext;
	}
	if (!pChildToDelete)
	{
		AFTK_ASSERT(!pChildToDelete);
		return;
	}
	
	//If we are the first child...
	if (m_pChild == pChildToDelete)
	{
		//Adjust previous sibling
		//First child cannot have previous siblings.
		AFTK_ASSERT(!pChildToDelete->m_pPrevious);

		//Adjust next sibling
		if (pChildToDelete->m_pNext)
		{
			pChildToDelete->m_pNext->m_pPrevious = pChildToDelete->m_pPrevious;
		}

		//Provide us with the new first child pointer.
		m_pChild = pChildToDelete->m_pNext;

		//Adjust next sibling
		if (pChildToDelete->m_pNext)
		{
			pChildToDelete->m_pNext->m_pPrevious = 0;
		}
	}
	//Else we are not the first child...
	else
	{
		if (pChildToDelete->m_pPrevious)
		{
			pChildToDelete->m_pPrevious->m_pNext = pChildToDelete->m_pNext;
		}
		//Adjust next sibling
		if (pChildToDelete->m_pNext)
		{
			pChildToDelete->m_pNext->m_pPrevious = pChildToDelete->m_pPrevious;
		}
	}

	pChildToDelete->m_pNext = 0;
	pChildToDelete->m_pPrevious = 0;
	pChildToDelete->m_pParent = 0;
}

void C_SceneNode::SetChildPtr(C_SceneNode* pNode)
{
	//Dirty our nChildren cache.
	m_nChildrenCache = -1;

	AFTK_ASSERT(pNode != this);
	m_pChild = pNode;

	if (m_pChild)
	{
		// Set parental pointers for all children of this node
		C_SceneNode* pCurrentChild = pNode;
		while (pCurrentChild)
		{
			pCurrentChild->m_pParent = this;
			pCurrentChild = pCurrentChild->m_pNext;
		}
	}

	if (pNode->m_pNext)
	{
		AFTK_ASSERT(pNode->m_pNext != pNode->m_pPrevious);
	}
	
	//Dirty matrix cache.
	pNode->DirtyAllTransformDependantCaches();
	//Dirty siblings.
	while (pNode->m_pNext)
	{
		pNode->DirtyAllTransformDependantCaches();
		pNode = pNode->m_pNext;
	}	
}

void C_SceneNode::SetLocalMatrix(const Matrix44& matrix, bool compensateChildren)
{
	//We need to be able to invert our matrices.
	if (UtMaths::GetDeterminant4x4(matrix) == 0.f)
	{
		assert(!"Non invertable matrix used in SetLocalMatrix!");
		return;
	}

	Aftk::Matrix44 oldLocalMatrix;
	if (compensateChildren)
		oldLocalMatrix = GetLocalMatrix(false);

	Maths::HVec3 scale;
	Maths::HVec3 shear;
	Maths::HVec3 rotate;
	Maths::HVec3 translate;

	Maths::HMat43 temp;
	TemplateToNightmareMatrix43(temp, matrix);

	//GetSRT grabs rotation values in roll, pitch, heading order. This is no good to us as legacy
	//animation curves are in XYZ order, and set matrix will be used on the nodes driven by these curves.
	//Therefore we must grab the rotation values in XYZ order from the given matrix.
	GetSShRT(
		scale,
		shear,
		rotate,
		translate, 
		temp, 
		m_rotationOrder);

	//Reset pivots and offsets
	memset(&m_scalePivot, 0, sizeof(Vector3F32));
	memset(&m_scalePivotTranslation, 0, sizeof(Vector3F32));
	memset(&m_rotatePivot, 0, sizeof(Vector3F32));
	memset(&m_rotatePivotTranslation, 0, sizeof(Vector3F32));
	UtMaths::SetIdentity(&m_rotateOrientation);

	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(0))->SetValue(translate.X());
	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(1))->SetValue(translate.Y());
	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(2))->SetValue(translate.Z());

	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(0))->SetValue(rotate.X());
	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(1))->SetValue(rotate.Y());
	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(2))->SetValue(rotate.Z());

	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(0))->SetValue(scale.X());
	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(1))->SetValue(scale.Y());
	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(2))->SetValue(scale.Z());

	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(0))->SetValue(shear.X());
	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(1))->SetValue(shear.Y());
	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(2))->SetValue(shear.Z());

	DirtyAllTransformDependantCaches();

	if (compensateChildren)
	{
		Aftk::Matrix44 newLocalMatrixInverse;
		Invert4x4(&newLocalMatrixInverse, matrix);

		Aftk::C_SceneNode* pChild = m_pChild;
		while (pChild)
		{
			pChild->SetLocalMatrix(pChild->GetLocalMatrix() * oldLocalMatrix * newLocalMatrixInverse);
			//Let's iterate...
			pChild = pChild->m_pNext;
		}
	}
}

void C_SceneNode::SetWorldMatrix(const Matrix44& worldMatrix, bool compensateChildren)
{
	//We need to be able to invert our matrices.
	if (UtMaths::GetDeterminant4x4(worldMatrix) == 0.f)
	{
		assert(!"Non invertable matrix used in SetWorldMatrix!");
		return;
	}

	Aftk::Matrix44 invExclusiveWorldMatrix;
	if (!m_pParent)
		UtMaths::SetIdentity(&invExclusiveWorldMatrix);
	else
		invExclusiveWorldMatrix = m_pParent->GetWorldMatrixInverse(false);

	SetLocalMatrix(worldMatrix * invExclusiveWorldMatrix, compensateChildren);
}

void C_SceneNode::SetBasePose(const Matrix44 &matrix)
{
	m_basePose = matrix;
#ifndef AFTK_LEAN_AND_MEAN
	UtMaths::Invert4x4(&m_basePoseInverse, m_basePose);
#endif
}

C_SceneNode* C_SceneNode::GetParentPtr()
{
	return m_pParent;
}

C_SceneNode* C_SceneNode::GetChildPtr(U32 instance)
{
	if (m_pChild != 0)
	{
		C_SceneNode* pCurrentChild = m_pChild;
		for (U32 i = 0; i < instance; i++)
		{
			pCurrentChild = pCurrentChild->m_pNext;
			if (pCurrentChild == 0)
			{
				return 0;
			}
		}
		return pCurrentChild;
	}
	return 0;
}

const C_SceneNode* C_SceneNode::GetChildPtr(U32 instance) const
{
	if (m_pChild != 0)
	{
		C_SceneNode* pCurrentChild = m_pChild;
		for (U32 i = 0; i < instance; i++)
		{
			pCurrentChild = pCurrentChild->m_pNext;
			if (pCurrentChild == 0)
			{
				return 0;
			}
		}
		return pCurrentChild;
	}
	return 0;
}

C_SceneNode* C_SceneNode::GetNextPtr()
{
	return m_pNext;
}

C_SceneNode* C_SceneNode::GetPreviousPtr()
{
	return m_pPrevious;
}

const C_SceneNode* C_SceneNode::GetParentPtr() const
{
	return m_pParent;
}

const C_SceneNode* C_SceneNode::GetNextPtr() const
{
	return m_pNext;
}

const C_SceneNode* C_SceneNode::GetPreviousPtr() const
{
	return m_pPrevious;
}

const Matrix44& C_SceneNode::EvaluateLocalMatrix() const
{
	//If we don't need re-evaluation...
	if (!m_dynamicMatrixCacheDirtyFlag)
		return m_dynamicLocalMatrixCache;

	//Refresh cache.
	RecalculateDynamicMatrixCache();
	return m_dynamicLocalMatrixCache;
}

Matrix44 C_SceneNode::EvaluateLocalMatrixInverse() const
{
	//Refresh cache.
	if (m_dynamicMatrixCacheDirtyFlag)
		RecalculateDynamicMatrixCache();

#ifndef AFTK_LEAN_AND_MEAN
	return m_dynamicLocalMatrixInverseCache;
#else
	Matrix44 tmp;
	UtMaths::Invert4x4(&tmp, m_dynamicLocalMatrixCache);
	return tmp;
#endif
}

Matrix44 C_SceneNode::EvaluateLocalMatrixInverseTranspose() const
{
	//Refresh cache.
	if (m_dynamicMatrixCacheDirtyFlag)
		RecalculateDynamicMatrixCache();

#ifndef AFTK_LEAN_AND_MEAN
	return m_dynamicLocalMatrixInverseTransposeCache;
#else
	//Calculate inverse transpose
	Matrix44 m_dynamicLocalMatrixInverseTranspose;
	Matrix44 matrixTranspose = m_dynamicLocalMatrixCache;
	UtMaths::Transpose(&matrixTranspose);
	UtMaths::Invert4x4(&m_dynamicLocalMatrixInverseTranspose, matrixTranspose);
	return m_dynamicLocalMatrixInverseTranspose;
#endif
}

const Matrix44& C_SceneNode::EvaluateWorldMatrix() const
{
	//If we don't need re-evaluation...
	if (!m_dynamicMatrixCacheDirtyFlag)
		return m_dynamicWorldMatrixCache;

	//Refresh cache.
	RecalculateDynamicMatrixCache();
	return m_dynamicWorldMatrixCache;
}

Matrix44 C_SceneNode::EvaluateWorldMatrixInverse() const
{
	if (m_dynamicMatrixCacheDirtyFlag)
		RecalculateDynamicMatrixCache();

#ifndef AFTK_LEAN_AND_MEAN
	return m_dynamicWorldMatrixInverseCache;
#else
	Matrix44 tmp;
	UtMaths::Invert4x4(&tmp, m_dynamicWorldMatrixCache);
	return tmp;
#endif
}

Matrix44 C_SceneNode::EvaluateWorldMatrixInverseTranspose() const
{
	//Refresh cache.
	if (m_dynamicMatrixCacheDirtyFlag)
		RecalculateDynamicMatrixCache();

#ifndef AFTK_LEAN_AND_MEAN
	return m_dynamicWorldMatrixInverseTransposeCache;
#else
	//Calculate inverse transpose
	Matrix44 m_dynamicWorldMatrixInverseTranspose;
	Matrix44 matrixTranspose = m_dynamicWorldMatrixCache;
	UtMaths::Transpose(&matrixTranspose);
	UtMaths::Invert4x4(&m_dynamicWorldMatrixInverseTranspose, matrixTranspose);
	return m_dynamicWorldMatrixInverseTranspose;
#endif
}

const C_SharedResource<C_BlindDataNode>* C_SceneNode::GetBlindDataResourcePtr() const
{
	return m_pBlindDataResource;
}

S32 C_SceneNode::GetUniqueId() const
{
	return m_uniqueId;
}

void C_SceneNode::SetBlindDataResourcePtr(const C_BlindDataResource* pBlindDataResource)
{
	if (pBlindDataResource == m_pBlindDataResource)
		return;

	if (m_pBlindDataResource)
		//Decrease old node reference count
		m_pScene->DecrementResourceReferenceCount(m_pBlindDataResource);

	m_pBlindDataResource = pBlindDataResource;

	if (m_pBlindDataResource)
		//Start using new blind data node - increase reference count
		m_pScene->IncrementResourceReferenceCount(m_pBlindDataResource);
}

void C_SceneNode::DeleteBlindDataResourcePtr()
{
	if (m_pBlindDataResource != 0)
	{
		//Decrease old node reference count
		m_pScene->DecrementResourceReferenceCount(m_pBlindDataResource);
		m_pBlindDataResource = 0;
	}
}

C_LayerNode* C_SceneNode::GetLayerPtr(U32 layer)
{
	if (layer == 0 && m_layers.Size() == 0)
		return NULL;

	return m_layers[layer];
}

const C_LayerNode* C_SceneNode::GetLayerPtr(U32 layer) const
{
	if (layer == 0 && m_layers.Size() == 0)
		return NULL;

	return m_layers[layer];
}

bool C_SceneNode::IsTransformAnimated() const
{
	if (IsTransformDirectlyAnimated() || IsTransformIndirectlyAnimated())
		return true;

	return false;
}

bool C_SceneNode::IsTransformDirectlyAnimated() const
{
	//If any of our static transform plugs are animated, return true.
	for (U32 iPlug = 0; iPlug < AFTK_SCENE_NODE_NUM_STATIC_TRANSFORM_PLUGS; iPlug++)
	{
		const I_Plug* pPlug = GetStaticPlugPtr(iPlug);
		const I_Object* pSourceConnection = pPlug->GetSourceConnectionPtr();

		if (pSourceConnection)
			return true;
		
		for (U32 jPlugChild = 0; jPlugChild < pPlug->GetNumChildren(); jPlugChild++)
		{
			pSourceConnection = pPlug->GetChildPtr(jPlugChild)->GetSourceConnectionPtr();
			if (pSourceConnection)
				return true;
		}
	}

	return false;
}

bool C_SceneNode::IsTransformIndirectlyAnimated() const
{
	if (m_pParent)
	{
		if (m_pParent->IsTransformDirectlyAnimated())
			return true;

		if (m_pParent->IsTransformIndirectlyAnimated())
			return true;
	}

	//Base-case - the root node cannot be indirectly animated.
	return false;
}

bool C_SceneNode::IsUsed() const
{
	if (C_Node::IsUsed() || (m_instancesOfThisNode.Size() > 0) || m_pChild || m_pParent || m_pScene->m_pRootSceneNode == this)
		return true;

	return false;
}

C_Bounds C_SceneNode::GetEvaluatedLocalBounds() const
{
#ifndef AFTK_LEAN_AND_MEAN
	if (m_localBoundsCacheDirtyFlag)
		RecalculateLocalBoundsCache();

	return m_localBoundsCache;
#else
	C_Bounds bounds;
	//Calculate local bounds.
	CalculateBounds(bounds, 0);
	return bounds;
#endif
}

C_Bounds C_SceneNode::GetEvaluatedWorldBounds(bool evaluateTransform) const
{
#ifndef AFTK_LEAN_AND_MEAN
	if (m_worldBoundsCacheDirtyFlag)
		RecalculateWorldBoundsCache();

	if (evaluateTransform)
		return m_dynamicWorldEvaluatedBoundsCache;

	return m_staticWorldEvaluatedBoundsCache;
#else
	C_Bounds bounds;
	if (evaluateTransform)
		CalculateBounds(bounds, &GetWorldMatrix(true));
	else
		CalculateBounds(bounds, &GetWorldMatrix(false));
	return bounds;
#endif
}

C_Bounds C_SceneNode::GetLocalBounds() const
{
#ifndef AFTK_LEAN_AND_MEAN
	if (m_localBoundsCacheDirtyFlag)
		RecalculateLocalBoundsCache();

	return m_localBoundsCache;
#else
	C_Bounds bounds;
	//Calculate local bounds.
	CalculateBounds(bounds, 0);
	return bounds;
#endif
}

C_Bounds C_SceneNode::GetWorldBounds(bool evaluateTransform) const
{
#ifndef AFTK_LEAN_AND_MEAN
	if (m_worldBoundsCacheDirtyFlag)
		RecalculateWorldBoundsCache();

	if (evaluateTransform)
		return m_dynamicWorldBoundsCache;

	return m_staticWorldBoundsCache;
#else
	C_Bounds bounds;
	if (evaluateTransform)
		CalculateBounds(bounds, &GetWorldMatrix(true));
	else
		CalculateBounds(bounds, &GetWorldMatrix(false));
	return bounds;
#endif
}

const C_NullPlug* C_SceneNode::GetTransformPlugPtr(E_AnimAttribute attr) const
{
	switch(attr)
	{
	case E_AnimAttribute_TranslateX:
		return (C_NullPlug*)m_translatePlug.GetChildPtr(0);
	case E_AnimAttribute_TranslateY:
		return (C_NullPlug*)m_translatePlug.GetChildPtr(1);
	case E_AnimAttribute_TranslateZ:
		return (C_NullPlug*)m_translatePlug.GetChildPtr(2);

	case E_AnimAttribute_ScaleX:
		return (C_NullPlug*)m_scalePlug.GetChildPtr(0);
	case E_AnimAttribute_ScaleY:
		return (C_NullPlug*)m_scalePlug.GetChildPtr(1);
	case E_AnimAttribute_ScaleZ:
		return (C_NullPlug*)m_scalePlug.GetChildPtr(2);

	case E_AnimAttribute_RotateX:
		return (C_NullPlug*)m_rotatePlug.GetChildPtr(0);
	case E_AnimAttribute_RotateY:
		return (C_NullPlug*)m_rotatePlug.GetChildPtr(1);
	case E_AnimAttribute_RotateZ:
		return (C_NullPlug*)m_rotatePlug.GetChildPtr(2);

	case E_AnimAttribute_ShearX:
		return (C_NullPlug*)m_shearPlug.GetChildPtr(0);
	case E_AnimAttribute_ShearY:
		return (C_NullPlug*)m_shearPlug.GetChildPtr(1);
	case E_AnimAttribute_ShearZ:
		return (C_NullPlug*)m_shearPlug.GetChildPtr(2);
	};

	return 0;
}

C_NullPlug* C_SceneNode::GetTransformPlugPtr(E_AnimAttribute attr)
{
	return const_cast<C_NullPlug*>(((const C_SceneNode*)this)->GetTransformPlugPtr(attr));
}

C_NullPlug& C_SceneNode::GetTranslatePlug()
{
	return m_translatePlug;
}

C_NullPlug& C_SceneNode::GetRotatePlug()
{
	return m_rotatePlug;
}

C_NullPlug& C_SceneNode::GetScalePlug()
{
	return m_scalePlug;
}

C_NullPlug& C_SceneNode::GetShearPlug()
{
	return m_shearPlug;
}

const C_NullPlug& C_SceneNode::GetTranslatePlug() const
{
	return m_translatePlug;
}

const C_NullPlug& C_SceneNode::GetRotatePlug() const
{
	return m_rotatePlug;
}

const C_NullPlug& C_SceneNode::GetScalePlug() const
{
	return m_scalePlug;
}

const C_NullPlug& C_SceneNode::GetShearPlug() const
{
	return m_shearPlug;
}

const C_StaticPlug<bool>& C_SceneNode::GetVisibilityPlug() const
{
	return m_visibilityPlug;
}

C_StaticPlug<bool>& C_SceneNode::GetVisibilityPlug()
{
	return m_visibilityPlug;
}

bool C_SceneNode::GetTranslation(Vector3F32& translate, bool evaluate) const
{
	if (!((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(0))->GetValue(translate[0], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(1))->GetValue(translate[1], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(2))->GetValue(translate[2], evaluate))
		return false;

	return true;
}

bool C_SceneNode::GetRotation(Vector3F32& rotateEuler, bool evaluate) const
{
	AFTK_ASSERT(m_pScene);
	if (m_rotationInterpolation == E_RotationInterpolation_Quaternion && evaluate)
	{
		if (DoWeSlerpThisNode(
			this,
			m_rotatePlug.GetChildPtr(0)->GetSourceConnectionPtr(),
			m_rotatePlug.GetChildPtr(1)->GetSourceConnectionPtr(),
			m_rotatePlug.GetChildPtr(2)->GetSourceConnectionPtr()))
		{
			Matrix44 rotateM;
			EvaluateRotation(rotateM);
			MatrixToEuler(rotateEuler, rotateM, m_rotationOrder);
			return true;
		}
	}

	if (!((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(0))->GetValue(rotateEuler[0], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(1))->GetValue(rotateEuler[1], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(2))->GetValue(rotateEuler[2], evaluate))
		return false;

	return true;
}

bool C_SceneNode::GetRotation(Matrix44& rotateM, bool evaluate) const
{
	AFTK_ASSERT(m_pScene);

	if (m_rotationInterpolation == E_RotationInterpolation_Quaternion && evaluate)
	{
		EvaluateRotation(rotateM);
		return true;
	}

	Vector3F32 rotateEuler;
	if (!((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(0))->GetValue(rotateEuler[0], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(1))->GetValue(rotateEuler[1], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(2))->GetValue(rotateEuler[2], evaluate))
		return false;

	Aftk::EulerToMatrix(rotateM, rotateEuler, m_rotationOrder);
	return true;
}

bool C_SceneNode::GetScale(Vector3F32& scale, bool evaluate) const
{
	if (!((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(0))->GetValue(scale[0], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(1))->GetValue(scale[1], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(2))->GetValue(scale[2], evaluate))
		return false;

	return true;
}

bool C_SceneNode::GetShear(Vector3F32& shear, bool evaluate) const
{
	if (!((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(0))->GetValue(shear[0], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(1))->GetValue(shear[1], evaluate))
		return false;
	if (!((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(2))->GetValue(shear[2], evaluate))
		return false;

	return true;
}

///\brief Adds a new dynamic plug to this node.
///
///@param newPlug The plug to add.
///@return The plug index.
S32	C_SceneNode::AddPlugPtr(C_DynamicPlug* pNewPlug)
{
	pNewPlug->SetLogicalIndex(GetNumPlugs());
	pNewPlug->SetParentPtr(this);
	m_dynamicPlugs.PushBack(pNewPlug);
	return pNewPlug->GetLogicalIndex();
}

const I_Plug* C_SceneNode::GetStaticPlugPtr(U32 instance) const
{
	switch(instance)
	{
		case AFTK_SCENE_NODE_STATIC_PLUG_TRANSLATE:
			return &m_translatePlug;
		case AFTK_SCENE_NODE_STATIC_PLUG_ROTATE:
			return &m_rotatePlug;
		case AFTK_SCENE_NODE_STATIC_PLUG_SCALE:
			return &m_scalePlug;	
		case AFTK_SCENE_NODE_STATIC_PLUG_SHEAR:
			return &m_shearPlug;
		case AFTK_SCENE_NODE_STATIC_PLUG_WORLD_MATRIX:
			return &m_worldMatrixProxyPlug;
		case AFTK_SCENE_NODE_STATIC_PLUG_VISIBILITY:
			return &m_visibilityPlug;
		default:
			break;
	};

	return 0;
}

U32	C_SceneNode::GetNumStaticPlugs() const
{
	return AFTK_SCENE_NODE_NUM_STATIC_PLUGS;
}

const C_DynamicPlug* C_SceneNode::GetDynamicPlugPtr(U32 instance) const
{
	return m_dynamicPlugs[instance];
}


C_DynamicPlug* C_SceneNode::GetDynamicPlugPtr(U32 instance)
{
	return m_dynamicPlugs[instance];
}

U32	C_SceneNode::GetNumDynamicPlugs() const
{
	return (U32)m_dynamicPlugs.Size();
}

bool C_SceneNode::DeletePlug(U32 plugIndex)
{
	U32 nStatic = GetNumStaticPlugs();
	if (plugIndex < nStatic)
		//We cannot delete static plugs.
		return false;

	U32 dynIndex = plugIndex - nStatic;
	if (dynIndex >= (U32)m_dynamicPlugs.Size())
	{
		//The specified dynamic plug does not exist.
		AFTK_ASSERT(false);
		return false;
	}

	//Free memory for plug.
	delete m_dynamicPlugs[dynIndex];
	//Remove plug pointer from array.
	m_dynamicPlugs.EraseByIndex(dynIndex);

	for (U32 i = dynIndex; i < (U32)m_dynamicPlugs.Size(); i++)
	{
		//Decrement logical index.
		m_dynamicPlugs[i]->SetLogicalIndex(
			m_dynamicPlugs[i]->GetLogicalIndex()-1);
	}

	return true;
}

const Matrix44& C_SceneNode::GetWorldMatrix(bool evaluate) const
{
	if (!evaluate)
	{
		if (m_staticMatrixCacheDirtyFlag)	
			RecalculateStaticMatrixCache();
		return m_staticWorldMatrixCache;
	}
	return EvaluateWorldMatrix();
}

Matrix44 C_SceneNode::GetWorldMatrixInverse(bool evaluate) const
{
	if (!evaluate)
	{
		if (m_staticMatrixCacheDirtyFlag)	
			RecalculateStaticMatrixCache();
	}

#ifndef AFTK_LEAN_AND_MEAN
	if (!evaluate)
	{
		return m_staticWorldMatrixInverseCache;
	}
	return EvaluateWorldMatrixInverse();
#else
	Matrix44 tmp;
	if (!evaluate)
		UtMaths::Invert4x4(&tmp, m_staticWorldMatrixCache);
	else
		UtMaths::Invert4x4(&tmp, m_dynamicWorldMatrixCache);
	return tmp;
#endif
}

Matrix44 C_SceneNode::GetWorldMatrixInverseTranspose(bool evaluate) const
{
	if (!evaluate)
	{
		if (m_staticMatrixCacheDirtyFlag)	
			RecalculateStaticMatrixCache();
	}

#ifndef AFTK_LEAN_AND_MEAN
	if (!evaluate)
	{
		return m_staticWorldMatrixInverseTransposeCache;
	}
	return EvaluateWorldMatrixInverseTranspose();
#else
	//Calculate inverse transpose
	Matrix44 dynamicWorldMatrixInverseTranspose;
	Matrix44 matrixTranspose;
	if (!evaluate)
		matrixTranspose = m_staticWorldMatrixCache;
	else
		matrixTranspose = m_dynamicWorldMatrixCache;

	UtMaths::Transpose(&matrixTranspose);
	UtMaths::Invert4x4(&dynamicWorldMatrixInverseTranspose, matrixTranspose);
	return dynamicWorldMatrixInverseTranspose;
#endif
}

void C_SceneNode::SetScalePivot(const Vector3F32& pivot)
{
	m_scalePivot = pivot;
}

void C_SceneNode::SetScalePivotTranslation(const Vector3F32& translate)
{
	m_scalePivotTranslation = translate;
}

void C_SceneNode::SetRotatePivot(const Vector3F32& pivot)
{
	m_rotatePivot = pivot;
}

void C_SceneNode::SetRotatePivotTranslation(const Vector3F32& translate)
{
	m_rotatePivotTranslation = translate;
}

void C_SceneNode::SetRotateOrientation(const Matrix44& m)
{
	m_rotateOrientation = m;
}

const Vector3F32& C_SceneNode::GetScalePivot() const
{
	return m_scalePivot;
}

const Vector3F32& C_SceneNode::GetScalePivotTranslation() const
{
	return m_scalePivotTranslation;
}

const Vector3F32& C_SceneNode::GetRotatePivot() const
{
	return m_rotatePivot;
}

const Vector3F32& C_SceneNode::GetRotatePivotTranslation() const
{
	return m_rotatePivotTranslation;
}

const Matrix44& C_SceneNode::GetRotateOrientation() const
{
	return m_rotateOrientation;
}

E_RotationInterpolation C_SceneNode::GetRotationInterpolation() const
{
	return m_rotationInterpolation;
}

void C_SceneNode::SetRotationInterpolation(E_RotationInterpolation interp)
{
	m_rotationInterpolation = interp;
	DirtyAllTransformDependantCaches();
}

const C_ProxyPlug<Matrix44>& C_SceneNode::GetWorldMatrixPlug() const
{
	return m_worldMatrixProxyPlug;
}

C_ProxyPlug<Matrix44>& C_SceneNode::GetWorldMatrixPlug()
{
	return m_worldMatrixProxyPlug;
}

Matrix44 C_SceneNode::OnWorldMatrixProxyQuery(const void* _pPlug, bool evaluate)
{
	const I_Plug* pPlug = (I_Plug*)_pPlug;
	const C_SceneNode* pNode;
	if (pPlug->IsPlugChild())
		pNode = (C_SceneNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		pNode = (C_SceneNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(pNode && pNode->HasType(E_ObjType_SceneNode));

	return pNode->GetWorldMatrix(evaluate);
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneNode::CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const
{
	bounds.Clear();
	Position3 p(0,0,0);
	if (pMatrix)
		p = ApplyMatrix(*pMatrix, p);
	bounds.AddVertex(p);
}

void C_SceneNode::AttachToLayer(C_LayerNode* pLayer)
{
	AFTK_ASSERT(pLayer);
	m_layers.PushBack(pLayer);
}

void C_SceneNode::DetachFromLayer(C_LayerNode* pLayer)
{
	for (U32 iLayer = 0; iLayer < (U32)m_layers.Size(); iLayer++)
	{
		if (m_layers[iLayer] == pLayer)
		{
			m_layers.EraseByIndex(iLayer);
			return;
		}
	}
}

U32 C_SceneNode::GetNumInstancesOfThisNode() const
{
	return (U32)m_instancesOfThisNode.Size();
}

const C_SceneNode* C_SceneNode::GetInstanceOfThisNode(U32 instance) const
{
	//For the love of God we need a += operator for the iterator here.
	//I should also look at exposing the UtSet container to the outside world
	//rather than pretend that it's an array.
	AFTK_ASSERT(instance < (U32)m_instancesOfThisNode.Size());
//	Ut::UtSet<C_SceneNode*>::ConstIterator it;
	Ut::UtVector<Ut::DynamicVector<C_SceneNode*> >::ConstIterator it;
	
	U32 i = 0;
	for (it = m_instancesOfThisNode.Begin(); it != m_instancesOfThisNode.End(); it++)
	{
		if (i == instance)
		{
			return *it;
		}
		i++;
	}

	return 0;
}

C_SceneNode* C_SceneNode::GetInstanceOfThisNode(U32 instance)
{
	//This way we avoid writing the above function twice.
	const C_SceneNode* pNode = ((const C_SceneNode*)this)->GetInstanceOfThisNode(instance);
	return const_cast<C_SceneNode*>(pNode);
}

void C_SceneNode::AddInstanceOfThisNode(C_SceneNode* pInstanceNode)
{
//	Ut::UtVector<Ut::DynamicVector<C_SceneNode*> >::Iterator it;
//	Ut::UtSet<C_SceneNode*>::Iterator it;
//	it = m_instancesOfThisNode.Find(pInstanceNode);
/*	if (it == m_instancesOfThisNode.End())
	{
		//We did not already track the given instance, add it.
		m_instancesOfThisNode.Insert(pInstanceNode);
	}
	else
	{
		//We already have this node instance tracked.
	}
	*/
	for (S32 i = 0; i < m_instancesOfThisNode.Size(); i++)
	{
		if (m_instancesOfThisNode[i] == pInstanceNode)
			return;
	}
	m_instancesOfThisNode.PushBack(pInstanceNode);
}

void C_SceneNode::DeleteInstanceOfThisNode(C_SceneNode* pInstanceNode)
{
	/*
	Ut::UtSet<C_SceneNode*>::Iterator it;
	it = m_instancesOfThisNode.Find(pInstanceNode);

	if (it != m_instancesOfThisNode.End())
	{
		//Delete the specified instance.
		m_instancesOfThisNode.Erase(pInstanceNode);
	}
	else
	{
		AFTK_ASSERT(it == m_instancesOfThisNode.End());
	}
*/
	for (S32 i = 0; i < m_instancesOfThisNode.Size(); i++)
	{
		if (m_instancesOfThisNode[i] == pInstanceNode)
		{
			m_instancesOfThisNode.EraseByIndex(i);
			return;
		}
	}

	assert(false);
}

U32 C_SceneNode::Repair()
{
	return ProcessProblems(true);
}

U32 C_SceneNode::Validate() const
{
	C_SceneNode* pThis = const_cast<C_SceneNode*>(this);
	return pThis->ProcessProblems(false);
}

void C_SceneNode::OnTransformPlugChanged(void* _pPlug)
{
	I_Plug* pPlug = (I_Plug*)_pPlug;

	C_SceneNode* pSceneNode;
	if (pPlug->IsPlugChild())
		pSceneNode = (C_SceneNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		pSceneNode = (C_SceneNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(pSceneNode && pSceneNode->HasType(E_ObjType_SceneNode));

	//Dirty caches.
	pSceneNode->DirtyAllTransformDependantCaches();

	//Re-evaluate all target plugs.
	for (U32 i = 0; i < (U32)pPlug->m_connectedPlugs.Size(); i++)
		pPlug->m_connectedPlugs[i]->OnChanged();
}

U32 C_SceneNode::ProcessProblems(bool fix)
{
	return 0;
}

std::string C_SceneNode::GetPath() const
{
	std::string finalString;
	if (m_pParent)
	{
		finalString = m_pParent->GetPath();
		finalString += "|";
	}
	finalString += m_name;
	return finalString;
}

C_Status C_SceneNode::Replace(C_SceneNode* pNode, bool assumeUniqueId)
{
	if (!pNode)
	{
		return C_Status(C_Status::E_Status_Failure, "Failed to replace node. Target node was null.");
	}

	bool firstChild = false;
	bool root = false;
	C_SceneNode* pPrevious	= pNode->GetPreviousPtr();
	C_SceneNode* pNext		= pNode->GetNextPtr();
	C_SceneNode* pParent	= pNode->GetParentPtr();
	C_SceneNode* pChild		= pNode->GetChildPtr();
	if (pParent)
	{
		if (m_pParent)
		{
			//Dirty parent's nChildren cache.
			m_pParent->m_nChildrenCache = -1;
		}

		if (pParent->GetChildPtr() == pNode)
		{
			firstChild = true;
		}
	}
	else if ((!pParent) && (!pPrevious))
	{
		root = true;
	}

	//Is the node we're replacing a member of a lod group? If so we'll need to add ourselves to this.
	C_SceneLodGroupNode* pLodGroup = 0;
	S32 lodLevelIndex = -1;
	Aftk::C_GroupLodLevelInfo lodLevelInfo;
	if (pNode->m_pParent)
	{
		if (pNode->m_pParent->GetType() == E_ObjType_SceneLodGroup)
		{
			lodLevelIndex = ((C_SceneLodGroupNode*)pNode->m_pParent)->GetLodLevelIndex(pNode);
			if (lodLevelIndex != -1)
			{
				pLodGroup = (C_SceneLodGroupNode*)pNode->m_pParent;
				lodLevelInfo = ((C_SceneLodGroupNode*)pNode->m_pParent)->GetLodLevelInfo(lodLevelIndex);
				lodLevelInfo.SetSceneNodePtr(this);
			}
		}
	}

	//Remove this node from scene graph.
	m_pScene->ExtractSceneNodeAndChildren(this);
	//Remove the node to replace from the scene graph.
	pNode->m_pScene->ExtractSceneNodeAndChildren(pNode);

	//We'll be reusing the unique id of the node we're replacing.
	S32 newUniqueId = pNode->m_uniqueId;

	//Make a note of the layers to node to be replaced is a member of.
	//Our replacement node will assume membership of these layers.
	Ut::UtVector<Ut::DynamicVector<C_LayerNode*> > layerList;
	for (U32 iLayer = 0; iLayer < pNode->GetNumLayers(); iLayer++)
		layerList.PushBack(pNode->GetLayerPtr(iLayer));


	//Delete the node we are to replace.
	delete pNode;

	////////////////// INSERT INTO HIERACHY //////////////////
	if (root)
	{
		if (!(m_pNext || m_pPrevious))
		{
			m_pScene->SetRootSceneNodePtr(this);
		}
		else
		{
			//If we're replaceing the root node _AND_ we have siblings, then this node cannot become the root.
			//Instead, parent this node to a new root.
			C_SceneNode* pNewRoot = new C_SceneNode(m_pScene);
			pNewRoot->SetName("AftkRoot");
			m_pScene->SetRootSceneNodePtr(pNewRoot);
			//Add this node as a child of our new root.
			pNewRoot->AddChildPtr(this);
		}
	}
	else if (firstChild)
	{
		pParent->SetChildPtr(this);
	}

	////////////////// END INSERT INTO HIERACHY //////////////////

	// We need to insert somewhere in the sibling array
	if (pPrevious)
	{
		pPrevious->m_pNext = this;
		m_pPrevious = pPrevious;
	}

	if (pNext)
	{
		pNext->m_pPrevious = this;
		m_pNext = pNext;
	}
	if (pParent)
	{
		//if the replaced node was the first child of the parent...
		if (pParent->m_pChild == pNode)
		{
			pParent->m_pChild = this;
		}
		m_pParent = pParent;
	}
	if (pChild)
	{
		m_pChild = pChild;
		//Loop thru and re-assign parental pointers for children.
		while (pChild)
		{
			pChild->m_pParent = this;
			pChild = pChild->m_pNext;
		}
	}

	//Remove the relacement node from any layers it's a member of.
	for (U32 iLayer = 0; iLayer < (U32)m_layers.Size(); iLayer++)
		m_layers[iLayer]->DeleteMember(this);

	//Add the replacement node to the layers of the replaced node.
	for (U32 iLayer = 0; iLayer < (U32)layerList.Size(); iLayer++)
		layerList[iLayer]->AddMember(this);

	//Add to lod group if required.
	if (pLodGroup)
	{
		pLodGroup->InsertLodLevelInfo(lodLevelIndex, lodLevelInfo);
	}

	if (assumeUniqueId)
	{
		//Assign new unique Id
#ifdef AFTK_USE_MEMORY_POOL
		/*
	std::map<
		S32, 
		C_SceneNode*,
		std::less<S32>,
		StdPoolAllocator<std::pair<S32, C_SceneNode*> > >::iterator it;
	*/
		Ut::UtMap<
		S32, 
		C_SceneNode*>::Iterator it;
#else
		Ut::UtMap<
		S32, 
		C_SceneNode*>::Iterator it;
#endif
		it = m_pScene->m_sceneNodes.Find(m_uniqueId);
		AFTK_ASSERT(it != m_pScene->m_sceneNodes.End());
		m_pScene->m_sceneNodes.Erase(it);

		m_pScene->m_sceneNodes[newUniqueId] = this;
		m_uniqueId = newUniqueId;
	}

	//Ensure that caches are refreshed.
	DirtyAllTransformDependantCaches();
	return C_Status::E_Status_Success;
}

void C_SceneNode::SetNullHierachyPtrs()
{
	m_pParent = 0;
	m_pChild = 0;
	m_pNext = 0;
	m_pPrevious = 0;
	//Dirty our cache.
	m_nChildrenCache = -1;
}

U32 C_SceneNode::GetNumChildren() const
{
	if (m_nChildrenCache != -1)
	{
		return m_nChildrenCache;
	}
	else
	{
		m_nChildrenCache = 0;
		if (m_pChild != 0)
		{
			C_SceneNode* pCurrentChild = m_pChild;
			while (pCurrentChild)
			{
				m_nChildrenCache++;
				if (pCurrentChild->m_pNext)
				{
					AFTK_ASSERT(pCurrentChild->m_pNext != pCurrentChild->m_pPrevious);
				}
				pCurrentChild = pCurrentChild->m_pNext;
			}
		}
	}

	return m_nChildrenCache;
}

U32 C_SceneNode::GetNumLayers() const
{
	return (U32)m_layers.Size();
}

S32 C_SceneNode::AddInfluenceClusterPtr(C_InfluenceClusterNode* pInfluenceCluster)
{
	for (U32 i = 0; i < (U32)m_influenceClusters.Size(); i++)
	{
		if (m_influenceClusters[i] == pInfluenceCluster)
		{
			return i;
		}
	}

	m_influenceClusters.PushBack(pInfluenceCluster);
	return m_influenceClusters.Size() -1;
}

void C_SceneNode::DeleteInfluenceClusterPtr(C_InfluenceClusterNode* pInfluenceCluster)
{
	for (U32 i = 0; i < (U32)m_influenceClusters.Size(); i++)
	{
		if (m_influenceClusters[i] == pInfluenceCluster)
		{
			m_influenceClusters.EraseByIndex(i);
			return;
		}
	}
	AFTK_ASSERT(false);
}

void C_SceneNode::DeleteInfluenceClusterPtr(U32 instance)
{
	m_influenceClusters.EraseByIndex(instance);
}

const C_SceneNode& C_SceneNode::operator=(const C_SceneNode& rhs)
{
	C_Node::operator=(rhs);

	ClearThis();
	
	m_pParent				= 0;
	m_pChild				= 0;
	m_pNext					= 0;
	m_pPrevious				= 0;
	SetBasePose(rhs.m_basePose);

	//If this node was not a member of a scene it will have been added by C_Node.
	//However, we need to provide it with a unique ID.
	if (m_uniqueId == -1)
	{
		// Add this node to the scene node list
		m_uniqueId = rhs.m_pScene->OnCreateSceneNode(this);
	}

	//Leave m_uniqueId as-is if already in existance
	AFTK_ASSERT(rhs.m_pScene != NULL);

	//Remove this node from any influence clusters it might have
	for (U32 i = 0; i < (U32)m_influenceClusters.Size(); i++)
	{
		//Remove the influence cluster associated with this node.
		m_influenceClusters[i]->SetSceneNodePtr(0);
	}

	m_rotationOrder				= rhs.m_rotationOrder;
	m_scalePivot				= rhs.m_scalePivot;
	m_scalePivotTranslation		= rhs.m_scalePivotTranslation;
	m_rotatePivot				= rhs.m_rotatePivot;
	m_rotatePivotTranslation	= rhs.m_rotatePivotTranslation;
	m_rotateOrientation			= rhs.m_rotateOrientation;
	m_rotationInterpolation		= rhs.m_rotationInterpolation;
	m_visibilityPlug			= rhs.m_visibilityPlug;

	//Note that this does not copy the m_instancesOfThisNode list.
	if (m_pScene == rhs.m_pScene)
	{
		//Copy blind data if required.
		if (m_pBlindDataResource != rhs.m_pBlindDataResource)
		{
			if (m_pBlindDataResource)
				m_pScene->DecrementResourceReferenceCount(m_pBlindDataResource);

			m_pBlindDataResource = rhs.m_pBlindDataResource;

			if (m_pBlindDataResource)
				m_pScene->IncrementResourceReferenceCount(m_pBlindDataResource);
		}

		//Copy across layer information.
		{
			//Remove this node from any layers it's a member of.
			for (U32 iLayer = 0; iLayer < (U32)m_layers.Size(); iLayer++)
				m_layers[iLayer]->DeleteMember(this);

			//Add this node to the layers of the rhs node.
			for (U32 iLayer = 0; iLayer < (U32)rhs.m_layers.Size(); iLayer++)
				rhs.m_layers[iLayer]->AddMember(this);
		}

		//Copy influence clusters of rhs [ONLY IF COPYING BETWEEN SAME SCENE!]
		for (U32 i = 0; i < (U32)rhs.m_influenceClusters.Size(); i++)
		{
			C_InfluenceClusterNode* pNewInfluenceCluster = new
				C_InfluenceClusterNode(m_pScene);
			*pNewInfluenceCluster = *rhs.m_influenceClusters[i];
			pNewInfluenceCluster->SetSceneNodePtr(this);
		}
		
		//Copy transform connectivity.
		m_translatePlug = rhs.m_translatePlug;
		m_rotatePlug = rhs.m_rotatePlug;
		m_scalePlug = rhs.m_scalePlug;
		m_shearPlug = rhs.m_shearPlug;
	}
	//Else we're copying this node between scenes
	else
	{
		//Copy blind data.
		{
			//Decrement existing blind data
			if (m_pBlindDataResource)
			{
				m_pScene->DecrementResourceReferenceCount(m_pBlindDataResource);
				m_pBlindDataResource = 0;
			}
	
			if (rhs.m_pBlindDataResource)
			{
				//Perform copy.
				C_BlindDataNode bd = rhs.m_pBlindDataResource->GetContent();
				//We must create a new blind data instance in this scene.
				m_pBlindDataResource = m_pScene->AddBlindDataNode(bd);
				//Increment new bd reference count.
				m_pScene->IncrementResourceReferenceCount(m_pBlindDataResource);
			}
		}

		//Remove this node from any layers it's a member of.
		for (U32 iLayer = 0; iLayer < (U32)m_layers.Size(); iLayer++)
			m_layers[iLayer]->DeleteMember(this);

		{
			F32 temp;
			//Disassociate from any transform drivers and copy values.
			m_translatePlug.SetSourceConnectionPtr(0);
			for (U32 iChild = 0; iChild < m_translatePlug.GetNumChildren(); iChild++)
			{
				((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(iChild))->SetSourceConnectionPtr(0);

				((C_StaticPlug<F32>*)rhs.m_translatePlug.GetChildPtr(iChild))->GetValue(temp, false);
				((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(iChild))->SetValue(temp);
			}

			m_rotatePlug.SetSourceConnectionPtr(0);
			for (U32 iChild = 0; iChild < m_rotatePlug.GetNumChildren(); iChild++)
			{
				((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(iChild))->SetSourceConnectionPtr(0);

				((C_StaticPlug<F32>*)rhs.m_rotatePlug.GetChildPtr(iChild))->GetValue(temp, false);
				((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(iChild))->SetValue(temp);
			}

			m_scalePlug.SetSourceConnectionPtr(0);
			for (U32 iChild = 0; iChild < m_scalePlug.GetNumChildren(); iChild++)
			{
				((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(iChild))->SetSourceConnectionPtr(0);

				((C_StaticPlug<F32>*)rhs.m_scalePlug.GetChildPtr(iChild))->GetValue(temp, false);
				((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(iChild))->SetValue(temp);
			}

			m_shearPlug.SetSourceConnectionPtr(0);
			for (U32 iChild = 0; iChild < m_shearPlug.GetNumChildren(); iChild++)
			{
				((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(iChild))->SetSourceConnectionPtr(0);
				
				((C_StaticPlug<F32>*)rhs.m_shearPlug.GetChildPtr(iChild))->GetValue(temp, false);
				((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(iChild))->SetValue(temp);
			}
		}
	}

	//Copy dynamic plugs
	m_dynamicPlugs.Resize(rhs.m_dynamicPlugs.Size());
	for (U32 i = 0; i < (U32)rhs.m_dynamicPlugs.Size(); i++)
	{
		m_dynamicPlugs[i] = new C_DynamicPlug(*((C_DynamicPlug*)rhs.m_dynamicPlugs[i]));
		m_dynamicPlugs[i]->SetLogicalIndex(i);
		m_dynamicPlugs[i]->SetParentPtr(this);
	}
/*
#ifdef BUILD_CONFIG_DEBUG
	if (GetType() != Aftk::E_ObjType_SceneJoint)
	{
		Matrix44 l = GetLocalMatrix(false);
		Matrix44 r = rhs.GetLocalMatrix(false);
		AFTK_ASSERT(Aftk::EqualityTestFMatrix(l, r));
	}
#endif
	*/
	return *this;
}

void C_SceneNode::OneTimeInit(C_Scene* pScene)
{
	m_uniqueId = -1;
	m_normalInverseTranspose = 1;

	//For the love of God, only reset this node's scene within the constructor. Otherwise we've got
	//No way of referencing shared objects held by the scene.
	m_pScene = pScene;

	if (m_pScene)
	{
		// Add this node to the scene node list
		m_uniqueId = m_pScene->OnCreateSceneNode(this);
	}

	SetNullHierachyPtrs();

	m_pBlindDataResource = 0;

	//Initialise transform plugs.
	{
		//Translate
		m_translatePlug.SetLogicalIndex(AFTK_SCENE_NODE_STATIC_PLUG_TRANSLATE);
		m_translatePlug.SetParentPtr(this);
		m_translatePlug.SetName("translate");
		m_translatePlug.m_children.Reserve(3);
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_translatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_translatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_translatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}
	{
		//Rotate
		m_rotatePlug.SetLogicalIndex(AFTK_SCENE_NODE_STATIC_PLUG_ROTATE);
		m_rotatePlug.SetParentPtr(this);
		m_rotatePlug.SetName("rotate");
		m_rotatePlug.m_children.Reserve(3);

		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_rotatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_rotatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_rotatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}
	{
		//Scale
		m_scalePlug.SetLogicalIndex(AFTK_SCENE_NODE_STATIC_PLUG_SCALE);
		m_scalePlug.SetParentPtr(this);
		m_scalePlug.SetName("scale");
		m_scalePlug.m_children.Reserve(3);

		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_scalePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_scalePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_scalePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}
	{
		//Shear
		m_shearPlug.SetLogicalIndex(AFTK_SCENE_NODE_STATIC_PLUG_SHEAR);
		m_shearPlug.SetParentPtr(this);
		m_shearPlug.SetName("shear");
		m_shearPlug.m_children.Reserve(3);

		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_shearPlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_shearPlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnTransformPlugChanged);
		m_shearPlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	//World matrix
	{
		m_worldMatrixProxyPlug.SetLogicalIndex(AFTK_SCENE_NODE_STATIC_PLUG_WORLD_MATRIX);
		m_worldMatrixProxyPlug.SetParentPtr(this);
		m_worldMatrixProxyPlug.SetName("worldMatrix");
	}

	{
		m_visibilityPlug.SetLogicalIndex(AFTK_SCENE_NODE_STATIC_PLUG_VISIBILITY);
		m_visibilityPlug.SetParentPtr(this);
		m_visibilityPlug.SetName("visibility");
	}

	m_hierarchyValid = true;
	m_staticMatrixCacheDirtyFlag = true;
	m_worldBoundsCacheDirtyFlag = true;
	m_dynamicMatrixCacheDirtyFlag = true;
	m_localBoundsCacheDirtyFlag = true;

	ClearThis();
}

void C_SceneNode::Clear()
{
	ClearThis();
	C_Node::Clear();
}

void C_SceneNode::ClearThis()
{
	// Important! These are setup in the constructor
	if (m_pScene && m_pBlindDataResource)
	{
		m_pScene->DecrementResourceReferenceCount(m_pBlindDataResource);
		m_pBlindDataResource = 0;
	}

	//Remove animation driver association and set identity values.
	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(0))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(1))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(2))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(0))->SetValue(0.f);
	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(1))->SetValue(0.f);
	((C_StaticPlug<F32>*)m_translatePlug.GetChildPtr(2))->SetValue(0.f);

	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(0))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(1))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(2))->SetSourceConnectionPtr(0);;
	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(0))->SetValue(0.f);
	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(1))->SetValue(0.f);
	((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(2))->SetValue(0.f);

	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(0))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(1))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(2))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(0))->SetValue(1.f);
	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(1))->SetValue(1.f);
	((C_StaticPlug<F32>*)m_scalePlug.GetChildPtr(2))->SetValue(1.f);

	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(0))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(1))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(2))->SetSourceConnectionPtr(0);
	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(0))->SetValue(0.f);
	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(1))->SetValue(0.f);
	((C_StaticPlug<F32>*)m_shearPlug.GetChildPtr(2))->SetValue(0.f);

	m_rotationOrder = E_RotationOrder_XYZ;

	memset(&m_scalePivot, 0, sizeof(Vector3F32));
	memset(&m_scalePivotTranslation, 0, sizeof(Vector3F32));
	memset(&m_rotatePivot, 0, sizeof(Vector3F32));
	memset(&m_rotatePivotTranslation, 0, sizeof(Vector3F32));
	UtMaths::SetIdentity(&m_rotateOrientation);
	m_rotationInterpolation = E_RotationInterpolation_Independent;

	//Remove influence cluster association.
	{
		//Copy the influence cluster array, as this will be modified by the cluster::deletescenenodeptr function.
		Ut::UtVector< Ut::DynamicVector<C_InfluenceClusterNode*> > influenceClusters = m_influenceClusters;		

		//Remove this node from the relevant influence clusters
		for (U32 i = 0; i < (U32)influenceClusters.Size(); i++)
		{
			//This function performs additional behind-the-scenes work, erasing the cluster from the array
			//we've just copied. If we didn't copy the array before this loop, we'd crash out due to the size
			//of the array changing within the loop.
			influenceClusters[i]->SetSceneNodePtr(0);
		}
	}

	SetNullHierachyPtrs();

	//Clear base pose information.
	UtMaths::SetIdentity(&m_basePose);
#ifndef AFTK_LEAN_AND_MEAN
	UtMaths::SetIdentity(&m_basePoseInverse);
#endif

	//Local matrices
	UtMaths::SetIdentity(&m_staticLocalMatrixCache);
#ifndef AFTK_LEAN_AND_MEAN
	UtMaths::SetIdentity(&m_staticLocalMatrixInverseCache);
	UtMaths::SetIdentity(&m_staticLocalMatrixInverseTransposeCache);
#endif

	//Dynamic local matrices
	UtMaths::SetIdentity(&m_dynamicLocalMatrixCache);
#ifndef AFTK_LEAN_AND_MEAN
	UtMaths::SetIdentity(&m_dynamicLocalMatrixInverseCache);
	UtMaths::SetIdentity(&m_dynamicLocalMatrixInverseTransposeCache);
#endif

	//World matrices
	UtMaths::SetIdentity(&m_staticWorldMatrixCache);
#ifndef AFTK_LEAN_AND_MEAN
	UtMaths::SetIdentity(&m_staticWorldMatrixInverseCache);
	UtMaths::SetIdentity(&m_staticWorldMatrixInverseTransposeCache);
#endif

	//Dynamic world matricies
	UtMaths::SetIdentity(&m_dynamicWorldMatrixCache);
#ifndef AFTK_LEAN_AND_MEAN
	UtMaths::SetIdentity(&m_dynamicWorldMatrixInverseCache);
	UtMaths::SetIdentity(&m_dynamicWorldMatrixInverseTransposeCache);
#endif

	//Cache flags
	DirtyAllTransformDependantCaches();

#ifndef AFTK_LEAN_AND_MEAN
	m_localBoundsCache.Clear();
	m_localEvaluatedBoundsCache.Clear();
#endif

	//Remove this node from any layers.
	{
		Ut::UtVector<Ut::DynamicVector<C_LayerNode*> > layersToProcess = 
			m_layers;
		for (U32 iLayer = 0; iLayer < (U32)layersToProcess.Size(); iLayer++)
			layersToProcess[iLayer]->DeleteMember(this);
	}

	for (U32 i = 0; i < (U32)m_dynamicPlugs.Size(); i++)
		delete m_dynamicPlugs[i];
}

void C_SceneNode::DirtyAllTransformDependantCaches() const
{
	if (m_staticMatrixCacheDirtyFlag && m_worldBoundsCacheDirtyFlag && m_dynamicMatrixCacheDirtyFlag)
		return;

	DirtyStaticTransformDependantCaches();
	DirtyDynamicTransformDependantCaches();
}

void C_SceneNode::DirtyStaticTransformDependantCaches() const
{
	if (m_staticMatrixCacheDirtyFlag && m_worldBoundsCacheDirtyFlag)
		return;

	//Dirty flag for local and world non-animated matrices.
	m_staticMatrixCacheDirtyFlag = true;
	m_worldBoundsCacheDirtyFlag = true;

#ifndef AFTK_LEAN_AND_MEAN
	//World-space non-animate bounds.
	m_staticWorldBoundsCache.Clear();
	m_staticWorldEvaluatedBoundsCache.Clear();
	m_dynamicWorldBoundsCache.Clear();
	m_dynamicWorldEvaluatedBoundsCache.Clear();
#endif

	if (m_hierarchyValid)
	{
		C_SceneNode* pChild = m_pChild;
		while(pChild && pChild->m_hierarchyValid)
		{
			pChild->DirtyStaticTransformDependantCaches();
			pChild = pChild->m_pNext;
		}
	}
}

void C_SceneNode::DirtyDynamicTransformDependantCaches() const
{
	if (m_dynamicMatrixCacheDirtyFlag && m_worldBoundsCacheDirtyFlag)
		return;

	//Dirty flag for local and world-space animated matrices.
	m_dynamicMatrixCacheDirtyFlag = true;
	m_worldBoundsCacheDirtyFlag = true;

	//TODO: Sort this
	const_cast<C_ProxyPlug<Matrix44>&>(m_worldMatrixProxyPlug).OnChanged();

	//Force an update of nodes evaluating us.
//	for (U32 i = 0; i < (U32)m_connectedPlugs.Size(); i++)
//		m_connectedPlugs[i]->OnChanged();

	if (m_hierarchyValid)
	{
		C_SceneNode* pChild = m_pChild;
		while(pChild && pChild->m_hierarchyValid)
		{
			pChild->DirtyDynamicTransformDependantCaches();
			pChild = pChild->m_pNext;
		}
	}
}

const Matrix44& C_SceneNode::GetLocalMatrix(bool evaluate) const
{
	if (!evaluate)
	{
		if (m_staticMatrixCacheDirtyFlag)
			RecalculateStaticMatrixCache();
		return m_staticLocalMatrixCache;
	}

	//Evaluate both static and dynamic caches.
	return EvaluateLocalMatrix();
}

Matrix44 C_SceneNode::GetLocalMatrixInverse(bool evaluate) const
{
	if (!evaluate)
	{
		if (m_staticMatrixCacheDirtyFlag)
			RecalculateStaticMatrixCache();
	}

#ifndef AFTK_LEAN_AND_MEAN
	if (!evaluate)
	{
		return m_staticLocalMatrixInverseCache;
	}

	//Evaluate both static and dynamic caches.
	return EvaluateLocalMatrixInverse();
#else
	if (!evaluate)
	{
		Matrix44 tmp;
		UtMaths::Invert4x4(&tmp, m_staticLocalMatrixCache);
		return tmp;
	}
	else
	{
		//Evaluate both static and dynamic caches.
		return EvaluateLocalMatrixInverse();
	}
#endif
}

Matrix44 C_SceneNode::GetLocalMatrixInverseTranspose(bool evaluate) const
{
	if (!evaluate)
	{
		if (m_staticMatrixCacheDirtyFlag)
			RecalculateStaticMatrixCache();
	}

#ifndef AFTK_LEAN_AND_MEAN
	if (!evaluate)
	{
		return m_staticLocalMatrixInverseTransposeCache;
	}

	//Evaluate both static and dynamic caches.
	return EvaluateLocalMatrixInverseTranspose();
#else
	if (!evaluate)
	{
		Matrix44 staticLocalMatrixInverseTranspose;
		Matrix44 matrixTranspose = m_staticLocalMatrixCache;
		UtMaths::Transpose(&matrixTranspose);
		UtMaths::Invert4x4(&staticLocalMatrixInverseTranspose, matrixTranspose);
		return staticLocalMatrixInverseTranspose;
	}
	else
	{
		return EvaluateLocalMatrixInverseTranspose();
	}
#endif
}

void C_SceneNode::RecalculateStaticMatrixCache() const
{
	//Build dynamic local matrix.
	{
		Vector3F32 translate;
		Vector3F32 scale;
		Vector3F32 shear;
		Vector3F32 rotate;

		GetTranslation(translate, false);
		GetScale(scale, false);
		GetShear(shear, false);
		GetRotation(rotate, false);

		BuildMatrix(
			m_staticLocalMatrixCache,
			m_scalePivot,
			scale,
			shear,
			m_scalePivotTranslation,
			m_rotatePivot,
			m_rotateOrientation,
			rotate, 
			m_rotatePivotTranslation,
			translate,
			m_rotationOrder);
	}


#ifndef AFTK_LEAN_AND_MEAN
	//Derive other local matrices.
	UtMaths::Invert4x4(&m_staticLocalMatrixInverseCache, m_staticLocalMatrixCache);
	Matrix44 matrixTranspose = m_staticLocalMatrixCache;
	UtMaths::Transpose(&matrixTranspose);
	UtMaths::Invert4x4(&m_staticLocalMatrixInverseTransposeCache, matrixTranspose);
#endif
	//Update world matrix cache
	{
		//Multiply in parent transform.
		if (m_pParent)
		{
			const Matrix44& parentMatrix = m_pParent->GetWorldMatrix(false);
			m_staticWorldMatrixCache = m_staticLocalMatrixCache * parentMatrix;
		}
		else
		{
			m_staticWorldMatrixCache = m_staticLocalMatrixCache;
		}

#ifndef AFTK_LEAN_AND_MEAN
		//Derive other static world matrices.
		{
			//Calculate inverse.
			UtMaths::Invert4x4(&m_staticWorldMatrixInverseCache, m_staticWorldMatrixCache);
			//Calculate inverse transpose
			Matrix44 matrixTranspose = m_staticWorldMatrixCache;
			UtMaths::Transpose(&matrixTranspose);
			UtMaths::Invert4x4(&m_staticWorldMatrixInverseTransposeCache, matrixTranspose);
		}
#endif

		//We're no longer flagged as dirty.
		m_staticMatrixCacheDirtyFlag	= false;
		//Dirty evaluation time for dynamic cache
		m_dynamicMatrixCacheDirtyFlag	= true;
		
		//Dirty our bounds.
		m_worldBoundsCacheDirtyFlag	= true;
	}
}

void C_SceneNode::EvaluateRotation(
	Matrix44& rotation) const
{
	//Build a slerped rotation matrix.
	BuildRotationMatrix(
		rotation, 
		this,
		((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(0))->GetSourceConnectionPtr(), 
		((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(1))->GetSourceConnectionPtr(),
		((C_StaticPlug<F32>*)m_rotatePlug.GetChildPtr(2))->GetSourceConnectionPtr());
}

void C_SceneNode::RecalculateDynamicMatrixCache() const
{
	//Build dynamic local matrix.
	{
		Vector3F32 translate;
		Vector3F32 scale;
		Vector3F32 shear;
		Matrix44 rotate;

		GetTranslation(translate, true);
		GetScale(scale, true);
		GetShear(shear, true);
		GetRotation(rotate, true);

		BuildMatrix(
			m_dynamicLocalMatrixCache,
			m_scalePivot,
			scale,
			shear,
			m_scalePivotTranslation,
			m_rotatePivot,
			m_rotateOrientation,
			rotate, 
			m_rotatePivotTranslation,
			translate);
	}

#ifndef AFTK_LEAN_AND_MEAN
	//Derive other dynamic local matrices.
	{
		//Calculate inverse.
		UtMaths::Invert4x4(&m_dynamicLocalMatrixInverseCache, m_dynamicLocalMatrixCache);
		//Calculate inverse transpose
		Matrix44 matrixTranspose = m_dynamicLocalMatrixCache;
		UtMaths::Transpose(&matrixTranspose);
		UtMaths::Invert4x4(&m_dynamicLocalMatrixInverseTransposeCache, matrixTranspose);
	}
#endif

	if (m_pParent)
	{
		//Apply parental transform - this will potentially trigger re-evaluation of the parent's
		//world matrix. Local space is transformed into world space by post-multiplication.
		m_dynamicWorldMatrixCache = 
			m_dynamicLocalMatrixCache * m_pParent->EvaluateWorldMatrix();
	}
	else
	{
		m_dynamicWorldMatrixCache = m_dynamicLocalMatrixCache;
	}

#ifndef AFTK_LEAN_AND_MEAN
	//Derive other dynamic world matrices
	{
		//Calculate inverse
		UtMaths::Invert4x4(&m_dynamicWorldMatrixInverseCache, m_dynamicWorldMatrixCache);
		//Calculate inverse transpose
		Matrix44 matrixTranspose = m_dynamicWorldMatrixCache;
		UtMaths::Transpose(&matrixTranspose);
		UtMaths::Invert4x4(&m_dynamicWorldMatrixInverseTransposeCache, matrixTranspose);
	}
#endif

	m_dynamicMatrixCacheDirtyFlag = false;
}

void C_SceneNode::AddNextPtr(C_SceneNode* pNode)
{
	if (m_pParent)
	{
		//Dirty parent's nChildren cache.
		m_pParent->m_nChildrenCache = -1;
	}

	if (pNode->m_pParent)
	{
		//Dirty parent's nChildren cache.
		pNode->m_pParent->m_nChildrenCache = -1;

		AFTK_ASSERT(pNode->m_pParent != m_pParent);
	}

	if (pNode->m_pNext)
	{
		AFTK_ASSERT(pNode->m_pNext != pNode->m_pPrevious);
	}
	AFTK_ASSERT(m_pScene->GetRootSceneNodePtr() != this);
	AFTK_ASSERT(pNode != this);
	//Find an end location to insert into sibling list
	C_SceneNode* pLastSibling = this;
	while (pLastSibling->m_pNext)
	{
		pLastSibling = pLastSibling->m_pNext;
	}
//	AFTK_ASSERT(pNode->m_pNext != pLastSibling);
	pLastSibling->m_pNext = pNode;
	pNode->m_pPrevious = pLastSibling;
	
	// Set parental pointers for all children of this node
	C_SceneNode* pCurrentChild = pNode;
	while (pCurrentChild)
	{
		pCurrentChild->m_pParent = m_pParent;
		pCurrentChild = pCurrentChild->m_pNext;
	}

	AFTK_ASSERT(pNode->m_pNext != pNode->m_pPrevious);
	AFTK_ASSERT(pLastSibling->m_pNext != pLastSibling->m_pPrevious);
	
	//Dirty matrix cache.
	pNode->DirtyAllTransformDependantCaches();
	//Dirty siblings.
	while (pNode->m_pNext)
	{
		pNode->DirtyAllTransformDependantCaches();
		pNode = pNode->m_pNext;
	}
}

bool C_SceneNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_SceneNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (classToAbortTest == E_ObjType_SceneNode)
		return true;
	return (C_SceneNode::IsEqualToThis(rhs, flags) && C_Node::IsEqualToAll(rhs, classToAbortTest));
}

bool C_SceneNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_SceneNode))
	{
		return false;
	}

	const C_SceneNode& rhs = (C_SceneNode&) _rhs;
	
	{
		if (!(m_layers.Size() == rhs.m_layers.Size()))
			return false;

		Ut::UtSet<C_LayerNode*> layerSet;
		for (U32 iLayer = 0; iLayer < (U32)m_layers.Size(); iLayer++)
			layerSet.Insert(m_layers[iLayer]);

		//Our arrays of layers are the same size - check that the layerSet for the lhs contains all of 
		//the rhs layers.
		for (U32 iLayer = 0; iLayer < (U32)rhs.m_layers.Size(); iLayer++)
		{
			Ut::UtSet<C_LayerNode*>::Iterator it = 
				layerSet.Find(rhs.m_layers[iLayer]);
			if (it == layerSet.End())
				return false;
		}
	}
	{
		if (!(m_influenceClusters.Size() == rhs.m_influenceClusters.Size()))
			return false;

		Ut::UtSet<C_InfluenceClusterNode*> clusterSet;
		for (U32 iInfluenceCluster = 0; iInfluenceCluster < (U32)m_influenceClusters.Size(); iInfluenceCluster++)
			clusterSet.Insert(m_influenceClusters[iInfluenceCluster]);

		//Our arrays of layers are the same size - check that the layerSet for the lhs contains all of 
		//the rhs layers.
		for (U32 iInfluenceCluster = 0; iInfluenceCluster < (U32)rhs.m_influenceClusters.Size(); iInfluenceCluster++)
		{
			Ut::UtSet<C_InfluenceClusterNode*>::Iterator it = 
				clusterSet.Find(rhs.m_influenceClusters[iInfluenceCluster]);
			if (it == clusterSet.End())
				return false;
		}
	}

	if (flags & E_Comparison_WorldTransform)
	{
		const Aftk::Matrix44& m1 = GetWorldMatrix();
		const Aftk::Matrix44& m2 = rhs.GetWorldMatrix();

		if (!Aftk::EqualityTestFMatrix(m1, m2))
			return false;

	}
	else if (
		(m_rotationInterpolation != rhs.m_rotationInterpolation) ||
		(m_rotationOrder != rhs.m_rotationOrder) ||
		(!EqualityTestFVector(m_scalePivot, rhs.m_scalePivot)) ||
		(!EqualityTestFVector(m_scalePivotTranslation, rhs.m_scalePivotTranslation)) ||
		(!EqualityTestFVector(m_rotatePivot, rhs.m_rotatePivot)) ||
		(!EqualityTestFVector(m_rotatePivotTranslation, rhs.m_rotatePivotTranslation)) ||
		(!EqualityTestFMatrix(m_rotateOrientation, rhs.m_rotateOrientation))
		)
		return false;
	
	if (GetNumPlugs() != rhs.GetNumPlugs())
		return false;
	for (U32 i = 0; i < GetNumPlugs(); i++)
		if (!GetPlugPtr(i)->IsEqualTo(*rhs.GetPlugPtr(i)))
			return false;

	return true;
}

U32 C_SceneNode::GetDepth() const
{
	if (!m_pParent)
		return 0;

	U32 depth = 1;
	Aftk::C_SceneNode* pParent = m_pParent;
	while(pParent)
	{
		pParent = pParent->m_pParent;
		++depth;
	}
	return depth;
}

bool C_SceneNode::operator==(const C_SceneNode &rhs) const
{
	return IsEqualToAll(rhs);
}

U32 C_SceneNode::GetHashOfThis() const
{
	U32 hash = 1;
	hash*= (U32)m_layers.Size();
	
	Matrix44 mat = GetWorldMatrix();

	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			if (U32 val = (U32)mat[i][j])
			{
				hash*=val;
			}
		}
	}

	return hash;
}

U32 C_SceneNode::GetHash(E_ObjType classToAbortTest) const
{
	if (classToAbortTest == E_ObjType_SceneNode)
		return true;
	return C_SceneNode::GetHashOfThis()*C_Node::GetHash();
}

void C_SceneNode::SetRotationOrder(E_RotationOrder order)
{
	if (order == m_rotationOrder)
		return;
	m_rotationOrder = order;
	//Recalculate transform using new order.
	DirtyAllTransformDependantCaches();
}

E_RotationOrder C_SceneNode::GetRotationOrder() const
{
	return m_rotationOrder;
}

void C_SceneNode::RecalculateLocalBoundsCache() const
{
#ifndef AFTK_LEAN_AND_MEAN
	//Calculate local bounds.
	CalculateBounds(m_localBoundsCache, 0);

	m_localEvaluatedBoundsCache = m_localBoundsCache;
#endif
	m_localBoundsCacheDirtyFlag = false;
}

void C_SceneNode::RecalculateWorldBoundsCache() const
{
#ifndef AFTK_LEAN_AND_MEAN
	//Calculate static local bounds.
	CalculateBounds(m_staticWorldBoundsCache, &GetWorldMatrix(false));
	//Calculate dynamic local bounds.
	CalculateBounds(m_dynamicWorldBoundsCache, &GetWorldMatrix(true));

	m_dynamicWorldEvaluatedBoundsCache = m_dynamicWorldBoundsCache;
	m_staticWorldEvaluatedBoundsCache = m_staticWorldBoundsCache;
#endif
	m_worldBoundsCacheDirtyFlag = false;
}

void C_SceneNode::DeleteInstanceSource(C_SceneNode* pInstance)
{
	//noop
}

#endif
