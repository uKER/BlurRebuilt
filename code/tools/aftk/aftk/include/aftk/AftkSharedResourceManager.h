#ifndef AFTK_SHARED_RESOURCE_MANAGER_H
#define AFTK_SHARED_RESOURCE_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	#ifndef DOXYGEN_SHOULD_SKIP_THIS

	///Manager class that allows the creation, sharing and
	///deletion of templated resources.
	template<class Type>
	class C_SharedResourceManager 
	{
	public:
		///
		///\brief Constructor.
		C_SharedResourceManager();
		///
		///\brief Destructor.
		~C_SharedResourceManager();
		///If the given resource does not already exist, add a copy of the given resource into the
		///manager and return a constant pointer to its location. If a resource of the given type does
		///already exist, then we increase its reference count.
		///
		const		C_SharedResource<Type>*	AddResource(const Type &content);

		void AddResourceNoTest(C_SharedResource<Type>* pResource);

		///Given a pointer to a resource, look thru the manager and see if we have a resource stored at the
		///given address. If we do, then we decrease its reference count. One this count reaches 0, we delete
		///the resource.
		void		DeleteResource(const C_SharedResource<Type>* pResource);
		// As above, except that we examine the  _contents_ of the resource, instead of its address, to
		// identify it.
		// void		DeleteResource(const Type &resource); TODO
		///
		///\brief Get the content occupying the given index in the array of resources.
		///@param instance The content instance to retrieve.
		///@return Type The shared resource content.
		const Type&	GetResource(U32 instance) const;
		///
		///Get the shared resource content associated with the given shared resource pointer.
		///@param pResource Shared resource pointer.
		///@return Type The shared resource content.
		const Type&	GetResource(const C_SharedResource<Type>* pResource) const;
		///
		const C_SharedResource<Type>* GetResourcePtr(U32 instance) const;
		///
		///Return the number of resources held by this manager
		///@return Number of resources.
		U32			GetNumResources() const;
		///
		///Set the value of the resource at the given memory address
		///@param pResource Shared resource pointer.
		///@param content new shared resource content.
		void		SetResource(const C_SharedResource<Type>* pResource, const Type &content);

		///
		///\brief Clear the contents of this resource manager.
		void		Clear();

	protected:
		void		Reset();
		void		IncrementReferenceCount(U32 instance, U32 ammount = 1);
		void		DecrementReferenceCount(U32 instance, U32 ammount = 1);
		C_SharedResource<Type>*	Exists(const Type& newContent);
		///
		///Deny access to copy constructor
		C_SharedResourceManager(const C_SharedResourceManager& rhs);
		///
		///Deny access to asignment operator.
		const C_SharedResourceManager<Type>& operator=(const C_SharedResourceManager& rhs);
		///
		C_SharedResource<Type>* GetResourcePtr(U32 instance);
		///
		///We store pointers to our shared resources within an STL Set which is a sorted container.
		///This makes it easy for us to find a given pointer, or to determine if a given resource
		///pointer exists
		Ut::UtSet<C_SharedResource<Type>* > m_sharedResources;
		///
		///The following vector is present so that we can treat the shared resources as an array.
		Ut::UtVector< Ut::DynamicVector<C_SharedResource<Type>*> > m_sharedResourcesArray;

		C_SharedResource<Type>*	m_pLastResourcePointerReturned;
		Type						m_emptyType;

		friend class C_Scene;
	};

	template<class Type>
	C_SharedResourceManager<Type>::C_SharedResourceManager()
	{
		m_pLastResourcePointerReturned = 0;
	}

	template<class Type>
	C_SharedResourceManager<Type>::~C_SharedResourceManager()
	{
		Clear();
	}

	template<class Type>
	C_SharedResource<Type>* C_SharedResourceManager<Type>::Exists(const Type& newContent)
	{
		if ((U32)m_sharedResourcesArray.Size() > 0)
		{
			if (m_pLastResourcePointerReturned != 0)
			{
				const Type* pExistingContent = &m_pLastResourcePointerReturned->GetContent();
				if (*pExistingContent == newContent)
				{
					//Use caching to speed up material asignment.
					return m_pLastResourcePointerReturned;
				}
			}
		}
		else
		{
			m_pLastResourcePointerReturned = 0;
		}

		// STL Set iterator.
		typename Ut::UtSet<C_SharedResource<Type>*>::ConstIterator it;
		//it;
		for (it = m_sharedResources.Begin(); it != m_sharedResources.End(); it++) 
		{
			C_SharedResource<Type>* pResource = *it;
			const Type* pExistingContent = &pResource->GetContent();
			if (*pExistingContent == newContent) 
			{
				return pResource;
				//Resource not added
			}
		}

		return 0;
	}

	template<class Type>
	const C_SharedResource<Type>* C_SharedResourceManager<Type>::AddResource(const Type &newContent)
	{
		C_SharedResource<Type>* pSharedResource = Exists(newContent);

		if (!pSharedResource)
		{
			pSharedResource = new C_SharedResource<Type>();
			pSharedResource->SetContent(newContent);

			pSharedResource->SetIndex((U32)m_sharedResourcesArray.Size());

			//Add the new shared resource to our STL set.
			m_sharedResources.insert(pSharedResource);
			//Maintain a linear array of resource pointers so that we can index them directly if needed
			m_sharedResourcesArray.PushBack(pSharedResource);
			//Resource added

			//Update our resource cache
			m_pLastResourcePointerReturned = pSharedResource;
		}
		return pSharedResource;
	}


	template<class Type>
	void C_SharedResourceManager<Type>::AddResourceNoTest(C_SharedResource<Type>* pResource)
	{
		pResource->SetIndex((U32)m_sharedResourcesArray.Size());

		//Add the new shared resource to our STL set.
		m_sharedResources.insert(pResource);
		//Maintain a linear array of resource pointers so that we can index them directly if needed
		m_sharedResourcesArray.PushBack(pResource);
		//Resource added

		//Update our resource cache
		m_pLastResourcePointerReturned = pResource;
	}

	template<class Type>
	void C_SharedResourceManager<Type>::DeleteResource(const C_SharedResource<Type>* pResource)
	{
		if (!pResource)
		{
			return;
		}

		typename Ut::UtSet<C_SharedResource<Type>*>::Iterator it;
		for (it = m_sharedResources.begin(); it != m_sharedResources.End(); it++) 
		{
			C_SharedResource<Type>* pCurrentResource = *it;
			if (pCurrentResource == pResource) 
			{
				// this manager _DOES_ contain a resource at the given address
				// set the contents of this resource
				U32 referenceCount = pCurrentResource->GetReferenceCount();
				if (referenceCount < 2)
				{
					// remove from array;
					m_sharedResourcesArray.EraseByIndex(pCurrentResource->GetIndex());

					// We've removed an element from our shared resource array. Therefore, our instance
					// numbers are no longer valid! We need to shuffle down the instance values of the
					// shared resources we contain.
					{
						typename Ut::UtSet<C_SharedResource<Type>*>::Iterator shuffleIt;
						shuffleIt = m_sharedResources.Begin();
						do
						{
							C_SharedResource<Type>* pResourceToPossiblyShuffle = *shuffleIt;
							//If the instance number of this resource is greater than the instance
							//of the node we are to remove then it must be shuffled down.

							if (pResourceToPossiblyShuffle->GetIndex() > pCurrentResource->GetIndex())
							{
								U32 newInstance = pResourceToPossiblyShuffle->GetIndex();
								newInstance--;
								pResourceToPossiblyShuffle->SetIndex(newInstance);
							}

							shuffleIt++;
						}
						while (shuffleIt != m_sharedResources.End());
					}
	                
					// free the memory taken by the unwanted resource
					delete pCurrentResource;
					// delete the now invalid pointer from the manager
					m_sharedResources.erase(it);
					// remove dead resource from cache if needed
					if (m_pLastResourcePointerReturned = pCurrentResource)
					{
						m_pLastResourcePointerReturned = 0;
					}
					return;
				} 
				else 
				{
					return;
				}
			}
		}
	}

	template<class Type>
	void C_SharedResourceManager<Type>::Clear()
	{
		typename Ut::UtSet<C_SharedResource<Type>*>::ConstIterator it;
		for (it = m_sharedResources.Begin(); it != m_sharedResources.End(); it++) 
		{
			C_SharedResource<Type>* pResource = *it;
			if (pResource != NULL) 
			{
				delete pResource;
			}
		}
		m_sharedResources.Clear();
		m_sharedResourcesArray.Clear();
		m_pLastResourcePointerReturned = 0;
	}

	template<class Type>
	const Type& C_SharedResourceManager<Type>::GetResource(U32 instance) const
	{
		return m_sharedResourcesArray[instance]->GetContent();
	}

	template<class Type>
	const Type& C_SharedResourceManager<Type>::GetResource(const C_SharedResource<Type>* pResource) const
	{
		typename Ut::UtSet<C_SharedResource<Type>*>::ConstIterator it;
		it = m_sharedResources.find((C_SharedResource<Type>*)pResource);
		C_SharedResource<Type>* pIteratorContents = *it;
		if (pIteratorContents != NULL) 
		{
			return pIteratorContents->GetContent();
		}
		else
		{
			return m_emptyType;
		}
	}

	template<class Type>
	U32 C_SharedResourceManager<Type>::GetNumResources() const
	{
		return (U32)m_sharedResources.size();
	}

	template<class Type>
	void C_SharedResourceManager<Type>::SetResource(const C_SharedResource<Type>* pResource, const Type &content)
	{
		U32 i;
		for (i = 0; i < m_sharedResources.size(); i++) {
			if (m_sharedResources[i] == pResource) {
				// this manager _DOES_ contain a resource at the given address
				// set the contents of this resource by performing a bitwise (shallow) copy
				this->m_sharedResource[i]->SetContent(content);
			}
		}
	}

	template<class Type>
	const C_SharedResource<Type>* C_SharedResourceManager<Type>::GetResourcePtr(U32 instance) const
	{
		return m_sharedResourcesArray[instance];
	}

	template<class Type>
	C_SharedResource<Type>* C_SharedResourceManager<Type>::GetResourcePtr(U32 instance)
	{
		return m_sharedResourcesArray[instance];
	}

	template<class Type>
	void C_SharedResourceManager<Type>::IncrementReferenceCount(U32 instance, U32 ammount)
	{
		m_sharedResourcesArray[instance]->IncrementReferenceCount(ammount);
	}

	template<class Type>
	void C_SharedResourceManager<Type>::DecrementReferenceCount(U32 instance, U32 ammount)
	{
		m_sharedResourcesArray[instance]->DecrementReferenceCount(ammount);
	}


	#endif
}
#endif
