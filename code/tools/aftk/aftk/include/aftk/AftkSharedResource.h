#ifndef AFTK_SHARED_RESOURCE_H
#define AFTK_SHARED_RESOURCE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_BlindDataResourceManager;
	template<typename>
	class C_NamedResourceManager;

	template<class Type>
	class C_SharedResource
	{
	public:
		///\brief Constructor.
		///
		C_SharedResource()
		{
			Clear();
		}

		///\brief Copy constructor
		///
		///@param rhs The resource to copy.
		C_SharedResource(const C_SharedResource& rhs)
		{
			*this = rhs;
		}

		///\brief Destructor.
		///
		~C_SharedResource() 
		{
			Clear();
		}

		///\brief Asignment operator.
		///
		///@param rhs The resource to copy.
		const C_SharedResource& operator=(const C_SharedResource& rhs)
		{
			Clear();
			m_referenceCount = rhs.m_referenceCount;
			m_index = rhs.m_index;
			m_content = rhs.m_content;
			return *this;
		}

		///\brief Equality test operator.
		///
		bool operator==(const C_SharedResource& rhs) const
		{
			return m_content == rhs.m_content;
		}

		///\brief Clears the contents of this class to their initial state.
		///
		void Clear()
		{
			m_referenceCount = 0;
			m_index = 0;
		}

		///\brief Sets the content of this resource container.
		///
		///@param content The content.
		void SetContent(const Type &content)
		{
			m_content = content;
		}

		///\brief Returns the content of this resource container.
		///
		///@return The content.
		const Type& GetContent() const 
		{
			return m_content;
		}

		///\brief Returns the content of this resource container.
		///
		///@return The content.
		Type& GetContent() 
		{
			return m_content;
		}

		///\brief Returns the reference count for this shared resource.
		///
		///@return the shared resource.
		U32 GetReferenceCount() const 
		{
			return m_referenceCount;
		}

		///\brief Returns this resources position in the array of resources in the relevant 
		///resource manager.
		///
		///@return The resource index.
		U32 GetIndex() const
		{
			return m_index;
		}

		///\brief Sets the index of this resource in the array of resources in the relevant
		///resource manager.
		///
		///@param instance The resource index.
		void SetIndex(U32 instance) 
		{
			m_index = instance;
		}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		///\brief Sets the reference count for this resource.
		///
		///@param count The reference count.
		void SetReferenceCount(U32 count) 
		{
			m_referenceCount = count;
		}

		///\brief Increments the reference count for this resource.
		///
		///@param ammount The ammount to increase the reference count by.
		void IncrementReferenceCount(U32 ammount = 1) 
		{
			m_referenceCount += ammount;
		}

		///\brief Decrements the reference count for this resource.
		///
		///@param ammount The ammount to decrease the reference count by.
		void DecrementReferenceCount(U32 ammount = 1) 
		{
			if (m_referenceCount != 0) 
			{
				m_referenceCount -= ammount;
			}
		}

		Type	m_content;
		U32		m_referenceCount;
		U32		m_index;

		FRIEND_CLASS C_BlindDataResourceManager;
		template<typename> friend class C_NamedResourceManager;
	#endif
	};
}

#endif
