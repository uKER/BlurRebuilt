#ifndef RENDER_DYNAMICEFFECT_BLOCK_H
#define RENDER_DYNAMICEFFECT_BLOCK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system\memory.h>
#include <core\containers\bitvector.h>
#include <SceneGraph\RenderNode.h>

namespace SceneGraph
{
	class C_RenderState;
}

namespace DynamicEffect
{

class I_DynamicEffect;
class C_Node;

// basic unit of culling/batching 80 bytes
// [TODO SCENEGRAPH] derive off SceneGraph::C_BoundsCull at least, possibly C_PassCull as well
class ALIGN16 C_RenderNode : public SceneGraph::I_RenderNode
{
public:
	C_RenderNode() : m_pEffect(NULL), m_pNode(NULL), m_iFlags(0), m_iVertexBuffer(0xFFFF), m_WorkerID(0), m_VertexDataSize(0) {};

	// helper functions for correctly setting bounds and culldistance
	void						SetBounds(const Maths::C_AxisBox& axisBox, const Maths::HF32& cullDistance);
	void						SetBounds(const Maths::C_Sphere& sphere, const Maths::HF32& cullDistance);
	void						SetCullDistance(const F32 cullDistance);
	void						SetNodeType(const S32 type) { m_Type = type; }
	void						Reset();

	// used by renderer for culling/batching/building
	const I_DynamicEffect*		m_pEffect;				// pointer to the effect which this block is part of
	mutable U32					m_iFlags;				// various other stuff, visibility bits for 2d blocks etc
	MathsStorage::Float2		m_vLodFade;
	const C_Node*				m_pNode;				// pointer to transformation local space etc
	mutable U16					m_iVertexBuffer;		// index for vertex data block for this frame etc
	mutable U16					m_WorkerID;
	U32							m_VertexDataSize;

	// bounds
	MathsStorage::Float4A16		m_vCentre;				// needs to have 1.0 in W
	MathsStorage::Float4A16		m_vSize;				// cull distance is W
};

FORCE_INLINE void C_RenderNode::Reset()
{
	m_pEffect = NULL;
	m_pNode = NULL;
	m_iFlags = 0;
	m_iVertexBuffer = 0xFFFF;
}

class ElementVector
{
public:
	ElementVector();
	~ElementVector();

	void		Reserve(S32 elementSize, S32 capacity);
	void		Clear() { m_Size = 0; }

	S32			ElementSize() const { return m_ElementSize; }
	S32			Capacity() const { return m_Capacity; }
	S32			Remaining() const { return m_Capacity - m_Size; }
	S32			Size() const { return m_Size; }
	bool		Empty() const { return m_Size == m_Capacity; }

	U8&			Back() { return m_pData[(m_Size - 1) * m_ElementSize]; }
	const U8&	Back() const { return m_pData[(m_Size - 1) * m_ElementSize]; }
	U8&			Front() { return *m_pData; }
	const U8&	Front() const { return *m_pData; }

	U8*			PushBack() { assert(m_Size < m_Capacity); return &m_pData[m_Size++ * m_ElementSize];  }
	U8*			PushBack(S32 num)
				{
					assert(m_Size + num <= m_Capacity);
					U8* pData = &m_pData[m_Size * m_ElementSize];
					m_Size += num;
					return pData;
				}
	U8*			PopBack() { assert(m_Size > 0); ; return &m_pData[--m_Size * m_ElementSize];  }

	FORCE_INLINE U8& operator [] (S32 i) {assert(i >= 0 && i < this->Size()); return m_pData[m_ElementSize * i]; } /// Random access to vector.
	FORCE_INLINE const U8& operator [] (S32 i) const {assert(i >= 0 && i < this->Size()); return m_pData[m_ElementSize * i]; } ///< Random access to vector.

	// for usage on SPU
	U8*			Fetch();
	U8*			FetchAsync(U32 tag);
	void		Store(U8* pEA);
	void		StoreAsync(U8* pEA, U32 tag);
private:

	S32		m_ElementSize;
	S32		m_Capacity;
	S32		m_Size;
	U8*		m_pData;
};

INLINE ElementVector::ElementVector() : m_pData(NULL), m_ElementSize(-1), m_Capacity(0), m_Size(0)
{
}

INLINE ElementVector::~ElementVector()
{
	if(m_pData)
		delete m_pData;
}

INLINE void ElementVector::Reserve(S32 elementSize, S32 capacity)
{
	if(elementSize != m_ElementSize || capacity != m_Capacity)
	{
		assert(elementSize > 0 && capacity > 0);

		if(m_pData)
			delete m_pData;

		m_ElementSize = elementSize;
		m_Capacity = capacity;

		m_pData = new U8[elementSize * capacity];
	}
}

// a block with a dynamic vector of objects of size N
/*class VectorBlock
{
public:
	VectorBlock() {};

	// increments count and returns pointer to end item
	U8* PushBack()
	{
#ifndef CFG_SPEED
		// if capacity is zero then we've probably run out of memory
		if(m_Element.Capacity() == 0)
			return NULL;
#endif
		m_Element.PushBack();
		return &m_Element.Back();
	};
	
	// returns pointer to particle on end of array and decrements count
	U8* PopBack()
	{
		return m_Element.PopBack();
	}

	// returns number of particles left in this block
	S32 Remaining() const
	{
		return m_Element.Remaining();
	}

	bool Empty() const
	{
		return m_Element.Empty();
	}

	void Reserve(U32 iElementSize, U32 iBlockSize)
	{
		m_Element.Reserve(iElementSize, iBlockSize);
	}
	
	S32 Size() const
	{
		return m_Element.Size();
	}

	void Clear()
	{
		m_Element.Clear();
	}

	void Reset()
	{
		m_Element.Clear();
	}

	ElementVector					m_Element;
};*/

}

#endif // RENDER_DYNAMICEFFECT_BLOCK_H
