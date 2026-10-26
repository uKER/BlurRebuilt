#include <stdafx.h>

using namespace AftkUt;

Aftk::C_Status C_RemoveBaseInstances::RemoveBaseInstances(
	Aftk::C_Scene* pAftkScene)
{
	Aftk::C_Status status;

	Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > baseInstances;

	//Build a list of flagged base instances.
	for (Aftk::C_SceneNodeIterator it(pAftkScene); it; ++it)
	{
		if (Query::IsBaseInstance(it))
			baseInstances.PushBack(it);
	}

	for (U32 i = 0; i < (U32)baseInstances.Size(); i++)
	{
		Aftk::C_SceneNode* pBaseInstance = baseInstances[i];

		if (pBaseInstance->GetNumInstancesOfThisNode() > 0)
		{
			Aftk::C_SceneNode* pNewBaseInstance = 
				(Aftk::C_SceneNode*)pBaseInstance->Clone(pAftkScene);

			Aftk::C_SceneNode* pInstanceToReplace =
				pBaseInstance->GetInstanceOfThisNode(0);

			//If we're an instance actor, copy proporties from our base instance.
			//Process blind data.
			{
				Aftk::C_BlindDataNode targetBd;
				//Get any existing blind data information from our cloned instance.
				if (pNewBaseInstance->GetBlindDataResourcePtr())
					targetBd = pNewBaseInstance->GetBlindDataResourcePtr()->GetContent();

				//Copy any dynamic attributes and blind data across to base instance.
				if (pInstanceToReplace->GetBlindDataResourcePtr())
				{
					const Aftk::C_BlindDataNode& bd = 
				 		pInstanceToReplace->GetBlindDataResourcePtr()->GetContent();
					for (U32 j = 0; j < bd.GetNumAttributes(); j++)
					{
						targetBd.AddAttribute(bd.GetAttribute(j));
					}

					//Copy name fromthe node we're replacing.
					targetBd.SetName(bd.GetName());
				}

				//Apply blind data.
				if (targetBd.GetNumAttributes() > 0)
				{
					const Aftk::C_BlindDataResource* pBd =
						pAftkScene->AddBlindDataNode(targetBd);
					
					pNewBaseInstance->SetBlindDataResourcePtr(pBd);
				}
			}

			//Process dynamic plugs.
			{
				for (U32 j = 0; j < pInstanceToReplace->GetNumDynamicPlugs(); j++)
				{
					Aftk::C_DynamicPlug* pSourcePlug = pInstanceToReplace->GetDynamicPlugPtr(j);
					Aftk::I_Plug* pTargetPlug = pNewBaseInstance->GetPlugPtr(
						pSourcePlug->GetName());

					//Copy value from the instance we're replacing.
					if (pTargetPlug && pTargetPlug->HasType(Aftk::E_ObjType_DynamicPlug))
					{
						//Overwrite existing plug.
						(Aftk::C_DynamicPlug&)*pTargetPlug = *pSourcePlug;
					}
					else
					{
						//Clone plug
						pNewBaseInstance->AddPlugPtr((Aftk::C_DynamicPlug*)pSourcePlug->Clone());
					}
				}
			}
			//Make a list of nodes instancing the one we're to replace.
			Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > instancesOfTheInstanceWereReplacing;
			for (U32 j = 1; j < (U32)pInstanceToReplace->GetNumInstancesOfThisNode(); j++)
			{
				instancesOfTheInstanceWereReplacing.PushBack(pInstanceToReplace->GetInstanceOfThisNode(j));
			}

			//Copy node name from the instance we're replacing.
			pNewBaseInstance->SetName(pInstanceToReplace->GetName());

			//Copy transform from the instance we're replacing.
			pNewBaseInstance->GetScalePlug() = pInstanceToReplace->GetScalePlug();
			pNewBaseInstance->GetRotatePlug() = pInstanceToReplace->GetRotatePlug();
			pNewBaseInstance->GetShearPlug() = pInstanceToReplace->GetShearPlug();
			pNewBaseInstance->GetTranslatePlug() = pInstanceToReplace->GetTranslatePlug();

			//Replace the first instance of our base instance with its first instance :)
			pNewBaseInstance->Replace(pInstanceToReplace, true);

			//Repoint instances of the replaced node
			for (U32 j = 0; j < (U32)instancesOfTheInstanceWereReplacing.Size(); j++)
			{
				Aftk::C_SceneNode* pInstance = instancesOfTheInstanceWereReplacing[j];
				status = C_Utility::ReparentInstance(pInstance, pNewBaseInstance);
				if (!status) return status;
			}

			//Repoint instances of the original base instance.
			{
				U32 nInstances = pBaseInstance->GetNumInstancesOfThisNode();
				for (U32 j = 0; j < nInstances; j++)
				{
					Aftk::C_SceneNode* pInstance = pBaseInstance->GetInstanceOfThisNode(0);
					status = C_Utility::ReparentInstance(pInstance, pNewBaseInstance);
					if (!status) return status;
				}
			}
			//Both sets of instances are now pointing at our newly cloned scene node.
		}
		//Delete original base instance as we no longer need it. The true flag ensures
		//that any children maintain the same world-space position.
		pAftkScene->DeleteSceneNodeAndReparentChildren(pBaseInstance, true);
	}

	return Aftk::C_Status::E_Status_Success;
}
