#include "stdafx.h"
#include <AftkSpeedTree\AftkSpeedTree.h>
#include <AftkSpeedTree\AftkSpeedTreeInstance.h>
#include <AftkSpeedTree\AftkSpeedTreeData.h>

using namespace AftkSpeedTree;

void InstanceProcessor::Process(Aftk::C_Scene& aftk)
{
	Aftk::C_SceneNodeIterator aftkIt(&aftk);
	Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneInstanceNode*> > sptInstances;

	for (; aftkIt != NULL; aftkIt++)
	{
		if (aftkIt->GetType() != Aftk::E_ObjType_SceneInstance)
			continue;

		Aftk::C_SceneInstanceNode *pInstance = (Aftk::C_SceneInstanceNode*)(&(*aftkIt));
		Aftk::C_SceneNode* pSourceNode = pInstance->GetSourceNodePtr();

		if (!pSourceNode)
			continue;

		const Aftk::C_GenericAttribute* pAttribute = Utility::FindAttribute(*pSourceNode, "SPTData");
		if (pAttribute)
			sptInstances.PushBack(pInstance);
	}

	for (S32 i=0; i<sptInstances.Size(); ++i)
		ProcessNode(sptInstances[i]);
}

void InstanceProcessor::ProcessNode(Aftk::C_SceneInstanceNode* pInstanceNode)
{
	Aftk::C_SceneNode* pNode = pInstanceNode->GetSourceNodePtr();

	if(pNode)
	{
		// check to see if this is an instance of a tree node
		if (const Aftk::C_GenericAttribute* attribute = Utility::FindAttribute(*pNode, "SPTData"))
		{
			//create mesh instance as a child of the spt node instance
			Aftk::C_SceneInstanceNode *pMeshInstance = new Aftk::C_SceneInstanceNode(pInstanceNode->GetScenePtr());
			pInstanceNode->AddChildPtr(pMeshInstance);
			{
				//copy flags over
				if (AftkUt::C_NodeFlagParser::IsFlagged(*pInstanceNode))
				{
					AftkUt::C_NodeFlagParser parser(*pInstanceNode);
					parser.StoreFlags(*pMeshInstance);
				}

				//inherit layer membership
				for (U32 i=0; i<pInstanceNode->GetNumLayers(); ++i)
				{
					Aftk::C_LayerNode *pLayerNode = pInstanceNode->GetLayerPtr(i);
					pLayerNode->AddMember(pMeshInstance);
				}
			}

			if (AftkUt::C_NodeFlagParser::IsFlagged(*pMeshInstance))
			{
				AftkUt::C_NodeFlagParser parser(*pMeshInstance);
				//apply flag overrides
				const SpeedTreeSetup& setup = m_pTreeData->GetSetup();
				for (S32 i=0; i<setup.m_flagOverrides.Size(); ++i)
					parser.SetParameterValue(setup.m_flagOverrides[i].name.Chars(), setup.m_flagOverrides[i].value.Chars(), AftkUt::E_ParamType_Unknown);
			}

			Util::C_String sFilename;
			const Aftk::C_GenericAttribute* attrFilename = Utility::FindAttribute(*pNode, "Filename");
			{
				std::string tmp;
				attrFilename->GetValue(tmp);
				sFilename = tmp.c_str();
			}

			// if it is, make the instance point to the child of the tree node (the scene mesh) instead of the node
			LOGDEBUG( g_Log, "\tInstanceProcessor: Found Instance: %s\n", Util::ExtractFilename(sFilename).Chars());
			
			// this instance might currently be pointing at the mesh parent (the SPTData node)
			// so make it point directly at the mesh instead	
			for(U32 i = 0; i < pNode->GetNumChildren(); i++)
			{
				Aftk::C_SceneNode* pChildNode = (pNode->GetChildPtr(i));
				if(pChildNode->GetType() == Aftk::E_ObjType_SceneMesh || pChildNode->GetType() == Aftk::E_ObjType_SceneInstance)
				{
					//there should only be one mesh right?
					if (pChildNode->GetName().find("SpeedTreeMesh:") != std::string::npos)
					{
						pMeshInstance->SetSourceNodePtr(pChildNode);
						break;
					}
				}
			}

			// while we are instancing an instance, step through hierarchy until we reach mesh
			Aftk::C_SceneNode* pIterate = pMeshInstance->GetSourceNodePtr();
			while(pIterate->GetType() == Aftk::E_ObjType_SceneInstance)
			{
				Aftk::C_SceneInstanceNode* pIterateInstance = (Aftk::C_SceneInstanceNode*)pIterate;
				pIterate = pIterateInstance->GetSourceNodePtr();
			}
			pMeshInstance->SetSourceNodePtr(pIterate);
			
			{

				AftkUt::C_NodeFlagParser parser(*pNode);
				bool useSHLighting = false;
				if (parser.GetParameterValue("UseSHLighting", useSHLighting) && useSHLighting)
					Utility::SetupDefaultSHBlindData(pInstanceNode, pInstanceNode);
			}

			// Add tree instance position to tree instance list, so we can billboard up stuff later.
			// Anything we're instancing at this point needs to be mesh. If it isn't then we've previously
			// failed to generate a base instance for this tree. E.g. missing / invalid spt file.
			if (pMeshInstance->GetSourceNodePtr() && 
				pMeshInstance->GetSourceNodePtr()->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				S32 nIndex = 
					m_pTreeData->AddTreeInstanceInfo(
					pMeshInstance->GetWorldMatrix(), 
					pMeshInstance,
					(Aftk::C_SceneMeshNode*)pMeshInstance->GetSourceNodePtr(),
					0);
				if(nIndex > -1)
				{
					// add a suitable name for this instance
					Util::C_String	sName;
					{
						std::stringstream	sStream;
						sStream << "SpeedTreeInstance: " << Util::ExtractFilename(m_pTreeData->m_treeMesh[nIndex].sFilename).Chars() << " (" << m_pTreeData->m_treeMesh[nIndex].nSeed << "," << m_pTreeData->m_treeMesh[nIndex].fSize << ")";
						sName = sStream.str().c_str();
					}
					pMeshInstance->SetName(sName.Chars());

					U32 nRandom = rand()%m_pTreeData->m_treeMesh[nIndex].meshNodes.Size();
					pMeshInstance->SetSourceNodePtr(m_pTreeData->m_treeMesh[nIndex].meshNodes[nRandom]);
				}
			}
		}
	}
}



