#ifndef AFTK_BLIND_DATA_RESOURCE_MANAGER_H
#define AFTK_BLIND_DATA_RESOURCE_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	#ifndef DOXYGEN_SHOULD_SKIP_THIS

	class C_HashKeyOpBlindData
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const C_SharedResource<C_BlindDataNode>* key)
		{
			return key->GetContent().GetHash();
		}

		static bool Equal(const C_SharedResource<C_BlindDataNode>* key1, const C_SharedResource<C_BlindDataNode>* key2)
		{
			if (key1->GetContent() == key2->GetContent())
				return true;

			return false;
		}
	};

	class C_BlindDataResourceManager 
	{
	public:
		///
		///\brief Constructor.
		C_BlindDataResourceManager();
		///
		///\brief Destructor.
		~C_BlindDataResourceManager();
		///If the given resource does not already exist, add a copy of the given resource into the
		///manager and return a constant pointer to its location. If a resource of the given type does
		///already exist, then we increase its reference count.
		///
		C_SharedResource<C_BlindDataNode>*	AddResource(
			const C_BlindDataNode &content);

		void AddResourceNoTest(C_SharedResource<C_BlindDataNode>* pResource);

		///Given a pointer to a resource, look thru the manager and see if we have a resource stored at the
		///given address. If we do, then we decrease its reference count. One this count reaches 0, we delete
		///the resource.
		void DeleteResource(const C_SharedResource<C_BlindDataNode>* pResource);

		///
		///\brief Get the content occupying the given index in the array of resources.
		///@param instance The content instance to retrieve.
		///@return Type The shared resource content.
		const C_BlindDataNode&	GetIthResource(U32 instance) const;

		const C_SharedResource<C_BlindDataNode>* GetIthResourcePtr(U32 instance) const;
		///
		///Return the number of resources held by this manager
		///@return Number of resources.
		U32			GetNumResources() const;

		///
		///\brief Clear the contents of this resource manager.
		void		Clear();

		void		Optimise();

	protected:
		void		Reset();

		void		IncrementIthReferenceCount(U32 i, U32 ammount);
		void		DecrementIthReferenceCount(U32 i, U32 ammount);
		U32			GetCapacity() const
		{
			return m_array.Size();
		}

		C_SharedResource<C_BlindDataNode>*	Exists(const C_BlindDataNode& newContent);
		///
		///Deny access to copy constructor
		C_BlindDataResourceManager(const C_BlindDataResourceManager& rhs);
		///
		///Deny access to asignment operator.
		const C_BlindDataResourceManager& operator=(const C_BlindDataResourceManager& rhs);
		///
		C_SharedResource<C_BlindDataNode>* GetIthResourcePtr(U32 instance);

		C_SharedResource<C_BlindDataNode>* GetResourcePtrByHashIndex(U32 hashIndex);

		Ut::OpenHashSet<C_SharedResource<C_BlindDataNode>*, C_HashKeyOpBlindData>
			m_hashTable;

		Ut::UtVector<Ut::DynamicVector<C_SharedResource<C_BlindDataNode>* > >
			m_array;

		C_SharedResource<C_BlindDataNode>*	m_pLastResourcePointerReturned;

		friend class C_Scene;
		friend class C_NativeScene;
		friend class C_NativeSceneNode;
	};

	inline C_BlindDataResourceManager::C_BlindDataResourceManager()
	{
		m_pLastResourcePointerReturned = 0;
	}

	inline C_BlindDataResourceManager::~C_BlindDataResourceManager()
	{
		Clear();
	}

	inline void C_BlindDataResourceManager::Optimise()
	{
		//m_hashTable.Optimise();
	}

	inline C_SharedResource<C_BlindDataNode>* C_BlindDataResourceManager::Exists(const C_BlindDataNode& newContent)
	{
		//Quite a cheeky cast - content must be first entry in shared resource. This is the only component used
		//for hashing.
		C_SharedResource<C_BlindDataNode>* pDummy = (C_SharedResource<C_BlindDataNode>*)&newContent;

		Ut::OpenHashSet<C_SharedResource<C_BlindDataNode>*, C_HashKeyOpBlindData>::Iterator it =
			m_hashTable.Find(pDummy);
		if (it != m_hashTable.End())
		{
			//We found an existing record.
			return it->GetKey();
		}

		return 0;
	}

	inline C_SharedResource<C_BlindDataNode>* C_BlindDataResourceManager::AddResource(
		const C_BlindDataNode &newContent)
	{
		C_SharedResource<C_BlindDataNode>* pSharedResource = Exists(newContent);
		if (!pSharedResource)
		{
			pSharedResource = new C_SharedResource<C_BlindDataNode>();
			pSharedResource->SetContent(newContent);

			Ut::Pair<Ut::OpenHashSet<C_SharedResource<C_BlindDataNode>*, C_HashKeyOpBlindData>::Iterator, bool>
				result = m_hashTable.Insert(pSharedResource);
			m_array.PushBack(pSharedResource);
	
			AFTK_ASSERT(m_array.Size() > 0);
			pSharedResource->SetIndex((U32)m_array.Size()-1);

			//If resource added...
			if (!result.second)
			{
				LOGERROR(g_aftkMainLogger, "Blind data insertion failed.");
			}

			//Update our resource cache
			m_pLastResourcePointerReturned = pSharedResource;
		}

		AFTK_ASSERT(m_array.Size() == m_hashTable.Size());
		if (m_hashTable.Size() != m_array.Size())
		{
			LOGERROR(g_aftkMainLogger, "Hash-array mismatch exiting AddResource");
		}
		return pSharedResource;
	}

	inline void C_BlindDataResourceManager::AddResourceNoTest(C_SharedResource<C_BlindDataNode>* pResource)
	{
		m_hashTable.Insert(pResource);
		m_array.PushBack(pResource);
		//Set resource index.
		pResource->SetIndex((U32)m_array.Size()-1);
		AFTK_ASSERT(m_hashTable.Size() == m_array.Size());
		if (m_hashTable.Size() != m_array.Size())
		{
			LOGERROR(g_aftkMainLogger, "Hash-array mismatch exiting AddResourceNoTest");
		}
	}

	inline void C_BlindDataResourceManager::DeleteResource(const C_SharedResource<C_BlindDataNode>* pResource)
	{
		AFTK_ASSERT(m_array.Size() == m_hashTable.Size());

		if (!pResource)
			return;

		Ut::OpenHashSet<C_SharedResource<C_BlindDataNode>*, C_HashKeyOpBlindData>::Iterator it = 
			m_hashTable.Find(const_cast<C_SharedResource<C_BlindDataNode>* >(pResource));
		if (it != m_hashTable.End())
		{
			C_SharedResource<C_BlindDataNode>* pCurrentResource = it->GetKey();
			// this manager _DOES_ contain a resource at the given address
			// set the contents of this resource
			U32 referenceCount = pCurrentResource->GetReferenceCount();
			if (referenceCount < 2)
			{
				m_hashTable.Erase(it);
				m_array.EraseByIndex(pCurrentResource->GetIndex());
				//Shuffle indices down.
				for (U32 i = pCurrentResource->GetIndex(); i < (U32)m_array.Size(); i++)
					m_array[i]->SetIndex(i);

				// free the memory taken by the unwanted resource
				delete pCurrentResource;
				// remove dead resource from cache if needed
				if (m_pLastResourcePointerReturned = pCurrentResource)
					m_pLastResourcePointerReturned = 0;
			} 
		}
		AFTK_ASSERT(m_array.Size() == m_hashTable.Size());
		if (m_hashTable.Size() != m_array.Size())
		{
			LOGERROR(g_aftkMainLogger, "Hash-array mismatch exiting DeleteResource");
		}
	}

	inline void C_BlindDataResourceManager::Clear()
	{
		for (
			Ut::UtVector<Ut::DynamicVector<C_SharedResource<C_BlindDataNode>* > >::Iterator it = m_array.Begin();
			it != m_array.End();
			++it)
		{
			C_SharedResource<C_BlindDataNode>* pResource = *it;
			delete pResource;
		}

		m_hashTable.Clear();
		m_array.Clear();

		m_pLastResourcePointerReturned = 0;
	}

	inline const C_BlindDataNode& C_BlindDataResourceManager::GetIthResource(U32 instance) const
	{
		return m_array[instance]->GetContent();
	}

	inline U32 C_BlindDataResourceManager::GetNumResources() const
	{
		return (U32)m_hashTable.Size();
	}

	inline const C_SharedResource<C_BlindDataNode>* C_BlindDataResourceManager::GetIthResourcePtr(U32 instance) const
	{
		return m_array[instance];
	}

	inline C_SharedResource<C_BlindDataNode>* C_BlindDataResourceManager::GetIthResourcePtr(U32 instance)
	{
		return m_array[instance];
	}

	inline void C_BlindDataResourceManager::IncrementIthReferenceCount(U32 i, U32 ammount)
	{
		GetIthResourcePtr(i)->IncrementReferenceCount(ammount);
	}

	inline void C_BlindDataResourceManager::DecrementIthReferenceCount(U32 i, U32 ammount)
	{
		GetIthResourcePtr(i)->DecrementReferenceCount(ammount);
	}

	#endif
}

#endif
