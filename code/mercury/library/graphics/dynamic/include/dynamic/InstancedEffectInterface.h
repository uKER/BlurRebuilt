#ifndef RENDER_DYNAMICEFFECT_INSTANCEDEFFECT_H
#define RENDER_DYNAMICEFFECT_INSTANCEDEFFECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Interface.h"
#include "Render/View/ViewManager.h"

namespace DynamicEffect
{

extern Debug::Logger& g_Log;

enum E_InstancedObjectType
{
	E_InstancedObjectType_Persistent,			// added and exists until removed
	E_InstancedObjectType_DynamicFrame,			// needs to be added per frame to exist
	E_InstancedObjectType_DynamicView			// needs to be added per view to exist
};

template<class Object>
struct Instance
{
	mutable const Object*			pObject;
	Display::C_Matrix				transform;
	bool							bTransparent;
	mutable U32						iIndex;
	mutable U32						lights;
};

template <class Object, class ObjectInstance>
class I_InstancedEffect : public I_DynamicEffect
{
public:
	void	SetNumManagedObjects(S32 iNumManaged);
	void	BeginFrame();
	void	BeginView();

	// add instance by value
	void	AddInstance(const ObjectInstance* pInstance, const E_InstancedObjectType eObjectType);
	void	AddInstancePersistent(const ObjectInstance* pInstance);
	void	AddInstanceDynamicFrame(const ObjectInstance* pInstance);
	void	AddInstanceDynamicView(const ObjectInstance* pInstance);

	// add instance by pointer
	void	AddInstance(const ObjectInstance& instance, const E_InstancedObjectType eObjectType);
	void	AddInstancePersistent(const ObjectInstance& instance);
	void	AddInstanceDynamicFrame(const ObjectInstance& instance);
	void	AddInstanceDynamicView(const ObjectInstance& instance);

