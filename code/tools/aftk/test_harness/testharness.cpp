#ifdef WIN32
#include <windows.h>
#include <crtdbg.h>
#endif

#include "MemoryDbg.h"

#include <core/core.h>
#include <core/maths.h>
#include <core/containers/vector.h>

#include <stdarg.h>
#include <cassert>
#include <math.h>

#include <string>

#include <iostream>
#include <sstream>
#include <fstream>

#ifdef GCC
#define min(A,B) ((A<B)?A:B)
#define max(A,B) ((A>B)?A:B)
#endif

#include "../Aftk/include/Aftk/Aftk.h"

#include <tools_debug/DebugManager.h>

#ifdef AFTK_MAYA_IO
#include <maya/MLibrary.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#endif


S32 g_globalAllocCount = 0;
S32 g_globalAllocSize = 0;

int count = 0;

void* operator new (size_t size) 
{
	if (size > 0)
	{
		g_globalAllocSize += (U32)size;
	}
	g_globalAllocCount++;

	void* p = malloc(size);
	return p; 
}

void operator delete (void *p)
{	
	//g_globalAllocCount--;
	free(p);	
}


Tu::DebugManager* g_pDebugManager;

class C_Progress : public Aftk::I_Progress
{
    virtual bool OnProgress(F32 percent)
	{
		std::cout << "Progress: " << percent << std::endl;
		return false;
	}

	virtual void OnPrintProgress(const std::string& text)
	{
		std::cout << "Status: " << text << std::endl;
	}
	
	virtual void OnPrintWarning(const std::string& text)
	{
		std::cout << "Warning: " << text << std::endl;
	}

	virtual void OnPrintError(const std::string& text)
	{
		std::cout << "Error: " << text << std::endl;
	}
};

void DoDebug(const std::string& text)
{
	*g_pDebugManager << "DEBUG: " << text;
}

void DoMessage(const std::string& text)
{
	*g_pDebugManager << text;
}

void DoWarning(const std::string& text)
{
	*g_pDebugManager << "WARNING: " << text;
}

void DoError(const std::string& text)
{
	*g_pDebugManager << "ERROR: "<< text;
}

void PrintMatrix(const Aftk::Matrix44& m)
{
	for (U32 i = 0; i < 4; i++)
		std::cout << m[i][0] << " " << m[i][1] << " " << m[i][2] << " " << m[i][3] << std::endl;
}
/*
// set the blind data that contains UV offset and atlas index info for instanced geometry
void SetInstanceParameters(Aftk::C_SceneMeshRemappableNode *pNode)
{
	//
	Aftk::C_BlindDataNode newBlindData;

	// copy existing blind data into our new blind data (if it's there)
	const Aftk::C_BlindDataResource *pBlindDataResource = pNode->GetBlindDataResourcePtr();
	if (pBlindDataResource)
	{
		const Aftk::C_BlindDataNode *pBlindData = &pNode->GetBlindDataResourcePtr()->GetContent();
		if (pBlindData)
			newBlindData = *pBlindData;
	}

	char *pszLmapFilename = "meh";

	// check whether blind data is already there
	bool bUVExists = false;
	bool bIndexExists = false;
	for (int i=0; i<newBlindData.GetNumAttributes(); i++)
	{
		const std::string &name = newBlindData.GetAttribute(i).GetName();

		if (name == "UVoffset")
		{
			bUVExists = true;
			newBlindData.GetAttribute(i).SetValue(++count);
		}
		else if (name == "AtlasName")
		{
			bIndexExists = true;
			newBlindData.GetAttribute(i).SetValue(pszLmapFilename);
		}
	}

	// --- set blind data for Ed

	if (!bUVExists)
	{
		// set UV offset
		Aftk::C_GenericAttribute uvOffsetAttr;
		uvOffsetAttr.SetName("UVoffset");
		uvOffsetAttr.SetValue(++count);
		newBlindData.AddAttribute(uvOffsetAttr);
	}

	if (!bIndexExists)
	{
		// set lightmap index
		Aftk::C_GenericAttribute atlasIndexAttr;
		atlasIndexAttr.SetName("AtlasName");
		atlasIndexAttr.SetValue(++count);
		newBlindData.AddAttribute(atlasIndexAttr);
	}

	// add new blind data into the scene and the scene node
	const Aftk::C_BlindDataResource* pNewBlindDataResource = 
		pNode->GetScenePtr()->AddBlindDataNode(newBlindData);
	pNode->SetBlindDataResourcePtr(pNewBlindDataResource);
}
*/
void SetSHBlindDataToNode(
	Aftk::C_SceneNode *pNode, 
	Aftk::Vector4F32 vR, 
	Aftk::Vector4F32 vG, 
	Aftk::Vector4F32 vB)
{
	Aftk::C_GenericAttribute attr;
	Aftk::C_BlindDataNode newBlindData;

	// copy existing blind data into our new blind data (if it's there)
	const Aftk::C_BlindDataResource *pBlindDataResource = pNode->GetBlindDataResourcePtr();
	if (pBlindDataResource)
	{
		const Aftk::C_BlindDataNode *pBlindData = &pNode->GetBlindDataResourcePtr()->GetContent();
		if (pBlindData)
			newBlindData = *pBlindData;
	}

	// check whether any tree SH node attributes already exist in the blind data node
	for (int i=0; i<newBlindData.GetNumAttributes(); )
	{
		if (stricmp(newBlindData.GetAttribute(i).GetName().c_str(), "SH_R") == 0 ||
			stricmp(newBlindData.GetAttribute(i).GetName().c_str(), "SH_G") == 0 ||
			stricmp(newBlindData.GetAttribute(i).GetName().c_str(), "SH_B") == 0)
			newBlindData.DeleteAttribute(i);
		else
			i++;
	}

	// add our attribute to the blind data
	attr.SetName("SH_R");		attr.SetValue(vR);	newBlindData.AddAttribute(attr);
	attr.SetName("SH_G");		attr.SetValue(vG);	newBlindData.AddAttribute(attr);
	attr.SetName("SH_B");		attr.SetValue(vB);	newBlindData.AddAttribute(attr);

	std::cout << pNode->GetUniqueId() << std::endl;
	// add new blind data into the scene and the scene node
	const Aftk::C_BlindDataResource* pNewBlindDataResource = 
		pNode->GetScenePtr()->AddBlindDataNode(newBlindData);
	pNode->SetBlindDataResourcePtr(pNewBlindDataResource);
}


