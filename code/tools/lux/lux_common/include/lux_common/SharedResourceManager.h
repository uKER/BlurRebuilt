#ifndef SHARED_RESOURCE_MANAGER_H
#define SHARED_RESOURCE_MANAGER_H

namespace LPVS
{
	template<class Type>
	class SharedResourceManager 
	{
	public:
		///
		///\brief Constructor.
		SharedResourceManager();
		///
		///\brief Destructor.
		virtual ~SharedResourceManager();
		///If the given resource does not already exist, add a copy of the given resource into the
		///manager and return a constant pointer to its location. If a resource of the given type does
		///already exist, then we increase its reference count.
		///
		void		SetIncreaseReferenceCountOnAddFlag(bool flag);
		///
		const		SharedResource<Type>*	AddResource(const Type &content);
		///Given a pointer to a resource, look thru the manager and see if we have a resource stored at the
		///given address. If we do, then we decrease its reference count. One this count reaches 0, we delete
		///the resource.
		void		DeleteResource(const SharedResource<Type>* pResource);
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
		const Type&	GetResource(const SharedResource<Type>* pResource) const;
		///
		const SharedResource<Type>* GetResourcePtr(U32 instance) const;
		///
		///Return the number of resources held by this manager
		///@return Number of resources.
		U32			GetNumResources() const;
		///
		///Set the value of the resource at the given memory address
		///@param pResource Shared resource pointer.
		///@param content new shared resource content.
		void		SetResource(const SharedResource<Type>* pResource, const Type &content);
		///
		///\brief Clear the contents of this resource manager.
		void		Clear();

	protected:
		void		Reset();
		void		IncrementReferenceCount(U32 instance);
		void		DecrementReferenceCount(U32 instance);
		///
		///Deny access to copy constructor
		SharedResourceManager(const SharedResourceManager& rhs);
		///
		///Deny access to asignment operator.
		const SharedResourceManager<Type>& operator=(const SharedResourceManager& rhs);
		///
		SharedResource<Type>* GetResourcePtr(U32 instance);
		///
		///We store pointers to our shared resources within an STL Set which is a sorted container.
		///This makes it easy for us to find a given pointer, or to determine if a given resource
		///pointer exists
		std::set<SharedResource<Type>*> m_sharedResources;
		///
		///The following vector is present so that we can treat the shared resources as an array.
		Ut::UtVector< Ut::DynamicVector<SharedResource<Type>*> > m_sharedResourcesArray;

		SharedResource<Type>*	m_pLastResourcePointerReturned;
		Type						m_emptyType;
		bool						m_increaseReferenceCountOnAddFlag;

		friend class Scene;
	};

	template<class Type>
	SharedResourceManager<Type>::SharedResourceManager()
	{
		m_pLastResourcePointerReturned = 0;
		m_increaseReferenceCountOnAddFlag = true;
	}

	template<class Type>
	SharedResourceManager<Type>::~SharedResourceManager()
	{
		Clear();
	}

	template<class Type>
	void SharedResourceManager<Type>::SetIncreaseReferenceCountOnAddFlag(bool flag)
	{
		m_increaseReferenceCountOnAddFlag = flag;
	}

	template<class Type>
	const SharedResource<Type>* SharedResourceManager<Type>::AddResource(const Type &newContent)
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

		SharedResource<Type>* pSharedResource;
		bool found = false;

