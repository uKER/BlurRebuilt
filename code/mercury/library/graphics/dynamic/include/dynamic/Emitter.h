#ifndef RENDER_DYNAMICEFFECT_EMITTER_H
#define RENDER_DYNAMICEFFECT_EMITTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Node.h"

namespace DynamicEffect
{

// provides basic functionality for emitter style classes, eg particle emitters, strip emitters etc
template<class Block>
class C_EmitterBase
{
public:
	C_EmitterBase() { }
	virtual ~C_EmitterBase() { }

	void						SetLocalTransform(const C_Node& transform) { m_Transform = transform; }

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	void						SetLocalTransform(XMVECTOR vPosition, XMVECTOR qRotation);
#endif

	void						SetLocalTransform(const Maths::HVec3& vPosition, const Maths::C_Quaternion& qRotation);

	const C_Node*				GetLocalTransform() const { return &m_Transform; }
	void						SetTargetTransform(const C_Node& transform) { m_Target = transform; }
	const C_Node*				GetTargetTransform() const { return &m_Target; }

	void						SetTransform(const C_Node& transform, const C_Node* pParent);
	
	// optional global transformation, eg emitters attached to objects, world space particles (decals)
	void						SetParentTransform(const C_Node* pTransform) { m_pParentTransformLS = pTransform; m_pParentTransformEA = pTransform; }
	const C_Node*				GetParentTransform() const { return m_pParentTransformLS; }
	const C_Node*				GetParentTransformEA() const { return m_pParentTransformEA; }

	C_Node						GetWorldTransform() const;
	Maths::HVec3				GetWorldPosition() const; // get world position
	Maths::HVec3				GetLocalPosition() const; // get local position

	U32							GetSalt() const { return m_iSalt; }

	void						SetBlock(Block* pBlock);
	void						SetBlock(Block* pBlockLS, Block* pBlockEA);
	Block*						GetBlock() const;
	Block*						GetBlockLS() const { return m_pBlockLS; }
	Block*						GetBlockEA() const { return m_pBlockEA; }
	U32							GetBlockSalt() const { return m_iBlockSalt; }


#ifndef PLATFORM_SPU
protected:
#endif
	Block*						m_pBlockEA;
	Block*						m_pBlockLS;

	U32							m_iBlockSalt;
	U32							m_iSalt;

	const C_Node*				m_pParentTransformEA;			// optional transformation
	const C_Node*				m_pParentTransformLS;			// optional transformation

	MathsStorage::Float4A16		m_vPosition;		// world space position for world emitters, local for local
	C_Node						m_Transform;
	C_Node						m_Target;
	
};

template<class Block>
INLINE Block* C_EmitterBase<Block>::GetBlock() const
{
#ifdef PLATFORM_SPU
	if(m_pBlockEA)
		return m_pBlockLS;
	else
		return NULL;
#else
	return m_pBlockEA;
#endif
}

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
template<class Block>
FORCE_INLINE void C_EmitterBase<Block>::SetLocalTransform(XMVECTOR vPosition, XMVECTOR qRotation)
{
	m_Transform.SetPosition(vPosition);
	m_Transform.SetRotation(qRotation);
}
#endif

template<class Block>
FORCE_INLINE void C_EmitterBase<Block>::SetLocalTransform(const Maths::HVec3& vPosition, const Maths::C_Quaternion& qRotation)
{
	m_Transform.SetPosition(vPosition);
	m_Transform.SetRotation(qRotation);
}

template<class Block>
FORCE_INLINE void C_EmitterBase<Block>::SetBlock(Block* pBlock)
{
	m_pBlockEA = pBlock;
	m_iBlockSalt = pBlock ? pBlock->m_iSalt : 0;
}

template<class Block>
FORCE_INLINE void C_EmitterBase<Block>::SetBlock(Block* pBlockLS, Block* pBlockEA)
{
	m_pBlockLS = pBlockLS;
	m_pBlockEA = pBlockEA;
	m_iBlockSalt = pBlockLS ? pBlockLS->m_iSalt : 0;
}

template<class Block>
FORCE_INLINE void C_EmitterBase<Block>::SetTransform(const C_Node& transform, const C_Node* pParent)
{
	SetLocalTransform(transform);
	SetParentTransform(pParent);
}

template<class Block>
FORCE_INLINE C_Node C_EmitterBase<Block>::GetWorldTransform() const
{
	Maths::C_QuatTrans transform(m_Transform);
	if(m_pParentTransformLS)
	{
		Maths::C_QuatTrans globalTransform(*m_pParentTransformLS);
		transform = transform * globalTransform;
	}
	
	return C_Node(transform);
}

template<class Block>
FORCE_INLINE Maths::HVec3 C_EmitterBase<Block>::GetWorldPosition() const
{
	Maths::C_QuatTrans transform(m_Transform);
	if(m_pParentTransformLS)
	{
		Maths::C_QuatTrans globalTransform(*m_pParentTransformLS);
		transform = transform * globalTransform;
	}

	return transform.Translation();
}

template<class Block>
FORCE_INLINE Maths::HVec3 C_EmitterBase<Block>::GetLocalPosition() const
{
	Maths::HVec3 position(Maths::Load<Maths::HVec3>(&m_Transform.GetPosition()));
	return position;
}

}

#endif // RENDER_DYNAMICEFFECT_EMITTER_H
