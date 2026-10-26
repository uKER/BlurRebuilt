#ifndef FBX_IO_SCENE_IMPORT_H
#define FBX_IO_SCENE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <fbxsdk.h>
#include "FbxIoImportOptions.h"

namespace FbxIo
{
	class C_SceneImport
	{
	public:

		C_SceneImport();

		Aftk::C_Status Import(
			KFbxScene* pScene, 
			Aftk::C_Scene* pAftkScene,
			KFbxImporter* pFbxImporter,
			C_ImportOptions* pOptions,
			Aftk::I_Progress* pProgressCallback);

		KFbxScene* GetFbxScenePtr();

		Aftk::C_Scene* GetAftkScenePtr();

		KFbxImporter* GetFbxImporterPtr();

		KFbxTakeInfo* GetTakeInfo();

		const C_ImportOptions* GetOptionsPtr() const;
		
	private:


		Aftk::C_SceneNode* ConvertNode(
			KFbxNode* pFbxNode, 
			Aftk::I_Progress* pProgress,
			Aftk::C_Status& status);

		U32 CountFbxSceneNodes(KFbxNode* pFbxNode);

		void CleanUpAfterFailure();

		KFbxScene*				m_pFbxScene;
		Aftk::C_Scene*			m_pAftkScene;
		KFbxImporter*			m_pFbxImporter;
		C_ImportOptions*		m_pOptions;
		KFbxTakeInfo*			m_pTakeInfo;

		U32 m_numProcessingStagesCompleted;
		U32 m_numProcessingStages;
	};
}

#endif