		// STL Set iterator.
		std::set<SharedResource<Type>*>::const_iterator it;
		for (it = m_sharedResources.begin(); it != m_sharedResources.end(); it++) 
		{
			SharedResource<Type>* pResource = *it;
			const Type* pExistingContent = &pResource->GetContent();
			if (*pExistingContent == newContent) 
			{
				if (m_increaseReferenceCountOnAddFlag)
				{
	 				pResource->IncrementReferenceCount();
				}
				pSharedResource = pResource;
				found = true;
				//Resource not added
			}
		}
		if (found == false) 
		{
			pSharedResource = new SharedResource<Type>();
			pSharedResource->SetContent(newContent);

			pSharedResource->SetInstance((U32)m_sharedResourcesArray.Size());
			if (m_increaseReferenceCountOnAddFlag)
			{
	 			pSharedResource->IncrementReferenceCount();
			}

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
	void SharedResourceManager<Type>::DeleteResource(const SharedResource<Type>* pResource)
	{
		std::set<SharedResource<Type>*>::iterator it;
		for (it = m_sharedResources.begin(); it != m_sharedResources.end(); it++) 
		{
			SharedResource<Type>* pCurrentResource = *it;
			if (pCurrentResource == pResource) 
			{
				// this manager _DOES_ contain a resource at the given address
				// set the contents of this resource
				U32 referenceCount = pCurrentResource->GetReferenceCount();
				if (referenceCount < 2)
				{
					// remove from array;
					m_sharedResourcesArray.Erase(pCurrentResource->GetInstance());

					// We've removed an element from our shared resource array. Therefore, our instance
					// numbers are no longer valid! We need to shuffle down the instance values of the
					// shared resources we contain.
					{
						std::set<SharedResource<Type>*>::iterator shuffleIt;
						shuffleIt = m_sharedResources.begin();
						do
						{
							SharedResource<Type>* pResourceToPossiblyShuffle = *shuffleIt;
							//If the instance number of this resource is greater than the instance
							//of the node we are to remove then it must be shuffled down.

							if (pResourceToPossiblyShuffle->GetInstance() > pCurrentResource->GetInstance())
							{
								U32 newInstance = pResourceToPossiblyShuffle->GetInstance();
								newInstance--;
								pResourceToPossiblyShuffle->SetInstance(newInstance);
							}

							shuffleIt++;
						}
						while (shuffleIt != m_sharedResources.end());
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
				} else 
				{
					pCurrentResource->DecrementReferenceCount();
					return;
				}
			}
		}
	}

	template<class Type>
	void SharedResourceManager<Type>::Clear()
	{
		std::set<SharedResource<Type>*>::const_iterator it;
		for (it = m_sharedResources.begin(); it != m_sharedResources.end(); it++) 
		{
			SharedResource<Type>* pResource = *it;
			if (pResource != NULL) 
			{
				delete pResource;
			}
		}
		m_sharedResources.clear();
		m_sharedResourcesArray.Clear();
		m_pLastResourcePointerReturned = 0;
	}

	template<class Type>
	const Type& SharedResourceManager<Type>::GetResource(U32 instance) const
	{
		return m_sharedResourcesArray[instance]->GetContent();
	}

	template<class Type>
	const Type& SharedResourceManager<Type>::GetResource(const SharedResource<Type>* pResource) const
	{
		std::set<SharedResource<Type>*>::const_iterator it;
		it = m_sharedResources.find((SharedResource<Type>*)pResource);
		SharedResource<Type>* pIteratorContents = *it;
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
	U32 SharedResourceManager<Type>::GetNumResources() const
	{
		return (U32)m_sharedResources.size();
	}

	template<class Type>
	void SharedResourceManager<Type>::SetResource(const SharedResource<Type>* pResource, const Type &content)
	{
		U32 i;
		for (i = 0; i < m_sharedResources.size(); i++) {
			if (m_sharedResources[i] == pResource) {
				// this manager _DOES_ contain a resource at the given address
				// set the contents of this resource by performing a bitwise (shallow) copy
				m_sharedResource[i]->SetContent(content);
			}
		}
	}

	template<class Type>
	const SharedResource<Type>* SharedResourceManager<Type>::GetResourcePtr(U32 instance) const
	{
		return m_sharedResourcesArray[instance];
	}

	template<class Type>
	SharedResource<Type>* SharedResourceManager<Type>::GetResourcePtr(U32 instance)
	{
		return m_sharedResourcesArray[instance];
	}

	template<class Type>
	void SharedResourceManager<Type>::IncrementReferenceCount(U32 instance)
	{
		m_sharedResourcesArray[instance]->IncrementReferenceCount();
	}

	template<class Type>
	void SharedResourceManager<Type>::DecrementReferenceCount(U32 instance)
	{
		m_sharedResourcesArray[instance]->DecrementReferenceCount();
	}

#endif

