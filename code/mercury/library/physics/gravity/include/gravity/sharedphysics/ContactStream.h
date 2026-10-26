#ifndef CONTACTSTREAM_H
#define CONTACTSTREAM_H

#include "gravity/sharedphysics/GraphNode.h"
#include "gravity/sharedphysics/AllocationManager.h"
#include "core/threads/Mutex.h"
#include "core/threads/Atomic.h"
#include "gravity/sharedphysics/select.h"

namespace SharedPhysics
{

class C_Collider;
class C_CollisionResultDB;

enum E_ContactStreamDataValidity
{
	E_NullData = 0,
	E_ValidData = 0x1234,
	E_DeletedData = 0x4321,
	E_Force_Dword = 0xffffffff
};

struct ALIGN16 C_ParticleContactPair
{
	E_ContactStreamDataValidity m_ValidData; //A magic number, which tells us the data is valid 
											 //and a valid result is stored here (can be set to an 
											 //invalid number to disable the contact)
	SharedPhysics::C_GraphNode* m_pParticlePtr;
	SharedPhysics::C_GraphNode* m_pNodePtr;
	U32 m_NumContacts;
};

struct ALIGN16 C_ParticleContactStorage
{
	E_ContactStreamDataValidity m_ValidData; //A magic number, which tells us the data is valid 
											 //and a valid result is stored here (can be set to an 
											 //invalid number to disable the contact)
	SharedPhysics::C_GraphNode* m_pParticlePtr;
	SharedPhysics::C_GraphNode* m_pNodePtr;
	MathsStorage::Float3 m_Contact;
	MathsStorage::Float3 m_EscapeVector;
	MathsStorage::Float3 m_TriangleNormal;
	F32 m_Penetration;
	F32 m_Stiffness;
	U32 m_Index;
	U32 m_Type;
};


struct C_ContactStreamResult
{
	C_GraphNode* m_pNodeA;
	C_GraphNode* m_pNodeB;
	C_Collider* m_pCollider;
	F32 m_ObjectAMassScale, m_ObjectBMassScale;
	F32 m_ObjectAInertiaScale, m_ObjectBInertiaScale;
	F32 m_BaumgarteOverride;
	E_ContactStreamDataValidity m_ValidData; //A magic number, which tells us the data is valid 
											 //and a valid result is stored here (can be set to an 
											 //invalid number to disable the contact)
	U16 m_OffsetToEnd;						 //An offset index to the end of this collision result
	U8 m_NumElements;						 //The number of elements in this result
	U8 m_TotalContacts;

	NOALIAS inline void Invalidate()
	{
		m_ValidData = E_DeletedData;
	}
};

struct C_ContactStreamPatch
{
	MathsStorage::Float3 m_Normal;
	E_ContactStreamDataValidity m_ValidData; //A magic number, which tells us the data is valid 
											 //and a valid result is stored here (can be set to an 
											 //invalid number to disable the contact)
	U16 m_OffsetToEnd;						 //An offset index to the end of this collision result
	U8 m_NumElements;						 //The number of elements in this result

	NOALIAS inline void Invalidate()
	{
		m_ValidData = E_DeletedData;
	}
};

struct C_ContactStreamPoint
{
	MathsStorage::Float3 m_ContactPoint;
	F32 m_PenetrationDepth;
	U32 m_HashValue;
	U32 m_PhysicsMaterialIndices;
	U32 m_EffectMaterialIndices;
	C_ConstraintPoint* m_pConstraint;
	E_ContactStreamDataValidity m_ValidData;

	NOALIAS inline void Invalidate()
	{
		m_ValidData = E_DeletedData;
	}

	inline U16 IndexA()
	{
		return m_PhysicsMaterialIndices >> 16;
	}

	inline U16 IndexB()
	{
		return m_PhysicsMaterialIndices & 0xffff;
	}
};

class ALIGN128 C_ContactStream
{
	friend class C_ContactStreamIterator;
	friend class C_ContactStreamWriter;
public:

	void* operator new(size_t size)
	{
		return BizAlignedMalloc(size, 128);
	}

	void* operator new [] (size_t size)
	{
		return BizAlignedMalloc(size, 128);
	}

