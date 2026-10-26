#ifndef AFTK_NAMED_RESOURCE_MANAGER_H
#define AFTK_NAMED_RESOURCE_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	#ifndef DOXYGEN_SHOULD_SKIP_THIS

	template<class Type>
	class C_NamedResourceManager 
	{
	public:
		///
		///\brief Constructor.
		C_NamedResourceManager();
		///
		///\brief Destructor.
		~C_NamedResourceManager();

		void AddResourceNoTest(C_SharedResource<Type>* pResource);
		///
		C_SharedResource<Type>*	AddResource(
			const Type &content,
			bool enforceUnique);
		///Given a pointer to a resource, look thru the manager and see if we have a resource stored at the
		///given address. If we do, then we decrease its reference count. One this count reaches 0, we delete
		///the resource.
		void DeleteResource(const C_SharedResource<Type>* pResource);

		const Type& GetResource(const std::string& name, bool &found) const;
		///
		///\brief Get the content occupying the given index in the array of resources.
		///@param instance The content instance to retrieve.
		///@return Type The shared resource content.
		const Type&	GetResource(U32 instance) const;
		Type&	GetResource(U32 instance);
		///
		///Get the shared resource content associated with the given shared resource pointer.
		///@param pResource Shared resource pointer.
		///@return Type The shared resource content.
		const Type&	GetResource(
			const C_SharedResource<Type>* pResource, 
			bool &found) const;
		///
		const C_SharedResource<Type>* GetResourcePtr(U32 instance) const;
		C_SharedResource<Type>* GetResourcePtr(U32 instance);
		///
		///Return the number of resources held by this manager
		///@return Number of resources.
		U32	GetNumResources() const;
		///
		///Set the value of the resource at the given memory address
		///@param pResource Shared resource pointer.
		///@param content new shared resource content.
		void SetResource(
			const C_SharedResource<Type>* pResource,
			const Type& newMaterial);

		void SetAttachData(
			const C_SharedResource<Type>* pMaterialResource,
			const C_AttachData& attachData);

		void SetAttachData(
			U32 instance,
			const C_AttachData& attachData);

		///
		///\brief Clear the contents of this resource manager.
		void		Clear();

	protected:
		void		IncrementReferenceCount(U32 instance, U32 ammount = 1);
		void		DecrementReferenceCount(U32 instance, U32 ammount = 1);
		std::string	GetUniqueName(const std::string& oldName) const;

		///
		///Deny access to copy constructor
		C_NamedResourceManager(const C_NamedResourceManager& rhs);
		///
		///Deny access to asignment operator.
		const C_NamedResourceManager& operator=(const C_NamedResourceManager& rhs);

		Ut::MultiMap<std::string, C_SharedResource<Type>* >		m_sharedResourcesMap;
		Ut::UtSet<C_SharedResource<Type>* >						m_sharedResourcesSet;
		///
		///The following vector is present so that we can treat the shared resources as an array.
		Ut::UtVector< Ut::DynamicVector<C_SharedResource<Type>*> > m_sharedResourcesArray;

		Type												m_emptyMaterial;

		friend class C_Scene;
	};

	template<class Type>
	C_NamedResourceManager<Type>::C_NamedResourceManager()
	{
	}

	template<class Type>
	C_NamedResourceManager<Type>::~C_NamedResourceManager()
	{
		Clear();
	}

	template<class Type>
	void C_NamedResourceManager<Type>::AddResourceNoTest(C_SharedResource<Type>* pSharedResource)
	{
		//If we get to this code, then we didn't find an existing material with the given name. Let's add this
		//material to the manager.
		{
			Type* pNewMaterialCopy = 
				const_cast< Type* >(&pSharedResource->GetContent());
		}
		pSharedResource->SetIndex((U32)m_sharedResourcesArray.Size());
		//Add the new shared resource to our map.
		
		m_sharedResourcesMap.Insert(
			typename Ut::MultiMap<std::string, C_SharedResource<Type>* >::DataType(pSharedResource->GetContent().GetName(), pSharedResource));
		//Add the new shared resource to our set.
		m_sharedResourcesSet.Insert(pSharedResource);
		//Maintain a linear array of resource pointers so that we can index them directly if needed
		m_sharedResourcesArray.PushBack(pSharedResource);
		//Resource added
	}

	template<class Type>
	C_SharedResource<Type>* C_NamedResourceManager<Type>::AddResource(
		const Type &newMaterial,
		bool enforceUnique)
	{
		if (enforceUnique)
		{
			//Our materials are indexed by name within a map. Material names are guaranteed to be unique.
			typename Ut::UtMap<std::string, C_SharedResource<Type>*>::ConstIterator it;

			it = m_sharedResourcesMap.Find(newMaterial.GetName());

			//We found an existing material with the same name as our 'new' material.
			if (it != m_sharedResourcesMap.End())
			{
				//We must compare the contents of these two materials in order to determine if they
				//are in fact identical.
				C_SharedResource<Type>* pSharedResource = it->GetValue();
				const Type* pExistingMaterial = &pSharedResource->GetContent();
				if ((*pExistingMaterial) == newMaterial) 
				{
					//Resource not added, use existing content
					return pSharedResource;
				}
				else
				{
					//Although the existing and 'new' material have the same name they are not identical with
					//respect to their content. In this case we must generate a unique name for the new material. 
					//Once this has been done, a recursive call to this function will add the material.
					Type materialCopy = newMaterial;
					materialCopy.SetName(GetUniqueName(newMaterial.GetName()));
					//Attempt to add the renamed material again
					return AddResource(materialCopy, enforceUnique);
				}
			}
		}

		//If we get to this code, then we didn't find an existing material with the given name. Let's add this
		//material to the manager.
		C_SharedResource<Type>* pSharedResource = 
			new C_SharedResource<Type>();

		pSharedResource->SetContent(newMaterial);
		{
			Type* pNewMaterialCopy = 
				const_cast< Type* >(&pSharedResource->GetContent());

			//Null material names are not valid! Generate a new name.
			if (pNewMaterialCopy->GetName().size() == 0)
			{
				pNewMaterialCopy->SetName("material");
				pNewMaterialCopy->SetName(GetUniqueName(pNewMaterialCopy->GetName()));
			}
		}
		pSharedResource->SetIndex((U32)m_sharedResourcesArray.Size());


		//Add the new shared resource to our map.
		m_sharedResourcesMap.Insert(
			typename Ut::MultiMap<std::string, C_SharedResource<Type>* >::DataType(newMaterial.GetName(), pSharedResource));
		//Add the new shared resource to our set.
		m_sharedResourcesSet.Insert(pSharedResource);
		//Maintain a linear array of resource pointers so that we can index them directly if needed
		m_sharedResourcesArray.PushBack(pSharedResource);
		//Resource added

		//Return a pointer to our new shared resource.
		return pSharedResource;
	}


	template<class Type>
	void C_NamedResourceManager<Type>::DeleteResource(const C_SharedResource<Type>* pMaterialResource)
	{
		if (!pMaterialResource)
		{
			return;
		}

		typename Ut::UtSet<C_SharedResource<Type>*>::Iterator it;
		it = m_sharedResourcesSet.Find(const_cast< C_SharedResource<Type>* >(pMaterialResource));

		if (it != m_sharedResourcesSet.End())
		{
			C_SharedResource<Type>* pCurrentResource = *it;

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
						shuffleIt = m_sharedResourcesSet.Begin();
						do
						{
							C_SharedResource<Type>* pResourceToPossiblyShuffle = *shuffleIt;
							//If the instance number of this resource is greater than the instance
							//of the node we are to remove then it must be shuffled down.

							if (pResourceToPossiblyShuffle->GetIndex() > pCurrentResource->GetIndex())
							{
								U32 newInstance = pResourceToPossiblyShuffle->GetIndex();
								newInstance--;
								{
									Type* pMaterialToAlter= 
										const_cast< Type* >(&pResourceToPossiblyShuffle->GetContent());
								}

								pResourceToPossiblyShuffle->SetIndex(newInstance);
							}

							shuffleIt++;
						}
						while (shuffleIt != m_sharedResourcesSet.End());
					}


				// free the memory taken by the unwanted resource
				{
					U32 oldSize = (U32)m_sharedResourcesMap.Size();
					m_sharedResourcesMap.Erase(pCurrentResource->GetContent().GetName());
					assert(oldSize > (U32)m_sharedResourcesMap.Size());
				}
				//We could be caching a dead material, clear cache.
				delete pCurrentResource;
				{
					U32 oldSize = (U32)m_sharedResourcesSet.Size();
					// delete the now invalid pointer from the manager
					m_sharedResourcesSet.Erase(it);
					assert(oldSize > (U32)m_sharedResourcesSet.Size());
				}
				return;
			}
			else 
			{
				return;
			}
		}
	}

	template<class Type>
	void C_NamedResourceManager<Type>::SetAttachData(
		const C_SharedResource<Type>* pMaterialResource,
		const C_AttachData& attachData)
	{
		typename Ut::UtSet<C_SharedResource<Type>* >::Iterator it;

		it = m_sharedResourcesSet.Find(const_cast< C_SharedResource<Type>* >(pMaterialResource));

		if (it == m_sharedResourcesSet.End())
		{
			//We attempted to attach data to a non existent material!
			assert(false && "Attaching data to invalid material!");
			return;
		}

		C_SharedResource<Type>* pCurrentResource = *it;
		Type* pMaterialNode = const_cast<Type*>(&pCurrentResource->GetContent());
		pMaterialNode->SetAttachData(attachData);
	}

	template<class Type>
	void C_NamedResourceManager<Type>::SetAttachData(U32 instance, const C_AttachData& attachData)
	{
		Type* pMaterialNode = const_cast<Type*>(&m_sharedResourcesArray[instance]->GetContent());
		pMaterialNode->SetAttachData(attachData);
	}

	template<class Type>
	void C_NamedResourceManager<Type>::Clear()
	{
		for (U32 i = 0; i < (U32)m_sharedResourcesArray.Size(); i++)
		{
			C_SharedResource<Type>* pResource = m_sharedResourcesArray[i];
			if (pResource != NULL) 
			{
				delete pResource;
			}
		}

		m_sharedResourcesSet.Clear();
		m_sharedResourcesMap.Clear();
		m_sharedResourcesArray.Clear();
	}

	template<class Type>
	const Type& C_NamedResourceManager<Type>::GetResource(const std::string& name, bool &found) const
	{
		typename Ut::UtMap<const std::string, C_SharedResource<Type>* >::ConstIterator it;

		it = m_sharedResourcesMap.Find(name);

		if (it == m_sharedResourcesMap.End())
		{
			//No such material!
			found = false;
			return m_emptyMaterial;
		}

		found = true;
		const C_SharedResource<Type>* pResource = it->GetValue();
		return pResource->GetContent();
	}

	template<class Type>
	const Type& C_NamedResourceManager<Type>::GetResource(U32 instance) const
	{
		return m_sharedResourcesArray[instance]->GetContent();
	}

	template<class Type>
	Type& C_NamedResourceManager<Type>::GetResource(U32 instance)
	{
		return m_sharedResourcesArray[instance]->GetContent();
	}

	template<class Type>
	const Type& C_NamedResourceManager<Type>::GetResource(
		const C_SharedResource<Type>* pResource, bool &found) const
	{
		typename Ut::UtSet<C_SharedResource<Type>*>::ConstIterator it;
		it = m_sharedResourcesSet.Find((C_SharedResource<Type>*)pResource);

		if (it != m_sharedResourcesSet.End())
		{
			found = true;
			C_SharedResource<Type>* pIteratorContents = *it;
			return pIteratorContents->GetContent();
		}
		else
		{
			//We didn't find the specified material
			found = false;
			return m_emptyMaterial;
		}
	}

	template<class Type>
	U32 C_NamedResourceManager<Type>::GetNumResources() const
	{
		return (U32)m_sharedResourcesMap.Size();
	}

	template<class Type>
	void C_NamedResourceManager<Type>::SetResource(
		const C_SharedResource<Type>* pSharedResource,
		const Type &content)
	{
		typename Ut::UtSet<C_SharedResource<Type>* >::Iterator it;

		it = m_sharedResourcesSet.Find(const_cast< C_SharedResource<Type>* >(pSharedResource));
		if (it == m_sharedResourcesSet.End())
		{
			//We couldn't find the given resource
			return;
//			AFTK_ASSERT(false);
		}

		//If we get here then this manager _DOES_ contain a resource at the given address
		//overwrite the contents of the shared resource

		//The name of the material may have been modified, therefore we must erase and reinsert
		//the material from our shared resource map before content setting.
		m_sharedResourcesMap.Erase(pSharedResource->GetContent().GetName());
		//Overwrite shared resource content with new material
		C_SharedResource<Type>* pSharedResourceNonConst = *it;
		pSharedResourceNonConst->SetContent(content);
		//Reinsert material into map with the correct name.

		m_sharedResourcesMap[pSharedResource->GetContent().GetName()] = pSharedResourceNonConst;
	}

	template<class Type>
	const C_SharedResource<Type>* C_NamedResourceManager<Type>::GetResourcePtr(U32 instance) const
	{
		return m_sharedResourcesArray[instance];
	}

	template<class Type>
	C_SharedResource<Type>* C_NamedResourceManager<Type>::GetResourcePtr(U32 instance)
	{
		return m_sharedResourcesArray[instance];
	}

	template<class Type>
	void C_NamedResourceManager<Type>::IncrementReferenceCount(U32 instance, U32 ammount)
	{
		m_sharedResourcesArray[instance]->IncrementReferenceCount(ammount);
	}

	template<class Type>
	void C_NamedResourceManager<Type>::DecrementReferenceCount(U32 instance, U32 ammount)
	{
		m_sharedResourcesArray[instance]->DecrementReferenceCount(ammount);
	}

	template<class Type>
	std::string C_NamedResourceManager<Type>::GetUniqueName(const std::string& oldName) const
	{
		std::string newName;

		S32 i;

		//Find the number of contiguous digits at the end of the oldName.
		for (i = (S32)oldName.length()-1; i >= 0; i--)
		{
			if (!std::isdigit(oldName[i]))
			{
				//We've found the end of the contigious digit block, break out.
				break;
			}
		}

		U32 number;
		std::string newNumberStr;
	    
		if (i == (S32)oldName.length()-1)
		{
			//We didn't find any digits at the end of the name
			newName = oldName;
			number = 1;
			{
				std::stringstream ss;
				ss << number;
				newNumberStr = ss.str();
			}
		}
		else
		{
			std::string digitBlock = oldName.substr(i+1, (S32)oldName.length()-i);
			{
				std::stringstream ss;
				ss << digitBlock;
				ss >> number;
			}
			number++;
			{
				std::stringstream ss;
				ss << number;
				newNumberStr = ss.str();
			}

			//Remove the digit block from the end of the old name during the copy
			newName = oldName.substr(0,oldName.length()-newNumberStr.length());
		}

		//Append our new digit block to the name
		newName += newNumberStr;

		return newName;
	}

	#endif
}

#endif

