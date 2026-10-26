#include "stdafx.h"
#include <ComponentSystem/CObjectManager.h>
#include <ComponentSystem/SObjectManagerDB.h>
#include <ComponentSystem/ComponentIterators.h>
#include <ComponentSystem/CMessage.h>
#include <ComponentSystem/IComponent.h>
#include <ComponentSystem/cparamsnode.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{
	CObjectManager::CObjectManager() : mDB(NULL)
	{
	}

	CObjectManager::~CObjectManager()
	{
		DeInit();
	}

	void CObjectManager::Init()
	{
		if (mDB != NULL)
		{
			delete mDB;
		}
		mDB = new SObjectManagerDB();

		//RegisterAllComponentTypes(); // this should be done outside of the component system
	}

	void CObjectManager::DeInit(void)
	{
		if (mDB != NULL)
		{
			DestroyAllComponents();

			// Now get rid of the database
			delete mDB;
			mDB = NULL;
		}
	}

	void CObjectManager::DeleteObject(CObjectId oId)
	{
		assert(mDB);
		if (ObjectExists(oId))
		{
			// delete the object and all its components...
			for(CComponentByObjectIterator iter(oId); !iter.IsDone(); ++iter)
			{
				IComponent *pComp = iter.GetComponent();
				EComponentTypeId compType = pComp->GetComponentTypeId();
				pComp->Deinit();
				mDB->mComponentTypeInfo[compType].mDestructionMethod(pComp);
				mDB->mComponentTypeToComponentMap[compType].erase(oId);
			}
		}
	}


	void CObjectManager::DestroyAllComponents()
	{
		assert(mDB);
		for (int compType = 0 ; compType < NUM_COMPONENT_TYPE_IDS ; ++compType)
		{
			for (CComponentByTypeIterator iter(static_cast<EComponentTypeId>(compType)) ; !iter.IsDone() ; ++iter)
			{
				IComponent *pComp = iter.GetComponent();
				if (pComp)
				{
					// Deinit and delete each component
					pComp->Deinit();
					mDB->mComponentTypeInfo[compType].mDestructionMethod(pComp);
				}
			}

			// Clear the map
			mDB->mComponentTypeToComponentMap[compType].clear();
		}
	}

	CComponentByTypeIterator CObjectManager::GetComponentsByComponentTypeId(EComponentTypeId type)
	{
		CComponentByTypeIterator iter(type);
		return iter;
	}

	CComponentByInterfaceIterator CObjectManager::GetComponentsByInterfaceId(EInterfaceId iid)
	{
		CComponentByInterfaceIterator iter(iid);
		return iter;
	}

	CComponentByObjectIterator CObjectManager::GetComponentsByObjectId(CObjectId oid)
	{
		CComponentByObjectIterator iter(oid);
		return iter;
	}

	// This method calls the static method RegisterComponentType() in each component class. This method
	// in turn sets up everything necessary for the component in question.
	void CObjectManager::RegisterAllComponentTypes()
	{
		// register  all the component types here.......
		
	}

	void CObjectManager::RegisterInterfaceWithComponent(EInterfaceId iid, EComponentTypeId cid)
	{
		assert(mDB);
		mDB->mInterfaceTypeToComponentTypes[iid].insert(cid);
	}

	void CObjectManager::RegisterComponentType(EComponentTypeId compID, ComponentCreationMethod pCreationFn, ComponentDestructionMethod pDestructionFn, CHash typeHash)
	{
		assert(mDB);
		mDB->mComponentTypeInfo[compID].mCreationMethod = pCreationFn;
		mDB->mComponentTypeInfo[compID].mDestructionMethod = pDestructionFn;
		mDB->mComponentTypeInfo[compID].mTypeHash = typeHash;
	}

	IComponent *CObjectManager::QueryInterface(CObjectId oId, EInterfaceId iid)
	{
		assert(mDB);
		std::set<EComponentTypeId> &compTypeSet = mDB->mInterfaceTypeToComponentTypes[iid];
		std::set<EComponentTypeId>::iterator compTypeSetIter;

		for (compTypeSetIter = compTypeSet.begin() ; compTypeSetIter != compTypeSet.end() ; ++compTypeSetIter)
		{
			EComponentTypeId compType = (*compTypeSetIter);
			CComponentMap &compMap = mDB->mComponentTypeToComponentMap[compType];
			CComponentMap::iterator compMapIter = compMap.find(oId);
			if (compMapIter != compMap.end())
			{ // We found our component
				return (*compMapIter).second;
			}
		}
		return NULL;
	}



	IComponent *CObjectManager::CreateComponent(CObjectId oId, CParameterNode *paramNode)
	{
		assert(mDB);

		// Get the component type
		EComponentTypeId cTypeId = paramNode->GetComponentType();

		if (cTypeId == CID_INVALID)
		{ // Don't know this one
			return NULL;
		}

		// Call the component creation method
		assert(mDB->mComponentTypeInfo[cTypeId].mCreationMethod);
		IComponent *pComp = mDB->mComponentTypeInfo[cTypeId].mCreationMethod();

		if (pComp != NULL)
		{
			pComp->SetObjectId(oId);
			pComp->Init(oId, *paramNode);
		}
		return pComp;
	}


	CObjectId CObjectManager::CreateObject(CParameterNode *paramNode)
	{
		CHash oid = paramNode->GetObjectName();
		return CreateObject(paramNode, oid);
	}

	CObjectId CObjectManager::CreateObject(CParameterNode *paramNode, CObjectId oId)
	{
		assert(paramNode->GetParameterType() == CHash("OBJECT"));

		if (ObjectExists(oId))
		{
			//assert(0);
			//return CHash();
			
			// warning we are returning the same id.
			return oId;
		}

		IComponent *comp = CreateComponent(oId, paramNode);
		assert(comp);

		if (AddComponentToDb(oId, comp))
		{
			// create child components......
			int sz = paramNode->GetChildNodeSize();
			for(int i=0; i<sz; i++)
			{
				CParameterNode *pNode = paramNode->GetNode(i);

				IComponent *comp = CreateComponent(oId, pNode);
				assert(comp);

				if (!AddComponentToDb(oId, comp))
				{
					assert(0);
				}
			}
		}
		else
		{
			assert(0);
		}



		return oId;
	}


	EComponentTypeId CObjectManager::GetComponentTypeFromHash(CHash typeName) const
	{
		assert(mDB);
		for (int i = 0 ; i < NUM_COMPONENT_TYPE_IDS ; ++i)
		{
			if (mDB->mComponentTypeInfo[i].mTypeHash == typeName)
			{
				return static_cast<EComponentTypeId>(i);
			}
		}
		return CID_INVALID;
	}

	bool CObjectManager::ObjectExists(CObjectId oId) const
	{
		assert(mDB);
		for (int compType = 0 ; compType < NUM_COMPONENT_TYPE_IDS ; ++compType)
		{
			CComponentMap &compMap = mDB->mComponentTypeToComponentMap[compType];
			CComponentMap::iterator compMapIter = compMap.find(oId);
			if (compMapIter != compMap.end())
			{ // We found a component belonging to this object, so the object exists.
				return true;
			}
		}
		return false;
	}

	bool CObjectManager::AddComponentToDb(CObjectId oId, IComponent *pComp)
	{ 
		assert(mDB);

		// Inserts pComp with the key oId, and returns whether the insert was successful
		return (mDB->mComponentTypeToComponentMap[pComp->GetComponentTypeId()].insert(CComponentMap::value_type(oId, pComp))).second;
	}

	bool CObjectManager::IsInterfaceImplementedByComponent(EInterfaceId iId, IComponent *pComp) const
	{
		assert(mDB);

		if (mDB->mInterfaceTypeToComponentTypes[iId].find(pComp->GetComponentTypeId()) == mDB->mInterfaceTypeToComponentTypes[iId].end())
		{
			return false;
		}
		return true;
	}

	void CObjectManager::SubscribeToMessageType(EComponentTypeId ctid, EMessageType mtid)
	{
		assert(mDB);

		mDB->mMessageTypeToComponentTypes[mtid].insert(ctid);
	}

	void CObjectManager::PostAMessage(const CObjectId& oid, const CMessage &msg)
	{
		PostAMessage( oid, *const_cast<CMessage *>(&msg));
	}

	void CObjectManager::PostAMessage(const CObjectId& oid, CMessage &msg)
	{ 
		PERF_EVENT("C_ObjectManager::PostAMessage");
		assert(mDB);

		std::set<EComponentTypeId>	&compSet = mDB->mMessageTypeToComponentTypes[msg.mType];
		std::set<EComponentTypeId>::iterator compSetIter;

		// Find the component types that subscribe to this message
		for (compSetIter = compSet.begin() ; compSetIter != compSet.end() ; ++compSetIter)
		{
			EComponentTypeId	ctid = (*compSetIter); // Found one

			// Go through the components of this component type and send message
			for (CComponentByTypeIterator ci =  GetComponentsByComponentTypeId(ctid) ; !ci.IsDone() ; ++ci)
			{
				IComponent *pComp = ci.GetComponent();
				if (pComp->GetObjectId() == oid)
				{
					pComp->HandleMessage(msg);
				}
			}
		}
	}

	void CObjectManager::BroadcastMessage(const CMessage &msg)
	{
		BroadcastMessage(*const_cast<CMessage *>(&msg));
	}

	void CObjectManager::BroadcastMessage(CMessage &msg)
	{
		assert(mDB);

		std::set<EComponentTypeId>	&compSet = mDB->mMessageTypeToComponentTypes[msg.mType];
		std::set<EComponentTypeId>::iterator compSetIter;

		// Find the component types that subscribe to this message
		for (compSetIter = compSet.begin() ; compSetIter != compSet.end() ; ++compSetIter)
		{
			EComponentTypeId	ctid = (*compSetIter); // Found one

			// Go through the components of this component type and send message
			for (CComponentByTypeIterator ci =  GetComponentsByComponentTypeId(ctid) ; !ci.IsDone() ; ++ci)
			{
				ci.GetComponent()->HandleMessage(msg);
			}
		}
	}
}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