int main(int argc, char **argv)
{
	Debug::AutoConfigureDebugLog();

	#ifdef _DEBUG
	MemMirrorOn();
	#endif

	g_pDebugManager = new Tu::DebugManager();
	#ifdef _DEBUG
		g_pDebugManager->SetDebugEnabledFlag(false);
		g_pDebugManager->SetConsoleEnabledFlag(true);
		g_pDebugManager->SetLogFileEnabledFlag(true);
		g_pDebugManager->Create("c:/bug.log");
	#else 
		g_pDebugManager->SetDebugEnabledFlag(false);
		g_pDebugManager->SetConsoleEnabledFlag(true);
		g_pDebugManager->SetLogFileEnabledFlag(true);
		g_pDebugManager->Create("c:/bug.log");
	#endif

#ifdef AFTK_MAYA_IO
	MStatus mayaStatus = MLibrary::initialize("TestHarness", true);
	if (!mayaStatus) 
	{
		*g_pDebugManager
			<< "Failed to initialise Maya engine. [" 
			<< mayaStatus.errorString().asChar() 
			<< "].";
	}
	MString mVersion = MGlobal::mayaVersion();
	std::cout << "Using Maya Version [" << mVersion.asChar() << "]" << std::endl;
#endif


	Aftk::C_Status aftkStatus;

	Aftk::C_Global::Create();
	C_Progress progress;

	Ut::UtVector<Ut::DynamicVector<Aftk::C_MaterialNode > > materials;
	materials.PushBack();

	Aftk::C_Scene* pAftkScene = new Aftk::C_Scene();

	DWORD start = GetTickCount();

	aftkStatus = pAftkScene->Read("BeforeMergeMeshes.a3d");

	Aftk::C_SceneMeshNode* pSceneNode = (Aftk::C_SceneMeshNode*)pAftkScene->GetSceneNodePtrByUniqueID(135);

	Ut::Vector<Aftk::C_SceneNode*> toDelete;
	for (Aftk::C_SceneNode* pSibling = pSceneNode->GetNextPtr(); pSibling; pSibling = pSibling->GetNextPtr())
	{
		if (pSibling->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		Aftk::C_SceneMeshNode* pMesh = (Aftk::C_SceneMeshNode*)pSibling;

		pSceneNode->AddMesh(pMesh);
		toDelete.PushBack(pMesh);
	}

	for (S32 i = 0; i< toDelete.Size(); i++)
	{
		pAftkScene->DeleteSceneNodeAndReparentChildren(toDelete[i], true);

	}
	pAftkScene->Write("test.a3d");
	return 0;

	pAftkScene->Validate();
	pAftkScene->Write("TestAssets/genattrout.a3d");
	return 0;
	pAftkScene->Optimise();
//	pAftkScene->Write("newout.a3d");
		

	if (!aftkStatus)
		return 1;
	U32 nErrors = pAftkScene->Validate();
	if (nErrors == 0)
	{
		std::cout << "ERRORS FOUND!" << std::endl;
		return 1;
	}
	std::cout << "OKAY" << std::endl;
	return 0;

//	aftkStatus = pAftkScene->Read("mergeme.a3d");
//	Aftk::C_SceneMeshNode* pCube1 = (Aftk::C_SceneMeshNode*)pAftkScene->GetSceneNodePtr("pCube1");
//	Aftk::C_SceneMeshNode* pNew = (Aftk::C_SceneMeshNode*)pCube1->Clone(pAftkScene);
//	pNew->AddMesh(pCube1);
//	pNew->Replace(pCube1);

//	std::cout << "welding" << std::endl;
/*
	for (Aftk::C_SceneNodeIterator it(pAftkScene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetType() != Aftk::E_ObjType_SceneLodGroup)
			continue;

		Aftk::C_SceneLodGroupNode* pLodGroup = (Aftk::C_SceneLodGroupNode*)pSceneNode;
		
		Aftk::C_SceneMeshBonedInstanceNode* pBonedInst0 = 
			(Aftk::C_SceneMeshBonedInstanceNode*)pLodGroup->GetLodLevelInfo(0).GetSceneNodePtr();
		Aftk::C_SceneMeshBonedInstanceNode* pBonedInst1 = 
			(Aftk::C_SceneMeshBonedInstanceNode*)pLodGroup->GetLodLevelInfo(1).GetSceneNodePtr();

		while(true)
		{
			pBonedInst0->Evaluate();
			Aftk::Position3 pos0 = pBonedInst0->GetEvaluatedPolygonVertexPosition(0);
			pBonedInst1->Evaluate();
			Aftk::Position3 pos1 = pBonedInst1->GetEvaluatedPolygonVertexPosition(1);
		}

		int b = 0;
	}
	Aftk::C_SceneMeshBonedInstanceNode* pFenceWorking1 = 
		(Aftk::C_SceneMeshBonedInstanceNode*)pAftkScene->GetSceneNodePtr("fence_4_meters_overhang_curve16");
	Aftk::C_SceneMeshBonedInstanceNode* pFenceBroken2 = 
		(Aftk::C_SceneMeshBonedInstanceNode*)pAftkScene->GetSceneNodePtr("fence_4_meters_overhang_curve14");
/*
	while(true)
	{
		pFenceWorking1->Evaluate();
		Aftk::Position3 pos0 = pFenceWorking1->GetEvaluatedPolygonVertexPosition(0);
		std::cout << std::endl;
		PrintMatrix(pFenceWorking1->GetBonePtr(0)->GetBasePose());
		std::cout << std::endl;
		PrintMatrix(pFenceWorking1->GetBonePtr(1)->GetBasePose());
		pFenceBroken2->Evaluate();
		Aftk::Position3 pos1 = pFenceBroken2->GetEvaluatedPolygonVertexPosition(1);
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		PrintMatrix(pFenceBroken2->GetBonePtr(0)->GetBasePose());
		std::cout << std::endl;
		PrintMatrix(pFenceBroken2->GetBonePtr(1)->GetBasePose());

		int btb = 0;
		return 0;
	}

	return 0;
*/
	DWORD end = GetTickCount();
	DWORD time = end - start;
	std::cout << "time : " << (F32)time / 1000.f << " seconds " << std::endl;
	U32 count = pAftkScene->Validate();
	std::cout << count << std::endl;
	pAftkScene->Write("newout.a3d");
	delete pAftkScene;
 	Aftk::C_Global::Destroy();
	return 0;
}
/*
	std::cout << "Node remover utility. Specify \"input\" \"output\" \"badnode1\" \"badnode2\" etc..." << std::endl;

	Aftk::C_Status aftkStatus;

	Aftk::C_Global::Create();

	Aftk::C_Scene* pAftkScene = new Aftk::C_Scene();

	if (argc < 3)
	{
		std::cout << "Not enough args" << std::endl;
		return 0;
	}

	aftkStatus = pAftkScene->Read(argv[1]);
	if (!aftkStatus)
	{
		std::cout << "Failed to read " << aftkStatus.GetErrorString() << std::endl;
		return 1;
	}

	for (U32 i = 3 ; i < argc; i++)
	{
		std::cout << "Deleting [" << argv[i] << "]" << std::endl;
		Aftk::C_SceneNode* pSceneNode = pAftkScene->GetSceneNodePtr(argv[i]);
		pAftkScene->DeleteSceneNodeAndReparentChildren(pSceneNode, true);
	}

	if (pAftkScene->Validate() != 0)
	{
		std::cout << "Failed to validate." << std::endl;
		return 1;
	}
	if (!pAftkScene->Write(argv[2]))
	{
		std::cout << "Failed to write " << argv[2] << std::endl;
		return 1;
	}

	std::cout << "Done. Freeing memory..." << std::endl;
	delete pAftkScene;
	Aftk::C_Global::Destroy();
	return 0;
}
/*
		Aftk::C_AnimationCurveNode* pCurve = 
			(Aftk::C_AnimationCurveNode*)pSceneNode->GetDynamicPlugPtr(1)->GetSourceConnectionPtr();

		F32 test;
		pCurve->Evaluate(test, 16.f);
		

		aftkStatus = pAftkScene->Write("fored.a3d");
		return 0;
//		aftkStatus = pAftkScene->Read("loc.mb");
/*
		Aftk::Vector3F32 scale;
		Aftk::C_SceneNode* pSceneNode;
		pSceneNode = pAftkScene->GetSceneNodePtr("l_buttock_driven");
		pSceneNode->GetScale(scale);

		Aftk::Matrix44 m = pSceneNode->GetWorldMatrix();
/*
		pSceneNode = pAftkScene->GetSceneNodePtr("spotLight2");
		pSceneNode->GetScale(scale);

		pSceneNode = pAftkScene->GetSceneNodePtr("spotLight3");
		pSceneNode->GetScale(scale);

		aftkStatus = pAftkScene->Write("janbug.a3d");
		return 0;
}
/*
		aftkStatus = pAftkScene->Read("upvector.a3d");
		pSceneNode = pAftkScene->GetSceneNodePtr("Cube_Aimed_aimConstraint1");

		return 0;
}
//		aftkStatus = pAftkScene->Read("polyflagtest.a3d");
/*		{
			pAftkScene->SetSlerpRotation(true);
			Aftk::C_SceneNodeIterator it(pAftkScene);
			for(; it; ++it)
			{
				//Get a pointer to the current scene node.
				Aftk::C_SceneNode* pSceneNode = it;

				std::cout << pSceneNode->GetName() << std::endl;

//				if (pSceneNode->GetName() != "RIGHT_FOREARM_TWIST_DRIVEN1")
				if (pSceneNode->GetName() != "pCube1")
					continue;

				for (F32 i = 0; i < 20.f; i++)
				{
					Aftk::Vector3F32 euler;

					pAftkScene->SetCurrentTime(i);

					Aftk::Matrix44 m;
					pSceneNode->GetRotation(m);
					pSceneNode->GetRotation(euler);

					Aftk::Vector3F32 trans;
					pSceneNode->GetTranslation(trans);
					std::cout << "t = " << i << " Euler = " << Maths::RadToDeg(euler[0]) << ", " << Maths::RadToDeg(euler[1]) << ", " << Maths::RadToDeg(euler[2]) << std::endl;
				}
			}
		}
		aftkStatus = pAftkScene->Write("polyflagtest.a3d");
		return 0;
		std::cout << "\n\n\n*************************\n\n\n" << std::endl;
		aftkStatus = pAftkScene->Read("polyflagtest.a3d");
		{
			Aftk::C_SceneNodeIterator it(pAftkScene);
			for(; it; it++)
			{
				//Get a pointer to the current scene node.
				Aftk::C_SceneNode* pSceneNode = it;

				if (pSceneNode->HasType(Aftk::E_ObjType_SceneCamera))
				{
					//Display the name of this node.
					Aftk::Matrix44 m = pSceneNode->GetParentPtr()->GetLocalMatrix();
					std::cout << m[0][0] << " " << m[0][1] << " " << m[0][2] << std::endl;
					std::cout << m[1][0] << " " << m[1][1] << " " << m[1][2] << std::endl;
					std::cout << m[2][0] << " " << m[2][1] << " " << m[2][2] << std::endl;
					std::cout << m[3][0] << " " << m[3][1] << " " << m[3][2] << std::endl;

					Aftk::C_SceneCameraNode* pCamera =(Aftk::C_SceneCameraNode*)pSceneNode;
					F32 fov = pCamera->GetHorizontalFieldOfView();
					pCamera->SetHorizontalFieldOfView(fov);

					std::cout << pCamera->GetFocalLengthPlug().GetPath() << std::endl;

					Aftk::C_Node* pNode = 
						(Aftk::C_Node*)pCamera->GetFocalLengthPlug().GetSourceConnectionPtr();

					std::cout << pNode->GetPath() << std::endl;

					Aftk::C_UnitConversionNode* pUnit = 
						(Aftk::C_UnitConversionNode*)pNode;

					std::cout << pUnit->GetInputPlug().GetSourceConnectionPtr()->GetPath() << std::endl;

					std::cout << pCamera->GetRotatePlug().GetPath() << " -> ";
					std::cout << pCamera->GetRotatePlug().GetChildPtr(0)->GetSourceConnectionPtr()->GetPath() << std::endl;

					pCamera->GetTranslatePlug().OnChanged();

					std::cout << Maths::RadToDeg(pCamera->GetHorizontalFieldOfView()) << std::endl;

					for (U32 i = 0; i < pCamera->GetNumStaticPlugs(); i++)
					{
						std::cout << pCamera->GetStaticPlugPtr(i)->GetPath() << std::endl;
					}
				}
			}
		}

		return 0;

		for (U32 i = 0; i < pAftkScene->GetNumMaterialNodes(); i++)
		{
			Aftk::C_MaterialNode::Output(pAftkScene->GetMaterialNode(i));
		}
		
//		aftkStatus = pAftkScene->Read("f:/thegoodandthebad/thegood.xsi");
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;

		const Ut::UtSet<Aftk::C_Node*>& dgSet = pAftkScene->GetDgNodeSet();

		for (Ut::UtSet<Aftk::C_Node*>::ConstIterator it = dgSet.Begin();
			it != dgSet.End();
			++it)
		{
			if (it->GetKey()->HasType(Aftk::E_ObjType_AnimationCurve))
			{
				const Aftk::C_AnimationCurveNode* pAnimCurve = 
					(const Aftk::C_AnimationCurveNode*)it->GetKey();
				F32 value;

				bool result = pAnimCurve->Evaluate(value, 5.f);
				std::cout << value << std::endl;
			}
		}
	}
/*
		pAftkScene->OutputSceneGraph();

		pAftkScene->SetCurrentTime(1.f);
//		Aftk::C_Bounds bounds = pAftkScene->GetBounds();

//		Aftk::C_ClipMixerNode* p = new Aftk::C_ClipMixerNode(pAftkScene);
//		pAftkScene->GetRootSceneNodePtr()->GetTranslatePlug().SetSourceConnectionPtr(p);

//		Aftk::Matrix44 id;
//		UtMaths::SetIdentity(&id);
//		pAftkScene->SetSceneTransform(id);

		Aftk::C_SceneNode* pHip;
		pHip = pAftkScene->GetSceneNodePtr("back1");
		AFTK_ASSERT(pHip);
		
		Aftk::Matrix44 m = pHip->GetLocalMatrix();
		*g_pDebugManager 
			<< m[0][0] << " " <<  m[0][1] << " " << m[0][2] << " " << m[0][3] << "\n"
			<< m[1][0] << " " <<  m[1][1] << " " << m[1][2] << " " << m[1][3] << "\n"
			<< m[2][0] << " " <<  m[2][1] << " " << m[2][2] << " " << m[2][3] << "\n"
			<< m[3][0] << " " <<  m[3][1] << " " << m[3][2] << " " << m[3][3] << "\n";

		return 0;
		pHip = pAftkScene->GetSceneNodePtr("hip");
		AFTK_ASSERT(pHip);

		*g_pDebugManager << "\n";
		
		m = pHip->GetLocalMatrix();
		*g_pDebugManager 
			<< m[0][0] << " " <<  m[0][1] << " " << m[0][2] << " " << m[0][3] << "\n"
			<< m[1][0] << " " <<  m[1][1] << " " << m[1][2] << " " << m[1][3] << "\n"
			<< m[2][0] << " " <<  m[2][1] << " " << m[2][2] << " " << m[2][3] << "\n"
			<< m[3][0] << " " <<  m[3][1] << " " << m[3][2] << " " << m[3][3] << "\n";
		
		U32 nRemoved2 = pAftkScene->DeleteUnusedDgNodes(Aftk::E_ObjType_ClipMixer);


		delete pAftkScene;
		Aftk::C_Global::Destroy();
		std::cout << "Destroyed." << std::endl;

		g_pDebugManager->Destroy();
		delete g_pDebugManager;

		#ifdef _DEBUG
		_CrtDumpMemoryLeaks();
		#endif
		return 0;
	}

/*		AFTK_ASSERT(pAftkScene->Validate() == 0);

		for (Aftk::C_SceneNodeIterator it(pAftkScene); it ; ++it)
		{
			Aftk::C_SceneNode* pSceneNode = it;
			*g_pDebugManager << pSceneNode->GetPath() << "\n";
			if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				*g_pDebugManager 
					<< (pSceneNode->GetLocalMatrix())[0] << "\n"
					<< (pSceneNode->GetLocalMatrix())[1] << "\n"
					<< (pSceneNode->GetLocalMatrix())[2] << "\n"
					<< (pSceneNode->GetLocalMatrix())[3] << "\n";
			}
		}
/*		{
			Aftk::C_Scene scene2;
			aftkStatus = scene2.Read("xsi/Moving_Steeringwheel.xsi", Aftk::E_FileType_BestGuess, options);
			if (!aftkStatus)
				std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;

			pAftkScene->Import(&scene2);
			if (!aftkStatus)
				std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;
		}
		AFTK_ASSERT(pAftkScene->Validate() == 0);
		*/
/*
		pAftkScene->OutputSceneGraph();
		aftkStatus = pAftkScene->Write("namespace.a3d", Aftk::E_FileType_BestGuess);
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;

		pAftkScene->Clear();
//		aftkStatus = pAftkScene->Read("tex.a3d", Aftk::E_FileType_BestGuess, options);
		aftkStatus = pAftkScene->Read("namespace.a3d", Aftk::E_FileType_BestGuess, options);
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;
		pHip = pAftkScene->GetSceneNodePtr("pCube1");
		
		pHip->GetLocalMatrix();
		AFTK_ASSERT(pAftkScene->Validate() == 0);
		U32 nRemoved4 = pAftkScene->DeleteUnusedDgNodes(Aftk::E_ObjType_ClipMixer);

/*
		const AftkMaterialResource* pMatRes = pAftkScene->GetMaterialResourcePtr(0);

		for (AftkSceneNodeIterator it(pAftkScene); it ; it++)
		{
			Aftk::C_SceneNode* pSceneNode = it;
			if (pSceneNode->GetType() == E_ObjType_SceneMesh)// && pSceneNode->GetUniqueId() == 183)
			{
				if (
					(pSceneNode->GetType() == E_ObjType_SceneMesh) && 
					(!stricmp("Portals:PortalRoomsTest1:pCube251", pSceneNode->GetName().c_str())))
				{
					AftkSceneMeshNode* pMesh = (AftkSceneMeshNode*)pSceneNode;
					for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(0); iPoly++)
					{
						C_Polygon* pPoly = &pMesh->GetPolygon(iPoly);
						S16 matIdx = pPoly->GetMaterialMappingIndex();
						const AftkMaterialResource* pMatRes = 
							pMesh->GetPolygonMaterialMappingPtr(matIdx);
						
						const AftkShaderImageNode* pImage = pMatRes->GetContent().GetShaderImageNodePtr(0);
						if (pImage)
						{
							*g_pDebugManager << "Poly " << iPoly << " " << pImage->GetFileName() << "\n";
						}
//						pAftkScene->OutputMaterial(pMatRes);
					}
				}
			}
		}
		*/
/*
		pAftkScene->DeleteUnusedMaterialNodes();

		for (AftkSceneNodeIterator it(pAftkScene); it ; it++)
		{
			Aftk::C_SceneNode* pSceneNode = it;
			if (pSceneNode->GetType() == E_ObjType_SceneMesh && pSceneNode->GetUniqueId() == 9)
			{
				AftkSceneMeshNode* pMesh = (AftkSceneMeshNode*)pSceneNode;
				int x = 5;
			}
			if (pSceneNode->GetType() == E_ObjType_SceneMesh)
			{
				AftkSceneMeshNode* pMesh = (AftkSceneMeshNode*)pSceneNode;
			}
		}

//		ASSERT(pMaterialResource->GetReferenceCount() == 0);

*/
/*	
		AFTK_ASSERT(pAftkScene->Validate() == 0);
		aftkStatus = pAftkScene->Write("f:/176/warehousedm2_new.a3d", AFTK_FILE_TYPE_A3D);
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;
		std::cout << "done" << std::endl;
*/
//		aftkStatus = pAftkScene->Read("f:/176/lighttest.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//		aftkStatus = pAftkScene->Read("f:/janbug/tokyo.a3d", Aftk::E_FileType_BestGuess, options);
//		aftkStatus = pAftkScene->Read("f:/176/simpletest.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
/*		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;

		pAftkScene->Repair();
		pAftkScene->ExpandMeshPolygonVertices();
		aftkStatus = pAftkScene->Write("f:/176/test.mb", Aftk::E_FileType_BestGuess);
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;
*/

		
//		return 0;
/*
		std::cout << "Reading native endian file." << std::endl;
		aftkStatus = pAftkScene->Read("f:/175/newformat.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;
		std::cout << pAftkScene->GetSceneTransform()[0][0] << " " << pAftkScene->GetSceneTransform()[1][1] << " " << pAftkScene->GetSceneTransform()[2][2] << std::endl;
		{
			////////////
			////////////
			for (AftkSceneNodeIterator it(pAftkScene); it ; it++)
			{
				Aftk::C_SceneNode* pSceneNode = it;
				if (pSceneNode->GetType() == E_ObjType_SceneMesh)
				{
					std::cout << "node: " << pSceneNode->GetName() << "\n";
					AftkSceneMeshNode* pMesh = (AftkSceneMeshNode*)pSceneNode;

					int n = pMesh->GetNumTextureCoordinates(0);
					for (int i=0; i<n; i++)
					{
						AftkTextureCoordinate4 uv = pMesh->GetTextureCoordinate(0,i);
						std::cout << uv[0] << " " << uv[1] << " " << uv[2] << " " << uv[3] << "\n";
					}
				}
			}
			////////////
		}
*/
/*
#else
		aftkStatus = pAftkScene->Read("./input.a3d", Aftk::E_FileType_BestGuess, options);
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;
//		std::cout << pAftkScene->GetSceneTransform()[0][0] << "," << pAftkScene->GetSceneTransform()[0][1] << "," << pAftkScene->GetSceneTransform()[0][2] << "," << pAftkScene->GetSceneTransform()[0][3] << std::endl;
//		std::cout << pAftkScene->GetSceneTransform()[1][0] << "," << pAftkScene->GetSceneTransform()[1][1] << "," << pAftkScene->GetSceneTransform()[1][2] << "," << pAftkScene->GetSceneTransform()[1][3] << std::endl;
//		std::cout << pAftkScene->GetSceneTransform()[2][0] << "," << pAftkScene->GetSceneTransform()[2][1] << "," << pAftkScene->GetSceneTransform()[2][2] << "," << pAftkScene->GetSceneTransform()[2][3] << std::endl;
//		std::cout << pAftkScene->GetSceneTransform()[3][0] << "," << pAftkScene->GetSceneTransform()[3][1] << "," << pAftkScene->GetSceneTransform()[3][2] << "," << pAftkScene->GetSceneTransform()[3][3] << std::endl;

		std::cout << "Writing native endian file." << std::endl;
		aftkStatus = pAftkScene->Write("./output.a3d", Aftk::E_FileType_A3D);
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;

		std::cout << "Reading native endian file." << std::endl;
		aftkStatus = pAftkScene->Read("./output.a3d", Aftk::E_FileType_BestGuess, options);
		if (!aftkStatus)
			std::cout << "Bollocks." << aftkStatus.GetErrorString() << std::endl;

#endif
		//aftkStatus = pAftkScene->Read("F:/TheGoodAndTheBad/thegood.mb", E_FileType_BestGuess, &nonFatalErrors, options);
	//	aftkStatus = pAftkScene->Read("f:/testing/texcube.a3d", E_FileType_BestGuess);
	//	aftkStatus = pAftkScene->Read("f:/160/Tokyo_Main_Only.a3d", E_FileType_BestGuess);
	//	aftkStatus = pAftkScene->Read("Gotham3/Art/Circuits/Las Vegas/Models/Routes/Route_01.mb", E_FileType_BestGuess, &nonFatalErrors, options);
	//	aftkStatus = pAftkScene->Read("f:/fixme/trees_input.a3d", E_FileType_BestGuess);
	//	aftkStatus = pAftkScene->Read("f:/170/tantest.mb");
	//	aftkStatus = pAftkScene->Read("shitstorm/Assets/Objects/Characters/Skins/PunkRogers/MayaWork/low+normal02.mb", E_FileType_BestGuess, &nonFatalErrors, options);
	//	aftkStatus = pAftkScene->Read("f:/160/cube65.a3d");
	//	aftkStatus = pAftkScene->Read("f:/trime/exp.a3d");
	}

	for (U32 i = 0; i < pAftkScene->GetNumMaterialNodes(); i++)
	{
		const Aftk::C_MaterialResource* pMatRes = pAftkScene->GetMaterialResourcePtr(i);
		Aftk::C_MaterialNode::Output(pMatRes->GetContent());
	}

	pAftkScene->OutputSceneGraph();

	delete pAftkScene;
	Aftk::C_Global::Destroy();
	std::cout << "Destroyed." << std::endl;

	g_pDebugManager->Destroy();
	delete g_pDebugManager;

	#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
	#endif
	return 0;
/*
	aftkStatus = pAftkScene->Read("f:/trime/wtf2.a3d");
	AftkSceneNodeIterator it(pAftkScene);
	AftkSceneMeshNode* pMesh;
	std::cout << "start" << std::endl;
//	DWORD start = GetTickCount();
	while(it)
	{
		Aftk::C_SceneNode* pNode = it;
		if (pNode->GetType() == E_ObjType_SceneMesh)
		{
			pMesh = (AftkSceneMeshNode*)pNode;
//			pMesh->Repair();
			for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(0); iPoly++)
			{
				const C_Polygon* pPoly = &pMesh->GetPolygon(iPoly);
				if (pPoly->GetNumPolygonVertices() <= 4)
					continue;
				
				for (U32 i = 0; i < pPoly->GetNumPolygonVertices(); i++)
				{
					const AftkPolygonVertex& pv = pPoly->GetPolygonVertex(i);
					S32 posIndex = pv.GetPositionIndex();
					std::cout << " pvi " << pPoly->GetPolygonVertexIndex(i) << " pos " << posIndex << std::endl;
				}
				std::cout << std::endl;
//				F32 area = pPoly->GetArea(&pMesh->GetWorldMatrix());
				AftkTriangleIterator triIt(*pPoly, &pMesh->GetWorldMatrix());
				while(triIt != 0)
				{
					const U32* t = triIt;
					std::cout << t[0] << " " << t[1] << " " << t[2] << std::endl;
					triIt++;
				}
			}
		}
		it++;
	}
*/
/*
	AftkSceneNodeIterator it(pAftkScene);
	{
		while(it)
		{
			Aftk::C_SceneNode* pSceneNode = it;

			U32 numLayers = pSceneNode->GetNumLayers();

			std::cout << "Object is: " << pSceneNode->GetName() << " ";
			for (U32 iLayer = 0; iLayer < numLayers; iLayer ++) {

				AftkLayer* currentLayer = pSceneNode->GetLayerPtr(iLayer);
				std::cout << " C_LayerNode is: " << currentLayer->GetName() << " ";

			}

			std::cout << std::endl;

			it++;
		}
	}

	return 0;

	}

	std::cout << "optimising..." << std::endl;
	U32 originalCount = pAftkScene->GetNumBlindDataNodes();
	std::cout << pAftkScene->DeleteUnusedBlindDataNodes() << " removed out of " << originalCount << std::endl;
	
//	DWORD end = GetTickCount();
//	DWORD time = end - start;
//	std::cout << "time : " << time << std::endl;

//	ConvertNormalInstancesToRemappedInstances(pAftkScene);
	
//	aftkStatus = pAftkScene->Read("f:/fixme/tokyo/route06.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//	aftkStatus = pAftkScene->Read("Gotham3/Art/Circuits/London/Models/Routes/route11.mb", E_FileType_BestGuess, &nonFatalErrors, options);
//	aftkStatus = pAftkScene->Read("f:/fixme/las vegas/lasvegas.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//	pAftkScene->Triangulate();
//	aftkStatus = pAftkScene->Read("Gotham3/Art/Circuits/Graphical/Export/GraphicalKen.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//	aftkStatus = pAftkScene->Read("f:/160/route01layer.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
	//g_aftkMemoryManager.CollectGarbage();
	std::cout << "C_Global alloc count " << g_globalAllocCount << std::endl;
	std::cout << "C_Global alloc size " << g_globalAllocSize << std::endl;
	pAftkScene->Write("f:/170/new.a3d", AFTK_FILE_TYPE_A3D);
	}

/*	WorkOutFreeMemory("After");

	std::cout << "press any key.\n";
	char c;
	std::cin >> c;
	std::cout << g_aftkMemoryManager.CollectGarbage() << " heaps freed\n";
	WorkOutFreeMemory("Collect");
	std::cout << "press any key.\n";
	std::cin >> c;
	
	delete pAftkScene;
	WorkOutFreeMemory("Dead");
	std::cout << g_aftkMemoryManager.CollectGarbage() << " heaps freed\n";
	WorkOutFreeMemory("Collect2");
	return 0;
*/
//	pAftkScene->Write("f:/160/unexpanded.a3d", AFTK_FILE_TYPE_A3D);

//	pAftkScene->ExpandUniqueRemappedInstances();

//	pAftkScene->Write("Gotham3/Art/Circuits/Graphical/Export/GraphicalKenExpanded.a3d", AFTK_FILE_TYPE_A3D);

//	std::cout << "read" << std::endl;

	/*
	{
		*g_pDebugManager << "C_Scene node count: " << pAftkScene->GetNumSceneNodes() << "\n";
		std::cout << "C_Scene node count: " << pAftkScene->GetNumSceneNodes() << "\n";
		Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > nodeList;
		U32 scenePolyCount = 0;
		{
			AftkSceneNodeIterator it(pAftkScene);
			{
				while(it)
				{
					Aftk::C_SceneNode* pSceneNode = it;
					*g_pDebugManager << "Dumping blind data for " << pSceneNode->GetName() << "\n";
					pAftkScene->OutputBlindData(pSceneNode->GetBlindDataResourcePtr());
					it++;
				}
			}
		}
	//	std::cout << "Poly count: " << scenePolyCount << "\n";
	//	delete pAftkScene;
	//	return 0;
	}
	
	delete pAftkScene;
	Aftk::C_Global::::Destroy();
	return 0;

//	pAftkScene->RemoveUnusedMaterialNodes();
//	pAftkScene->Optimise();
//	std::cout << "writing..." << std::endl;
//	pAftkScene->Write("f:/160/memory.a3d", AFTK_FILE_TYPE_A3D);
//	*g_pDebugManager << "C_Scene poly count: " << scenePolyCount << "\n";

	delete pAftkScene;
	Aftk::C_Global::::Destroy();
	g_pDebugManager->Destroy();
	delete g_pDebugManager;
	std::cout << "C_Global alloc count " << g_globalAllocCount << std::endl;
	std::cout << "C_Global alloc size " << g_globalAllocSize << std::endl;
	#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
	#endif
	return 0;
	/*
	{
		C_Scene temp;
		temp.Read("f:/fixme/Tokyo/Route06.a3d");
 		pAftkScene->Import(&temp);
	}
	pAftkScene->Write("Gotham3/Art/Circuits/Graphical/Export/graphicalkenhack_new.a3d", AFTK_FILE_TYPE_A3D);
	delete pAftkScene;
	return 0;
	{
		for (U32 i = 0; i < pAftkScene->GetNumLayers(); i++)
		{
			AftkLayer* pLayer = pAftkScene->GetLayerPtr(i);
			U32 polyCount = 0;
			for (U32 j = 0; j < pLayer->GetNumMembers(); j++)
			{
				Aftk::C_SceneNode* pSceneNode = pLayer->GetMemberPtr(j);
				if (pSceneNode->HasType(E_ObjType_SceneMesh))
				{
					AftkSceneMeshRemappableNode* pRemappable = 
						(AftkSceneMeshRemappableNode*)pSceneNode;
					polyCount += pRemappable->GetNumPolygons(0);
				}
			}
			*g_pDebugManager << pLayer->GetName() << " layer poly count " << polyCount << "\n";
		}
		AftkSceneNodeIterator it(pAftkScene);
		{
			U32 polyCount = 0;
			while(it)
			{
				Aftk::C_SceneNode* pSceneNode = it;
				if (pSceneNode->GetLayerPtr() == 0)
				{
					if (pSceneNode->HasType(E_ObjType_SceneMesh))
					{
						AftkSceneMeshRemappableNode* pRemappable = 
							(AftkSceneMeshRemappableNode*)pSceneNode;
						polyCount += pRemappable->GetNumPolygons(0);
					}
				}
				it++;
			}
			*g_pDebugManager << "NO LAYER poly count " << polyCount << "\n";
		}
	}

	for (U32 i = 0; i < pAftkScene->GetNumLayers(); i++)
	{
		AftkLayer* pLayer = pAftkScene->GetLayerPtr(i);
		*g_pDebugManager << pLayer->GetName() << "\n";
		for (U32 j = 0; j < pLayer->GetNumMembers(); j++)
		{
			Aftk::C_SceneNode* pSceneNode = pLayer->GetMemberPtr(j);
			*g_pDebugManager << "    " << pSceneNode->GetName();
			if (pSceneNode->HasType(E_ObjType_SceneMesh))
			{
				AftkSceneMeshRemappableNode* pRemappable = 
					(AftkSceneMeshRemappableNode*)pSceneNode;
				*g_pDebugManager << " Poly count " << pRemappable->GetNumPolygons(0);
			}
			*g_pDebugManager << "\n";
		}
	}

	{
		AftkSceneNodeIterator it(pAftkScene);
		{
			while(it)
			{
				Aftk::C_SceneNode* pSceneNode = it;
				if (pSceneNode->GetLayerPtr() == 0)
				{
					*g_pDebugManager << "NO LAYER " << pSceneNode->GetName();
					if (pSceneNode->HasType(E_ObjType_SceneMesh))
					{
						AftkSceneMeshRemappableNode* pRemappable = 
							(AftkSceneMeshRemappableNode*)pSceneNode;
						*g_pDebugManager << " Poly count " << pRemappable->GetNumPolygons(0);
					}
					*g_pDebugManager << "\n";
				}
				it++;
			}
		}
	}

	return 0;
	
//	pAftkScene->Triangulate();
//	pAftkScene->Write("f:/160/2expand.a3d", AFTK_FILE_TYPE_A3D);
//	aftkStatus = pAftkScene->Read("F:/rogbug/route05.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//	aftkStatus = pAftkScene->ExpandUniqueRemappedInstances();
//	pAftkScene->OutputSceneGraph();
/*
	{
		C_Scene temp;
//		aftkStatus = temp.Read("f:/160/barrier.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//		temp.Write("f:/160/barrier.a3d", AFTK_FILE_TYPE_A3D);
//		aftkStatus = temp.Read("f:/160/garage07.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
		//aftkStatus = temp.Read("f:/testing/texcube.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//		pAftkScene->SetSceneTransform(temp.GetSceneTransform());
//		pAftkScene->Import(&temp);
		C_Scene temp2;
		aftkStatus = temp2.Read("F:/160/Bendy/route01.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//		aftkStatus = temp2.Read("F:/160/barriertest.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
		pAftkScene->Import(&temp2);
//		DumpSkinningInfo(&temp2);

		aftkStatus = temp.Read("F:/160/Bendy/route02.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
//		aftkStatus = temp.Read("F:/160/barriertest.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
		pAftkScene->Import(&temp);
//		DumpSkinningInfo(&temp);
		int x = 5;
		DumpSkinningInfo(pAftkScene);
	}
	*/
//	if (pAftkScene->Validate() != 0)
//	{
//		int x = 5;
//	}
//	pAftkScene->Write("f:/160/expand.a3d", AFTK_FILE_TYPE_A3D);
//	return 0;
	
//	pAftkScene->Write("f:/testing/import.a3d", AFTK_FILE_TYPE_A3D);

//	pAftkScene->Read("Gotham3/Art/Circuits/Tokyo/Models/Routes/Route01.mb", E_FileType_BestGuess, &nonFatalErrors, options);
//	pAftkScene->OutputSceneGraph();
//pAftkScene->Read("f:/testing/import.a3d", AFTK_FILE_TYPE_A3D);
//	if (pAftkScene->Validate() == 0)
//	{
//		pAftkScene->Write("f:/160/merge.a3d", AFTK_FILE_TYPE_A3D);
//	}
/*	{
		U32 rootCount = 0;
		Ut::UtVector< Ut::DynamicVector<Aftk::C_SceneNode*> > rootList;
		AftkSceneNodeIterator it(pAftkScene);
		while (it)
		{
			Aftk::C_SceneNode* pNode = it;
			if (pNode->GetName() == "AftkRoot")
			{
				std::cout << "Root" << std::endl;
				rootCount++;
				if (rootCount > 1) 
					if (rootCount < 14)
						rootList.PushBack(pNode);
			}
			it++;
		}
		
		for (U32 i = 0; i < (U32)rootList.Size(); i++)
		{
			pAftkScene->DeleteSceneNodeAndChildren(rootList[i]);
		}

		pAftkScene->Write("f:/rogbug/repro.a3d", AFTK_FILE_TYPE_A3D);
	}
	

//			if (pNode->GetName() == "eject_END")
//			{
//				int x = 5;
//			}

/*


	Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > newNodes;
	{
		AftkSceneNodeIterator it(pAftkScene);
		while (it)
		{
			Aftk::C_SceneNode* pNode = it;

			if (pNode->HasType(E_ObjType_SceneMeshRemappable))
			{
//				if (pNode->GetName() == "INSTANCE08")
				{
					AftkSceneMeshRemappableNode* pBoned = (AftkSceneMeshRemappableNode*)pNode;
					if (pNode->GetName() == "INSTANCE08")
					{
						int x = 5;
					}
//					AftkSceneMeshNode* pMesh = new AftkSceneMeshNode(*pBoned);
					for (U16 iMat = 0; iMat < pBoned->GetNumPolygonMaterialMappings(); iMat++)
					{
						const AftkMaterialResource* pRes = 
							pBoned->GetPolygonMaterialMappingPtr(iMat);

						const AftkShaderImageNode* pImage = pRes->GetContent().GetShaderImageNodePtr(1);

						*g_pDebugManager << "p:" << pBoned->GetParentPtr()->GetName() << " " << pBoned->GetName() << " " << pRes->GetContent().GetName() << " " << pImage->GetFileName() << "\n";
					}

					AftkMatrix44 newMatrix = pMesh->GetLocalMatrix();
					newMatrix[3][1] += 2.f;
					pMesh->SetLocalMatrix(newMatrix);
					std::string newName = pMesh->GetName();
					newName += "_copy";
					pMesh->SetName(newName);
					newNodes.PushBack(pMesh);
					
				}
			}
			it++;
		}
	}

	for (U32 i =0 ; i< (U32)newNodes.Size(); i++)
	{
		pAftkScene->GetRootSceneNodePtr()->AddChildPtr(newNodes[i]);
	}

*/
//	std::cout << "2..." << std::endl;
//	pAftkScene->Validate();
//	pAftkScene->CalculateTangentsAndBinormals(true);
//	pAftkScene->Write("f:/testing/uv.a3d", AFTK_FILE_TYPE_A3D);
/*	aftkStatus = pAftkScene->Read("f:/160/n00borig.a3d", E_FileType_BestGuess, &nonFatalErrors, options);
	std::cout << "3..." << std::endl;
	pAftkScene->Validate();
	std::cout << "4..." << std::endl;
/*
	std::cout << "start" << std::endl;
	DWORD start = GetTickCount();
	pAftkScene->WeldMeshComponents();
	pAftkScene->WeldMeshPolygonVertices();
	DWORD end = GetTickCount();
	DWORD time = end - start;
	std::cout << "time : " << time << std::endl;

	delete pAftkScene;
	return 0;

	std::cout << "start" << std::endl;
	DWORD start = GetTickCount();
	pAftkScene->CalculateTangentsAndBinormals(90.f, 90.f, false, true);
	DWORD end = GetTickCount();
	DWORD time = end - start;
	std::cout << "time : " << time << std::endl;
//	aftkStatus = pAftkScene->Write("f:/EdBug/meh.a3d", AFTK_FILE_TYPE_A3D);

//	aftkStatus = pAftkScene->Read("Gotham3/Art/Circuits/nurburgring/models/buildings/ground_mesh3.mb", E_FileType_BestGuess, &nonFatalErrors);
//	pAftkScene->WeldMeshes();
//	aftkStatus = pAftkScene->Read("C:/Documents and Settings/kenm/My Documents/maya/projects/default/scenes/uvset.mb");
//	aftkStatus = pAftkScene->Read("F:/ChadBug/ClusterTest.xsi");
//	aftkStatus = pAftkScene->Read("f:/160/barrier_group.a3d");
	AFTK_ASSERT(pAftkScene->Validate() == 0);
//	aftkStatus = pAftkScene->Read("f:/testing/path.a3d");
	if (!aftkStatus)
	{
		*g_pDebugManager << aftkStatus.GetErrorString() << "\n";
	}

	for (U32 iLayer = 0; iLayer < pAftkScene->GetNumLayers(); iLayer++)
	{
		const AftkLayer* pAftkLayer = pAftkScene->GetLayerPtr(iLayer);
		*g_pDebugManager << "<LAYER=" << pAftkLayer->GetName() << ">\n";
		for (U32 jNode = 0; jNode < pAftkLayer->GetNumMembers(); jNode++)
		{
			const Aftk::C_SceneNode* pNode = pAftkLayer->GetMemberPtr(jNode);
			*g_pDebugManager << jNode << " [" << pNode->GetName() << "]\n";
		}
		*g_pDebugManager << "</LAYER=" << pAftkLayer->GetName() << ">\n";
	}


	*g_pDebugManager << "<NO LAYER>\n";
	AftkSceneNodeIterator it(pAftkScene);
	U32 iNode = 0;
	while (it)
	{
		Aftk::C_SceneNode* pNode = it;
		if (pNode->GetLayerPtr() == 0)
		{
			*g_pDebugManager << iNode << " [" << pNode->GetName() << "]\n";
			iNode++;
		}
		it++;
	}
	*g_pDebugManager << "</NO LAYER>\n";

//	delete pAftkScene;
//	return 0;


	{
		AftkSceneNodeIterator it(pAftkScene);
		while (it)
		{
			Aftk::C_SceneNode* pNode = it;

			if (pNode->HasType(E_ObjType_SceneMesh))
			{
				AftkSceneMeshNode* pMesh = (AftkSceneMeshNode*) pNode;
				for (U32 iLodLevel = 0; iLodLevel < pMesh->GetNumLodLevels(); iLodLevel++)
				{
					for (U32 jPoly = 0; jPoly < pMesh->GetNumPolygons(iLodLevel); jPoly++)
					{
						const C_Polygon* pPoly = &pMesh->GetPolygon(iLodLevel, jPoly);
						std::cout << pPoly->GetArea() << std::endl;
					}
				}
			}
		
			if (pNode->HasType(E_ObjType_SceneMeshBonedInstance))
			{
				AftkSceneMeshBonedInstanceNode* pBonedMesh = (AftkSceneMeshBonedInstanceNode*)pNode;

				AftkSceneMeshNode* pMesh = pBonedMesh->GetSourceNodePtr();
				pMesh->BuildAnimationHelpers();
				for (U32 iLodLevel = 0; iLodLevel < pMesh->GetNumLodLevels(); iLodLevel++)
				{
					U32 masterIndex = 0;
					*g_pDebugManager << "BLOD LEVEL " << iLodLevel << "\n";
					for (U32 jPoly = 0; jPoly < pMesh->GetNumPolygons(iLodLevel); jPoly++)
					{
						const C_Polygon* pPoly = &pMesh->GetPolygon(iLodLevel, jPoly);
						for (U32 kPV = 0; kPV < pPoly->GetNumPolygonVertices(); kPV++)
						{
							S32 posIndex = pPoly->GetPolygonVertex(kPV).GetPositionIndex();
							const AftkVertexWeight& weight = pMesh->GetVertexWeight(posIndex);
							*g_pDebugManager << " PV Index " <<pPoly->GetPolygonVertexIndex(kPV) << " pos index " << posIndex << " Pos: " << pMesh->GetVertexPosition(posIndex) << 
										"W " << weight.GetInfluenceNodeIndex(0) << " " << weight.GetWeight(0) << "\n";
									masterIndex++;
						}
					}
				}
			}

			if (pNode->HasType(E_ObjType_SceneLodGroup))
			{
				AftkSceneLodGroupNode* pGroup = (AftkSceneLodGroupNode*)pNode;
				for (U32 iLodLevel = 0; iLodLevel < pGroup->GetNumLodLevels(); iLodLevel++)
				{
					U32 masterIndex = 0;
					AftkGroupLodLevelInfo& info = pGroup->GetLodLevelInfo(iLodLevel);

					*g_pDebugManager << "GLOD LEVEL " << iLodLevel << "\n";

					AftkSceneMeshBonedInstanceNode* pBonedMesh = (AftkSceneMeshBonedInstanceNode*)info.GetSceneNodePtr();
					AftkSceneMeshNode* pMesh = pBonedMesh->GetSourceNodePtr();
					pBonedMesh->BuildAnimationHelpers();
					for (U32 jPoly = 0; jPoly < pMesh->GetNumPolygons(0); jPoly++)
					{
						const C_Polygon* pPoly = &pMesh->GetPolygon(0, jPoly);
						for (U32 kPV = 0; kPV < pPoly->GetNumPolygonVertices(); kPV++)
						{
							S32 posIndex = pPoly->GetPolygonVertex(kPV).GetPositionIndex();
							const AftkVertexWeight& weight = pMesh->GetVertexWeight(posIndex);
							*g_pDebugManager << " PV Index " <<pPoly->GetPolygonVertexIndex(kPV) << " pos index " << posIndex << " Pos: " << pMesh->GetVertexPosition(posIndex) << 
										"W " << weight.GetInfluenceNodeIndex(0) << " " << weight.GetWeight(0) << "\n";
									masterIndex++;
						}
					}
				}
			}

			it++;
		}
	}
	return 0;

	std::cout << "done" <<  std::endl;
	pAftkScene->RepairMeshNodes();
	AFTK_ASSERT(pAftkScene->Validate() == 0);
	aftkStatus = pAftkScene->Write("f:/janbug/slow.a3d", AFTK_FILE_TYPE_A3D);

	std::cout << "done" <<  std::endl;
	aftkStatus = pAftkScene->Read("f:/janbug/slow.a3d");
	AFTK_ASSERT(pAftkScene->Validate() == 0);

	{
		AftkSceneNodeIterator it(pAftkScene);
		AftkSceneMeshNode* pCube1 = (AftkSceneMeshNode*)pAftkScene->GetSceneNode("pCube1");
		Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > toDelete;
		while (it)
		{
			Aftk::C_SceneNode* pNode = it;
			if (pNode->GetName() != "pCube1")
			{
				if (pNode->HasType(E_ObjType_SceneMesh))
				{
					AftkSceneMeshNode* pMesh = (AftkSceneMeshNode*)pNode;
					pCube1->AddMesh(pMesh);
					toDelete.PushBack(pMesh);
				}
			}
			it++;
		}
		for (U32 i = 0; i < (U32)toDelete.Size(); i++)
		{
			pAftkScene->DeleteSceneNodeAndReparentChildren(toDelete[i], false);
		}
	}
	aftkStatus = pAftkScene->Write("f:/160/test.a3d", AFTK_FILE_TYPE_A3D);

//	const Aftk::C_SceneNode* pTest = pAftkScene->GetSceneNodeByPartialPath("|pPlane1|pCube1|pSphere1");
	
	//aftkStatus = pAftkScene->Read("C:/Documents and Settings/kenm/My Documents/maya/projects/default/scenes/ReferenceTest/IamUsedOnce.mb");

	//aftkStatus = pAftkScene->Read("F:/barriers/barriertest.mb");

//	aftkStatus = pAftkScene->Read("Gotham3/Art/Circuits/Tokyo/Models/paulbug.mb");
//	aftkStatus = pAftkScene->Read("Gotham3/Art/Circuits/Tokyo/Models/paulbug2.mb");

	*g_pDebugManager << "*** Dumping active x-ref scenes ***\n";
	for (U32 iRef = 0; iRef < pAftkScene->GetNumActiveXRefScenes(); iRef++)
	{
		const C_Scene::AftkActiveXRefEntry* pAftkXRefEntry = 
			((const C_Scene*)pAftkScene)->GetActiveXRefEntryPtr(iRef);
		*g_pDebugManager << "[" << iRef << "] " << pAftkXRefEntry->pXRefScene->GetFileName() << " CRC " << pAftkXRefEntry->pXRefScene->GetCRC() << " count [" << pAftkXRefEntry->usageList.Size() << "]\n";
		
	}
	*g_pDebugManager << "***********************************\n";

	*g_pDebugManager << "*** Dumping x-ref scenes ***\n";
	for (U32 iRef = 0; iRef < pAftkScene->GetNumXRefScenes(); iRef++)
	{
		const AftkXRefScene* pAftkXRefScene = 
			((const C_Scene*)pAftkScene)->GetXRefScenePtr(iRef);
		*g_pDebugManager << "[" << iRef << "] " << pAftkXRefScene->GetFileName() << "\n";
	}
	*g_pDebugManager << "***********************************\n";

//	aftkStatus = pAftkScene->LoadAllXRefScenes();
/*
	pAftkScene->OutputSceneGraph();
*/
	/*
	exportOptions.AddAttribute(AftkGenericAttribute());
	{
		AftkGenericAttribute* pAttr = &exportOptions.GetAttribute(0);
		pAttr->SetName("metadata");
		pAttr->SetValue("metatest");
	}
	aftkStatus = pAftkScene->Write("f:/testing/xref.a3d", AFTK_FILE_TYPE_A3D, 0, exportOptions);
	*/
/*
	pAftkScene->Clear();

	aftkStatus = pAftkScene->Read("f:/testing/xref.a3d");

	aftkStatus = pAftkScene->LoadAllXRefScenes();
*/
	/*
	*g_pDebugManager << "*** Dumping active x-ref scenes ***\n";
	for (U32 iRef = 0; iRef < pAftkScene->GetNumActiveXRefScenes(); iRef++)
	{
		const C_Scene::AftkActiveXRefEntry* pAftkXRefEntry = 
			((const C_Scene*)pAftkScene)->GetActiveXRefEntryPtr(iRef);
		*g_pDebugManager << "[" << iRef << "] " << pAftkXRefEntry->pXRefScene->GetFileName() << " count [" << pAftkXRefEntry->usageList.Size() << "]\n";
	}
	*g_pDebugManager << "***********************************\n";

	*g_pDebugManager << "*** Dumping x-ref scenes ***\n";
	for (U32 iRef = 0; iRef < pAftkScene->GetNumXRefScenes(); iRef++)
	{
		const AftkXRefScene* pAftkXRefScene = 
			((const C_Scene*)pAftkScene)->GetXRefScenePtr(iRef);
		*g_pDebugManager << "[" << iRef << "] " << pAftkXRefScene->GetFileName() << "\n";
	}
	*g_pDebugManager << "***********************************\n";
*/
//	pAftkScene->OutputSceneGraph();

//	pAftkScene->UnloadAllXRefScenes();
/*	
	AFTK_ASSERT(aftkStatus);
	U32 num = pAftkScene->Validate();
	std::cout << num << "\n";
	*g_pDebugManager << "File loaded\n";

//	aftkStatus = pAftkScene->Write("F:/testing/xref/a3d");

	*g_pDebugManager << "Done\n";
	//getchar();
	delete pAftkScene;

	g_pDebugManager->Destroy();
	delete g_pDebugManager;

	#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
	#endif

#ifdef AFTK_MAYA_IO
	MLibrary::cleanup(0);
#endif
}
*/