	void	ReserveInstances(U32 iPersistent, U32 iDynamicFrame, U32 iDynamicView);

protected:
	I_InstancedEffect(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : I_DynamicEffect(pRenderer, pManager, iIndex, iTypeIndex), m_iReservePersistent(0), m_iReserveDynamicFrame(0), m_iReserveDynamicView(0), m_iView(0) { m_bBegin = true; }
	virtual ~I_InstancedEffect() {}

	typedef	Ut::Vector<Object*>					ManagedObjectPtrs;
	typedef Ut::ListPool<Object>				ObjectList;
	typedef Ut::Vector<const ObjectInstance*>	ObjectInstancePtrs;
	typedef Ut::Vector<ObjectInstance>			ObjectInstances;

	void	OnGather(const SceneGraph::C_View& view) const;

	Object*	GetManagedObject(S32 iIndex);
	void	SetManagedObject(S32 iIndex, Object* pObject);
	
	// add instance by value
	void	Reserve();

	U32									m_iReservePersistent;
	U32									m_iReserveDynamicFrame;
	U32									m_iReserveDynamicView;
	
	ManagedObjectPtrs					m_ManagedObjects;
	ObjectList							m_ObjectList;

	ObjectInstancePtrs					m_InstancePtrsPersistent;	// Persistent instances are added/removed by update functions
	ObjectInstances						m_InstancesPersistent;
	mutable ObjectInstancePtrs			m_InstancePtrsDynamicFrame;	// volatile instances are added by render functions (eg level/city rendering) then cleared
	mutable ObjectInstances				m_InstancesDynamicFrame;
	
	S32									m_iView;
	mutable Ut::Vector<ObjectInstancePtrs>	m_InstancePtrsDynamicView;
	mutable Ut::Vector<ObjectInstances>		m_InstancesDynamicView;
};

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::OnGather(const SceneGraph::C_View& view) const
{

}

/*
template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::OnGatherBlocks(BlockArrays& blockArrays, const U32 iViewIndex) const
{
	Maths::HVec4 vViewDirection(Maths::Load<Maths::HVec4>(&m_pRenderer->GetViewDirection()));
	Maths::HF32 vDepthScale(Maths::Load<Maths::HF32>(&m_pRenderer->GetViewPosition().w));
	Maths::HF32 vMaxInt = (F32)0xFFFFFFFF;

	// Persistent instances will have visbits available
	for(S32 iInstance = 0; iInstance < m_InstancePtrsPersistent.Size(); ++iInstance)
	{
		const Object* pObject = m_InstancePtrsPersistent[iInstance]->pObject;
		const C_RenderNode* pBlock = (Object*)pObject;

		Maths::HMat43 mTransform = Maths::HMat43(Maths::Load<Maths::HMat44>(&m_InstancePtrsPersistent[iInstance]->transform));
		Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&pBlock->m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&pBlock->m_vSize));

		Maths::HVec3 vCentre3(vCentreOne);
		Maths::HVec3 vSize3(vSizeCull);

		Maths::HVec4 vCentre(vCentre3 * mTransform);
		Maths::HVec4 vSize(vSize3 * mTransform.Rotation());

		vCentre.W(vCentreOne.W());
		vSize.W(vSizeCull.W());

		S32 visible = Render::C_ViewManager::Instance().GetView(iViewIndex)->m_View.GetViewFrustum().CullSphere(vCentre, Maths::Length(vSize3), vSizeCull.W());

		if(visible)
		{
			C_RenderNode	block;
			block.pBlock = &(*pBlock);
			block.pMatrix = &m_InstancePtrsPersistent[iInstance]->transform;
			block.pInstance = (void*)m_InstancePtrsPersistent[iInstance];

			// calculate dynamic lights
			Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&pBlock->m_vCentre));
			Maths::HMat43 mTransform(Maths::Load<Maths::HMat44>(&m_InstancePtrsPersistent[iInstance]->transform));
			Maths::HVec3 vCentre3(vCentreOne);

			vCentre3 = vCentre3 * mTransform;
			Maths::HF32 fLength(Maths::Length(Maths::Load<Maths::HVec3>(&pBlock->m_vSize)));
			m_InstancePtrsPersistent[iInstance]->lights = m_pRenderer->GetLightMask(Maths::C_Sphere(vCentre3, fLength));
			
			if(m_InstancePtrsPersistent[iInstance]->bTransparent)
			{
				Maths::HVec4 vCentre(vCentre3);
				vCentre.W(vCentreOne.W());

				Maths::HF32 vDepth = (vCentre | vViewDirection);
				vDepth = vMaxInt - vDepth * vDepthScale;

				block.sortKey = (U32)vDepth;
				blockArrays.transparent.PushBack(block);
			}
			else
			{
				block.sortKey = m_iIndex;	// sort by effect instead of depth
				blockArrays.opaque.PushBack(block);
			}
		}
	}

	// volatile instances don't have visbits, so add them
	for(S32 iInstance = 0; iInstance < m_InstancePtrsDynamicView[m_iView].Size(); ++iInstance)
	{
		const ObjectInstance* pInstance = m_InstancePtrsDynamicView[m_iView][iInstance];
		const Object* pObject = pInstance->pObject;
		const C_RenderNode* pBlock = (Object*)pObject;

		C_RenderNode	block;
		block.pBlock = &(*pBlock);
		block.pMatrix = &pInstance->transform;
		block.pInstance = (void*)pInstance;
		
		// calculate dynamic lights
		Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&pBlock->m_vCentre));
		Maths::HMat43 mTransform(Maths::Load<Maths::HMat44>(&pInstance->transform));
		Maths::HVec3 vCentre3(vCentreOne);

		vCentre3 = vCentre3 * mTransform;
		Maths::HF32 fLength(Maths::Length(Maths::Load<Maths::HVec3>(&pBlock->m_vSize)));
		pInstance->lights = m_pRenderer->GetLightMask(Maths::C_Sphere(vCentre3, fLength));
		
		if(pInstance->bTransparent)
		{
			Maths::HVec4 vCentre(vCentre3);
			vCentre.W(vCentreOne.W());

			Maths::HF32 vDepth = (vCentre | vViewDirection);
			vDepth = vMaxInt - vDepth * vDepthScale;

			block.sortKey = (U32)vDepth;
			blockArrays.transparent.PushBack(block);
		}
		else
		{
			block.sortKey = m_iIndex;	// sort by effect instead of depth
			blockArrays.opaque.PushBack(block);
		}
	}

	// volatile instances don't have visbits, so add them
	for(S32 iInstance = 0; iInstance < m_InstancePtrsDynamicFrame.Size(); ++iInstance)
	{
		const ObjectInstance* pInstance = m_InstancePtrsDynamicFrame[iInstance];
		const Object* pObject = pInstance->pObject;
		const C_RenderNode* pBlock = (Object*)pObject;

		C_RenderNode	block;
		block.pBlock = &(*pBlock);
		block.pMatrix = &pInstance->transform;
		block.pInstance = (void*)pInstance;

		Maths::HMat43 mTransform;
		mTransform = Maths::HMat43(Maths::Load<Maths::HMat44>(&pInstance->transform));
		
		Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&pBlock->m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&pBlock->m_vSize));

		Maths::HVec3 vCentre3(vCentreOne);
		Maths::HVec3 vSize3(vSizeCull);

		vCentre3 = vCentre3 * mTransform;
		pInstance->lights = m_pRenderer->GetLightMask(Maths::C_Sphere(vCentre3, Maths::Length(vSize3)));

		Maths::HVec4 vCentre(vCentre3);
		Maths::HVec4 vSize(vSize3 * mTransform.Rotation());

		vCentre.W(vCentreOne.W());
		vSize.W(vSizeCull.W());

		S32 visible;
		visible = Display::Device()->GetView()->GetViewFrustum().CullSphere(vCentre, Maths::Length(vSize3), vSizeCull.W());

		if(visible)
		{
			if(pInstance->bTransparent)
			{
				Maths::HF32 vDepth = (vCentre | vViewDirection);
				vDepth = vMaxInt - vDepth * vDepthScale;

				block.sortKey = (U32)vDepth;
				blockArrays.transparent.PushBack(block);
			}
			else
			{
				block.sortKey = m_iIndex;	// sort by effect instead of depth
				blockArrays.opaque.PushBack(block);
			}
		}
	}
}
*/

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::SetNumManagedObjects(S32 iNumManaged)
{
	if(iNumManaged > m_ManagedObjects.Size())
	{
		Object* pObject = NULL;
		iNumManaged += iNumManaged % 16;
		m_ManagedObjects.Reserve(iNumManaged);

		while(m_ManagedObjects.Size() < iNumManaged)
			m_ManagedObjects.PushBack(pObject);
	}
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::AddInstance(const ObjectInstance* pInstance, const E_InstancedObjectType eObjectType)
{
	switch(eObjectType)
	{
	case E_InstancedObjectType_Persistent:
		AddInstancePersistent(pInstance);
		break;
	case E_InstancedObjectType_DynamicFrame:
		AddInstanceDynamicFrame(pInstance);
		break;
	case E_InstancedObjectType_DynamicView:
		AddInstanceDynamicView(pInstance);
		break;
	}
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::AddInstance(const ObjectInstance& instance, const E_InstancedObjectType eObjectType)
{
	switch(eObjectType)
	{
	case E_InstancedObjectType_Persistent:
		AddInstancePersistent(instance);
		break;
	case E_InstancedObjectType_DynamicFrame:
		AddInstanceDynamicFrame(instance);
		break;
	case E_InstancedObjectType_DynamicView:
		AddInstanceDynamicView(instance);
		break;
	}
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::AddInstancePersistent(const ObjectInstance* pInstance)
{
	m_InstancePtrsPersistent.PushBack(pInstance);
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::AddInstanceDynamicFrame(const ObjectInstance* pInstance)
{
	m_InstancePtrsDynamicFrame.PushBack(pInstance);
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::AddInstanceDynamicView(const ObjectInstance* pInstance)
{
	m_InstancePtrsDynamicView[m_iView].PushBack(pInstance);
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::AddInstancePersistent(const ObjectInstance& instance)
{
	if(m_InstancesPersistent.Size() == m_InstancesPersistent.Capacity())
	{
		LOGERROR(g_Log, "InstancedEffect: InstancesPersistent resized, clearing pointers");
		m_InstancePtrsPersistent.Clear();
	}
	m_InstancesPersistent.PushBack(instance);
	AddInstancePersistent(&m_InstancesPersistent.Back());
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::AddInstanceDynamicFrame(const ObjectInstance& instance)
{
	if(m_InstancesDynamicFrame.Size() == m_InstancesDynamicFrame.Capacity())
	{
		LOGERROR(g_Log, "InstancedEffect: InstancesDynamicFrame resized, clearing pointers");
		m_InstancePtrsDynamicFrame.Clear();
	}
	m_InstancesDynamicFrame.PushBack(instance);
	AddInstanceDynamicFrame(&m_InstancesDynamicFrame.Back());
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::AddInstanceDynamicView(const ObjectInstance& instance)
{
	if(m_InstancesDynamicView[m_iView].Size() == m_InstancesDynamicView[m_iView].Capacity())
	{
		LOGERROR(g_Log, "InstancedEffect: InstancesDynamicView resized, clearing pointers");
		m_InstancePtrsDynamicView[m_iView].Clear();
	}
	m_InstancesDynamicView[m_iView].PushBack(instance);
	AddInstanceDynamicView(&m_InstancesDynamicView[m_iView].Back());
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::BeginFrame()
{
	m_InstancesDynamicFrame.Clear();
	m_InstancePtrsDynamicFrame.Clear();

	S32 nViewCount = Render::C_ViewManager::Instance().GetViewCount();
	m_InstancesDynamicView.Resize(nViewCount);
	m_InstancePtrsDynamicView.Resize(nViewCount);

	for(S32 i = 0; i < nViewCount; ++i)
	{
		m_InstancesDynamicView[i].Clear();
		m_InstancePtrsDynamicView[i].Clear();
	}

	Reserve();

	// clear all blocks to not have a vertex buffer
	for(typename ObjectList::ConstIterator pIterator = m_ObjectList.m_Active.Begin(); pIterator != m_ObjectList.m_Active.End(); ++pIterator)
		pIterator->m_iVertexBuffer = 0xFFFF;
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::Reserve()
{
	m_InstancesPersistent.Reserve(m_iReservePersistent);
	m_InstancePtrsPersistent.Reserve(m_iReservePersistent);

	m_InstancesDynamicFrame.Reserve(m_iReserveDynamicFrame);
	m_InstancePtrsDynamicFrame.Reserve(m_iReserveDynamicFrame);

	for(S32 i = 0; i < m_InstancesDynamicView.Size(); ++i)
	{
		m_InstancesDynamicView[i].Reserve(m_iReserveDynamicView);
		m_InstancePtrsDynamicView[i].Reserve(m_iReserveDynamicView);
	}
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::ReserveInstances(U32 iPersistent, U32 iDynamicFrame, U32 iDynamicView)
{
	m_iReservePersistent = iPersistent;
	m_iReserveDynamicFrame = iDynamicFrame;
	m_iReserveDynamicView = iDynamicView;
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::BeginView()
{
	m_iView = Display::Device()->GetView()->GetIndex();
}

template <class Object, class ObjectInstance>
Object* I_InstancedEffect<Object, ObjectInstance>::GetManagedObject(S32 iIndex)
{
	assert(iIndex >= 0 && iIndex < m_ManagedObjects.Size());

	return m_ManagedObjects[iIndex];
}

template <class Object, class ObjectInstance>
void I_InstancedEffect<Object, ObjectInstance>::SetManagedObject(S32 iIndex, Object* pObject)
{
	m_ManagedObjects[iIndex] = pObject;
}

}

#endif // RENDER_DYNAMICEFFECT_INSTANCEDEFFECTMANAGER_H
