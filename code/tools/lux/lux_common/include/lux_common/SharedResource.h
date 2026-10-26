/******************************************************************************/
// Name: SharedResource.h
//
// Author: Ken MacLeod
// 
// Description: 
//
// History: created 4rd March 2004
//
// Copyright: 2004 Bizarre Creations
//
// Notes: N/A
//
/******************************************************************************/

#ifndef SHARED_RESOURCE_H
#define SHARED_RESOURCE_H

using namespace LPVS

template<class Type>
class SharedResource
{
public:
	SharedResource()
	{
		Reset();
	}

	SharedResource(const SharedResource& SharedResource)
	{
		*this = SharedResource;
	}

	virtual ~SharedResource() 
	{
		Clear();
	}

	const SharedResource& operator=(const SharedResource& SharedResource)
	{
		Clear();
		m_referenceCount = SharedResource.m_referenceCount;
		m_instance = SharedResource.m_instance;
		m_content = SharedResource.m_content;
		return *this;
	}

	void Clear()
	{
		Reset();
	}

	void SetContent(const Type &content)
	{
		m_content = content;
	}

	const Type& GetContent() const 
	{
		return m_content;
	}

	U32 GetReferenceCount() const 
	{
		return m_referenceCount;
	}

	///
	// Get the resource instance number. In other words its position in the array
	// of resources.
	U32 GetInstance() const
	{
		return m_instance;
	}

	void SetInstance(U32 instance) 
	{
		m_instance = instance;
	}

	void SetReferenceCount(U32 count) 
	{
		m_referenceCount = count;
	}

	void IncrementReferenceCount() 
	{
		m_referenceCount++;
	}

	void DecrementReferenceCount() 
	{
		if (m_referenceCount != 0) {
			m_referenceCount--;
		}
	}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
protected:
	void Reset()
	{
		m_referenceCount = 0;
		m_instance = 0;
	}

	Type	m_content;
	U32		m_referenceCount;
	U32		m_instance;
#endif
};

#endif