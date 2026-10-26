#ifndef AFTK_SHARED_RESOURCE_MAPPING_H
#define AFTK_SHARED_RESOURCE_MAPPING_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	template <class T>
	class C_SharedResourceMapping
	{
	public:
		C_SharedResourceMapping()
		{
			m_index = 0;
			m_pSharedResource = 0;
			m_referenceCount = 0;
		}

		~C_SharedResourceMapping()
		{
		}

		C_SharedResourceMapping(const C_SharedResourceMapping& rhs)
		{
			*this = rhs;
		}

		const C_SharedResourceMapping& operator=(const C_SharedResourceMapping& rhs)
		{
			m_index = rhs.m_index;
			m_pSharedResource = rhs.m_pSharedResource;
			m_referenceCount = rhs.m_referenceCount;

			return *this;
		}

		bool operator!=(const C_SharedResourceMapping& rhs) const
		{
			if (
				(m_index != rhs.m_index) ||
				(m_pSharedResource != rhs.m_pSharedResource) ||
				(m_referenceCount != rhs.m_referenceCount)
				)
			{
				return true;
			}

			return false;
		}


		bool operator==(const C_SharedResourceMapping& rhs) const
		{
			return !((*this)!=rhs);
		}

		void SetIndex(U16 index)
		{
			AFTK_ASSERT(m_pSharedResource);
			m_index = index;
		}

		void SetSharedResourcePtr(const C_SharedResource< T >* pResource)
		{
			m_pSharedResource = pResource;
		}

		void SetReferenceCount(U32 refCount)
		{
			AFTK_ASSERT(m_pSharedResource);
			m_referenceCount = refCount;
		}

		void IncrementReferenceCount() 
		{
			AFTK_ASSERT(m_pSharedResource);
			m_referenceCount++;
		}

		void DecrementReferenceCount() 
		{
			AFTK_ASSERT(m_pSharedResource);
			if (m_referenceCount != 0)
			{
				m_referenceCount--;
			}
		}

		U16 GetIndex() const
		{
			AFTK_ASSERT(m_pSharedResource);
			return m_index;
		}

		const C_SharedResource< T >*	GetSharedResourcePtr() const
		{
			return m_pSharedResource;
		}

		U32 GetReferenceCount() const
		{
			AFTK_ASSERT(m_pSharedResource);
			return m_referenceCount;
		}

	private:
		U16								m_index;
		const C_SharedResource< T >*	m_pSharedResource;
		U32								m_referenceCount;
	};
}

#endif
