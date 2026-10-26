#include "stdafx.h"

#include "FbxIoSceneImport.h"
#include "FbxIoSceneNodeImport.h"
#include "FbxIoAnimationImport.h"

using namespace FbxIo;

C_SceneImport::C_SceneImport() : 
	m_pFbxScene(NULL),
	m_pAftkScene(NULL),
	m_pFbxImporter(NULL),
	m_pOptions(NULL),
	m_pTakeInfo(NULL),
	m_numProcessingStagesCompleted(0),
	m_numProcessingStages(0)
{}

Aftk::C_Status C_SceneImport::Import(
	KFbxScene* pFbxScene, 
	Aftk::C_Scene *pAftkScene,
	KFbxImporter* pFbxImporter,
	C_ImportOptions* pOptions,
	Aftk::I_Progress* pProgress)
{
	m_pFbxScene = pFbxScene;
	m_pAftkScene = pAftkScene;
	m_pFbxImporter = pFbxImporter;
	m_pOptions = pOptions;

	Aftk::C_Status status;

	m_numProcessingStagesCompleted = 0;
	m_numProcessingStages = CountFbxSceneNodes(pFbxScene->GetRootNode());

	pAftkScene->Clear();

	//Set scene transform
	if (pAftkScene->GetRootSceneNodePtr())
	{
		Aftk::Matrix44 sceneTransform;
		SetIdentity(&sceneTransform);
		sceneTransform[2] *= -1.f;
		pAftkScene->GetRootSceneNodePtr()->SetLocalMatrix(
			sceneTransform * pAftkScene->GetRootSceneNodePtr()->GetLocalMatrix(false));
	}

	//Obtain take info 
	if(m_pOptions->takeName == "default") 
	{
		m_pTakeInfo = pFbxImporter->GetTakeInfo(0);
		m_pOptions->takeName = m_pTakeInfo->mName;
		m_pOptions->takeIndex = 0;
	} 
	else
	{
		int takeCount = pFbxImporter->GetTakeCount();
		KFbxTakeInfo* pTakeInfoTmp = NULL;
		for(int i = 0; i < takeCount; i++)
		{
			pTakeInfoTmp = pFbxImporter->GetTakeInfo(i);
			std::string takeName(pTakeInfoTmp->mName.Buffer());
			if(m_pOptions->takeName == takeName) 
			{
				m_pTakeInfo = pTakeInfoTmp;
				m_pOptions->takeIndex = i;
			}
		}
	}

	if(!m_pTakeInfo || m_pOptions->takeName == "")
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Incorrect take name."); 
	}

	//Set Aftk scene timing info
	
	KFbxGlobalTimeSettings *globalTime	= &pFbxScene->GetGlobalTimeSettings();
	KTime::ETimeMode globalMode			= globalTime->GetTimeMode();
	KTimeSpan localTs		= m_pTakeInfo->mLocalTimeSpan;
	KTime start				= localTs.GetStart();
	KTime stop				= localTs.GetStop();
	double frameRate		= KTime::GetFrameRate(globalMode); 
	start.SetGlobalTimeMode(globalMode); //this globally affects all KTimes, thus stop's "set" is not called

	pAftkScene->SetStartTime((F32)start.GetFrame(true));
	pAftkScene->SetEndTime((F32)stop.GetFrame(true));
	pAftkScene->SetFrameRate((F32)frameRate);
	
	/*
	KFbxGlobalTimeSettings *lTimeSettings;
	lTimeSettings = &pFbxScene->GetGlobalTimeSettings();
	KTimeSpan tspan;
	lTimeSettings->GetTimelineDefautTimeSpan(tspan);
	KTime start2				= tspan.GetStart();
	KTime stop2					= tspan.GetStop();
	KTime duration2				= tspan.GetDuration();
	double seconds2			= duration2.GetSecondDouble();
	KTime::ETimeMode lTimeMode = lTimeSettings->GetTimeMode();
	double frameRate2 = KTime::GetFrameRate(lTimeMode); 

	pAftkScene->SetStartTime((F32)start2.GetFrame(true));
	pAftkScene->SetEndTime((F32)stop2.GetFrame(true));
	pAftkScene->SetFrameRate((F32)frameRate2);*/

	//Convert nodes
	Aftk::C_SceneNode* pAftkRootNode = ConvertNode(
		pFbxScene->GetRootNode(), 
		pProgress,
		status);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END

	//Set Root node
	pAftkScene->SetRootSceneNodePtr(pAftkRootNode);

	pAftkScene->Optimise();

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_SceneNode* C_SceneImport::ConvertNode(
	KFbxNode* pFbxNode, 
	Aftk::I_Progress* pProgress,
	Aftk::C_Status& status)
{
	Aftk::C_SceneNode* pAftkSceneNode = NULL;

	if(pFbxNode->GetNodeAttribute() != NULL) {
	
		KFbxNodeAttribute::EAttributeType type = pFbxNode->GetNodeAttribute()->GetAttributeType();

		switch (type) 
		{	
			//case KFbxNodeAttribute::eMESH:
			//{
				/*
				pAftkSceneNode = new Aftk::C_SceneMeshNode(m_pAftkScene);
				if (!pAftkSceneNode)
				{
					status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new mesh node.", 1);
					return 0;
				}
				status = C_SceneMeshNodeImport::Import(
					this,
					pXsiNode, 
					pAftkSceneNode);
				if (!status)
				{
					//Returning the error reported by the importer.
					return 0;
				}*/

				//break;
			//}
			/*
			case CSLTemplate::SI_IK_ROOT:
			case CSLTemplate::SI_IK_JOINT:
			case CSLTemplate::SI_IK_EFFECTOR:
			{
				pAftkSceneNode = new Aftk::C_SceneJointNode(m_pAftkScene);
				if (!pAftkSceneNode)
				{
					status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new joint node", 1);
					return 0;
				}
				status = C_SceneIkNodeImport::Import(
					this,
					pXsiNode,
					pAftkSceneNode);
				if (!status)
				{
					return 0;
				}
				break;
			}*/
			case KFbxNodeAttribute::eSKELETON:
			{
				pAftkSceneNode = new Aftk::C_SceneJointNode(m_pAftkScene);
				if (!pAftkSceneNode)
				{
					status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new scene node.", 1);
					return 0;
				}

				status = C_SceneNodeImport::Import(
					this,
					pFbxNode,
					pAftkSceneNode);
				if (!status)
				{
					//Returning the error reported by the importer.
					return 0;
				}

				break;
			}
			case KFbxNodeAttribute::eMESH:
			case KFbxNodeAttribute::eMARKER:
			case KFbxNodeAttribute::eNULL:
			{
			
				pAftkSceneNode = new Aftk::C_SceneNode(m_pAftkScene);
				if (!pAftkSceneNode)
				{
					status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new scene node.", 1);
					return 0;
				}

				status = C_SceneNodeImport::Import(
					this,
					pFbxNode,
					pAftkSceneNode);
				if (!status)
				{
					//Returning the error reported by the importer.
					return 0;
				}

				break;
			}
		}

	} else {
		pAftkSceneNode = new Aftk::C_SceneNode(m_pAftkScene);
		if (!pAftkSceneNode)
		{
			status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new scene ndoe.", 1);
			return 0;
		}
		pAftkSceneNode->SetName(pFbxNode->GetName());
	}

	
	if (pAftkSceneNode)
	{
		
		status = AnimationImport::Import(
			this,
			pFbxNode,
			pAftkSceneNode);

		if (!status)
		{
			//Returning the error reported by the importer.
			return 0;
		}
		

		//Update progress...
		m_numProcessingStagesCompleted++;
		F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
		if (pProgress)
		{
			if (pProgress->OnProgress(progress))
			{
				status = Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene reading aborted.");
				return 0;
			}
		}

		//Establish hierachy relationships
		if (pFbxNode->GetChildCount() > 0)
		{
			Aftk::C_SceneNode* pAftkSceneNodeChild = 
				ConvertNode(
					pFbxNode->GetChild(0), 
					pProgress,
					status
				);
			if (!status)
			{
				//Return the error reported by the child exporter.
				return 0;
			}
			
			if(pAftkSceneNodeChild) {
				pAftkSceneNode->SetChildPtr(pAftkSceneNodeChild);
			}

			for (int i = 1; i < pFbxNode->GetChildCount(); i++ )
			{
				Aftk::C_SceneNode* pAftkSceneNodeNext = ConvertNode(
					pFbxNode->GetChild(i), 
					pProgress,
					status);
				if (!status)
				{
					//Return the error reported by the child exporter.
					return 0;
				}

				if(pAftkSceneNodeNext) {
					pAftkSceneNode->AddChildPtr(pAftkSceneNodeNext);
					pAftkSceneNodeChild = pAftkSceneNodeNext;
				}
			}
		}

		return pAftkSceneNode;
	}
	else
	{
		return 0;
	}
}

U32 C_SceneImport::CountFbxSceneNodes(KFbxNode* pFbxNode)
{
	U32 count = 1;

	for (S32 i = 0; i < pFbxNode->GetChildCount(); i++ )
	{
		count += CountFbxSceneNodes(pFbxNode->GetChild(i));
	}

	return count;
}

KFbxScene* C_SceneImport::GetFbxScenePtr()
{
	return m_pFbxScene;
}

Aftk::C_Scene* C_SceneImport::GetAftkScenePtr()
{
	return m_pAftkScene;
}

const C_ImportOptions* C_SceneImport::GetOptionsPtr() const
{
	return m_pOptions;
}

KFbxImporter* C_SceneImport::GetFbxImporterPtr()
{
	return m_pFbxImporter;
}

KFbxTakeInfo* C_SceneImport::GetTakeInfo()
{
	return m_pTakeInfo;
}

void C_SceneImport::CleanUpAfterFailure()
{
}