	void operator delete( void* ptr )
	{
		BizAlignedFree(ptr);
	}

	void operator delete [] (void* ptr)
	{
		BizAlignedFree(ptr);
	}
	C_ContactStream() : m_pStream(NULL), m_pCurrentWritePoint(NULL), m_pStreamEnd(NULL)
	{
		m_pPPUPtr = this;
	}

	C_ContactStream(U32 size) : m_Size(size)
	{
		m_pPPUPtr = this;

#ifndef PLATFORM_SPU
		m_pCurrentWritePoint = m_pStream = (U8*)BizAlignedMalloc(m_Size + sizeof(SharedPhysics::C_ContactStreamResult), 16);
		m_pStreamEnd = m_pCurrentWritePoint + size;
		C_ContactStreamResult* pResultScrap((C_ContactStreamResult*)m_pStream);
		pResultScrap->m_ValidData = E_NullData;
#endif
	}

	C_ContactStream(U8* pStream, U32 size) : m_Size(size)
	{
		m_pPPUPtr = this;
		m_pCurrentWritePoint = m_pStream = pStream;
		m_pStreamEnd = m_pCurrentWritePoint + size;
		C_ContactStreamResult* pResultScrap((C_ContactStreamResult*)m_pStream);
		pResultScrap->m_ValidData = E_NullData;
	}

	~C_ContactStream();

	NOALIAS inline void Reset()
	{
		m_pCurrentWritePoint = m_pStream;
		C_ContactStreamResult* pResultScrap((C_ContactStreamResult*)m_pStream);
		pResultScrap->m_ValidData = E_NullData;
	}

	//Atomically locks memory by incrementing the pointer. Then we need to write data to this memory!
	U8* LockMemory(const U32 size);

	//Same as above, but the result stream is in Spu memory so doesn't involve DMAs or thread contention
	U8* LockMemorySpu(const U32 size);

	public:

	ALIGN128 U8* m_pCurrentWritePoint;
	U32 m_Size;
	U8* m_pStream;
	U8* m_pStreamEnd;
	C_ContactStream* m_pPPUPtr;

private:
	C_ContactStream& operator = (const C_ContactStream& other);

	C_ContactStream(const C_ContactStream& other);
};

class C_ConstContactStreamPtr
{
	friend class C_ContactStreamIterator;
	friend class C_SingleContactIterator;
public:

	C_ConstContactStreamPtr() : m_pStream(NULL), m_pCurrentReadPoint(NULL)
	{
	}

	C_ConstContactStreamPtr(C_ContactStream* pStream, U8* pCurrent) : m_pStream(pStream), m_pCurrentReadPoint(pCurrent)
	{
	}

	C_ConstContactStreamPtr(const C_ConstContactStreamPtr& other) : m_pStream(other.m_pStream), m_pCurrentReadPoint(other.m_pCurrentReadPoint)
	{
	}

	C_ConstContactStreamPtr(C_ContactStream& stream) : m_pStream(&stream), m_pCurrentReadPoint(stream.m_pCurrentWritePoint)
	{
	}

	void Reset()
	{
		m_pStream = NULL;
		m_pCurrentReadPoint = NULL;
	}

	NOALIAS inline C_ConstContactStreamPtr& operator = (const C_ConstContactStreamPtr& ptr)
	{
		m_pStream = ptr.m_pStream;
		m_pCurrentReadPoint = ptr.m_pCurrentReadPoint;
		return *this;
	}

	NOALIAS inline C_ConstContactStreamPtr& operator = (C_ContactStream& stream)
	{
		m_pStream = &stream;
		m_pCurrentReadPoint = stream.m_pCurrentWritePoint;
		return *this;
	}

	NOALIAS inline U8* ReadPoint() const
	{
		return m_pCurrentReadPoint;
	}
	C_ContactStream* m_pStream;
	U8* m_pCurrentReadPoint; //Can move ptr but not modify the contents - read only..
};

NOALIAS inline void EncodeContacts(C_Collider* result, C_ContactStream& stream);
NOALIAS inline void EncodeContacts(C_CollisionResultDB& result, C_ContactStream& stream);


};

#endif
