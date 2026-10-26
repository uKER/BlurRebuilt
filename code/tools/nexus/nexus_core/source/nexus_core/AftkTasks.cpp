#include "stdafx.h"
#ifdef NEXUS_MAYA
#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>
#include <maya/MStringArray.h>
#endif

#include <nexus_core/core.h>

#include "aftkut/aftkut.h"
#include <metadata/metadata.h>
#include <core/utility/stringproc.h>
//Contains chdir function.
#include <direct.h>

// Contains the meta manager api.
#include <system/misc/StringUtil.h>
#include <file_tag_db/FileTagDB.h>
#include <tools_shared/VersionStrings.h>
#include <tools_shared/utility/Misc.h>
#include <tools_shared/utility/AssetMeta.h>

namespace Tools
{
bool MetaMismatch(
	const Util::C_String& outputFileName,
	const Util::C_String& toolFileName)
{
	return true;
}

}
namespace Nexus
{

bool SaveMeta(const Util::C_String& mainInput, const Util::C_String& output)
{
#ifdef NEXUS_MAYA
	Ut::Vector<FileTag::C_MetaTag::C_Input> inputs;

	//Add references.
	Aftk::E_FileType fileType;
	Aftk::C_Global::GetFileType(mainInput.Chars(), fileType);
	if ((fileType == Aftk::E_FileType_Mb || fileType == Aftk::E_FileType_Ma))
	{
		assert(Nexus::g_pCore->IsMayaInitialised());
		MStringArray arrResult;
		MCommandResult result;
		MStatus mStatus = MGlobal::executeCommand("file -q -l", result, true, false);
		if (!mStatus) false;
		mStatus = result.getResult(arrResult);
		if (!mStatus) false;

		for (U32 i = 0; i < arrResult.length(); ++i)
		{
			Aftk::C_Global::GetFileType(arrResult[i].asChar(), fileType);
			if ((fileType == Aftk::E_FileType_Mb || fileType == Aftk::E_FileType_Ma))
			{
				inputs.PushBack();
				FileTag::C_MetaTag::C_Input& input = inputs.Back();
				input.file = arrResult[i].asChar();

				WIN32_FILE_ATTRIBUTE_DATA attribs;
				if (!GetFileAttributesEx(input.file.Chars(), GetFileExInfoStandard, &attribs)) 
					return Tu::Status::FAILURE;
				input.fileTime = (S64)(attribs.ftLastWriteTime.dwHighDateTime)<<32;
				input.fileTime |= attribs.ftLastWriteTime.dwLowDateTime;
			}
		}
	}
	else
	{
		//Add main input file.
		inputs.PushBack();
		FileTag::C_MetaTag::C_Input& input = inputs.Back();
		input.file = mainInput;
		WIN32_FILE_ATTRIBUTE_DATA attribs;
		if (!GetFileAttributesEx(input.file.Chars(), GetFileExInfoStandard, &attribs)) 
			return Tu::Status::FAILURE;
	}

	FileTag::C_MetaTag metaTag;
	if (FileTag::C_MetaManager::Instance().AddTag(metaTag, output))
	{
		metaTag.SetBuildTime(Tools::GetCurrentExeTime());
		metaTag.SetInputs(inputs);
	}

	//Flush databases to disk.
	FileTag::C_MetaManager::Instance().Write();
#endif
	return true;
}


Tu::Status PrepareSceneForMayaConversion(Progress* pProgressSystem)
{
	if (pProgressSystem)
	{
		pProgressSystem->OnProgress(0);
		pProgressSystem->SetTitle("Renaming mesh nodes...");
	}
	Nexus::g_pCore->GetAftkScenePtr()->MakeUniqueSceneNodeNames();

	//Validate that the contents of the loaded scene are sane.
	if (pProgressSystem)
	{
		pProgressSystem->OnProgress(0);
		pProgressSystem->SetTitle("Repairing scene....");
		if (Nexus::g_pCore->GetAftkScenePtr()->Repair(pProgressSystem) != 0)
			return Tu::Status(Tu::Status::FAILURE, "Could not repair scene prior to saving.");
	}

	Aftk::C_Status status = Aftk::C_SceneNodeVisit::Traverse(
		Nexus::g_pCore->GetAftkScenePtr(), 
		Aftk::C_MeshExpandPolygonVertices::C_ExpandPolygonVerticesVisitor(),
		(Aftk::I_Progress*)pProgressSystem);
	
	//Force \n character.
	if (pProgressSystem)
		pProgressSystem->SetTitle("");
	return Tu::Status::SUCCESS;
}

	
void AftkTasks::NewScene()
{
	//Create renderer if needed.
	Nexus::g_pCore->Clear();
}

Tu::Status OpenScene(
	void* pProgress,
	Aftk::C_Scene* pScene,
	const StringArray& arguments)
{
	if (arguments.Size() < 1)
		return Tu::Status(Tu::Status::FAILURE, "Insufficient arguments.");

	Progress* pProgressSystem = (Progress*)pProgress;
	Aftk::C_Status status;

	Aftk::E_FileType fileType;
	status = Aftk::C_Global::GetFileType(arguments[0], fileType);
	if (!status)
	{
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	#ifdef NEXUS_MAYA
	if ((fileType == Aftk::E_FileType_Mb) || (fileType == Aftk::E_FileType_Ma))
	{
		Tu::Status tuStatus;
		if (!Nexus::g_pCore->IsMayaInitialised())
		{
			tuStatus = Nexus::g_pCore->InitialiseMaya();
			if (!tuStatus) return tuStatus;
		}
	}
	#endif

	const char* args;
	if (arguments.Size() > 1)
		args = arguments[1].c_str();
	else
		args = "";

	status = pScene->Read(
		arguments[0],
		Aftk::E_FileType_BestGuess,
		args,
 		pProgressSystem);

//	pScene->SetSlerpRotation(false);
	
	//We didn't cancel the task, and we didn't succeed either.
	if (!status)
	{
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	Nexus::g_pCore->SetIsSceneViewDirty(true);
	return Tu::Status::SUCCESS;
}

Tu::Status OpenExternalScene(
	void* pProgress,
	Aftk::C_Scene* pScene,
	const StringArray& arguments)
{
	Progress* pProgressSystem = (Progress*)pProgress;
	Aftk::C_Status status;

	//Determine where to read scene data from.
	Aftk::E_ExternalScene source;
	if (!_stricmp(arguments[0].c_str(), "maya"))
	{
		source = Aftk::E_ExternalScene_Maya;
	}
	else
	{
		return Tu::Status(Tu::Status::FAILURE, "Failed to determine source external scene.");
	}

	#ifdef NEXUS_MAYA
	if (source == Aftk::E_ExternalScene_Maya)
	{
		Tu::Status tuStatus;
		if (!Nexus::g_pCore->IsMayaInitialised())
		{
			tuStatus = Nexus::g_pCore->InitialiseMaya();
			if (!tuStatus) return tuStatus;
		}
	}
	#endif

	if (pProgressSystem)
		pProgressSystem->SetTitle("Opening from external scene...");

	status = Nexus::g_pCore->GetAftkScenePtr()->ReadExternalScene(
		source,
		arguments[1].c_str(),
		(Aftk::I_Progress*)pProgress);

	//We didn't cancel the task, and we didn't succeed either.
	if (!status)
	{
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	Nexus::g_pCore->SetIsSceneViewDirty(true);
	return Tu::Status::SUCCESS;
}


Tu::Status AftkTasks::CalculateTangentsAndBinormals(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	if (arguments.Size() > 0)
	{
		if (arguments.Size() != 1)
		{
			Nexus::g_pCore->Unlock();
			return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);
		}
	}
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Calculating tangents and binormals...");
	}

	bool normalise;
	if (arguments.Size() == 1)
	{
		if (arguments[0] == "true")
			normalise = true;
		else
			normalise = false;
	}

	Aftk::C_MeshCalculateTangents::C_CalculateTangentsVisitor visitor;
	visitor.normalise = normalise;
	visitor.useVertexNormal = false;
	Aftk::C_Status status = Aftk::C_SceneNodeVisit::Traverse(
		Nexus::g_pCore->GetAftkScenePtr(), 
		visitor,
		(Aftk::I_Progress*)pProgress);

	if (!status)
	{
		Nexus::g_pCore->Unlock();
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	Nexus::g_pCore->OnSceneGeometryChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::OpenScene(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Loading scene...");
	}

	Tu::Status status;

	status = Nexus::OpenScene(
		pProgress,
		Nexus::g_pCore->GetAftkScenePtr(),
		arguments);
	//We didn't cancel the task, and we didn't succeed either.
	if (!status) 
	{
		Nexus::g_pCore->Unlock();
		return status;
	}

	{
		//Set our working directory to that of the file we've just opened.
		std::string dir = Tu::DirFromString(arguments[0]);
		if (_chdir(dir.c_str()) == -1)
			Nexus::g_pCore->PrintWarning("Could not set working directory.");
		Nexus::g_pCore->SetIsSceneLoaded(true);
	}

	Nexus::g_pCore->SetSceneFileName(arguments[0]);
	Nexus::g_pCore->OnSceneChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::ImportScene(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Importing scene...");
	}

	Tu::Status status;
	Aftk::C_Scene* pSceneToImport = new Aftk::C_Scene();
	status = Nexus::OpenScene(
		pProgressSystem,
		pSceneToImport,
		arguments);
	//We didn't cancel the task, and we didn't succeed either.
	if (!status) 
	{
		delete pSceneToImport;
		Nexus::g_pCore->Unlock();
		return status;
	}

	Aftk::C_Status aftkStatus =
		AftkUt::C_Import::Import(
		Nexus::g_pCore->GetAftkScenePtr(),
		pSceneToImport, 
		pProgressSystem);
	//We didn't cancel the task, and we didn't succeed either.
	if (!aftkStatus)
	{
		delete pSceneToImport;
		Nexus::g_pCore->Unlock();
		return Tu::Status(
			Tu::Status::FAILURE, 
			aftkStatus.GetSeverity(),
			aftkStatus.GetErrorString());
	}

	delete pSceneToImport;

	{
		Nexus::g_pCore->SetIsSceneLoaded(true);
	}

	Nexus::g_pCore->OnSceneChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::ConvertScene(
	void* pProgress,
	const StringArray& arguments)
{
	if (arguments.Size() < 4)
		return Tu::Status(Tu::Status::FAILURE, "Insufficient arguments.");

	StringArray openArgs;
	openArgs.PushBack(arguments[0]); //Filename
	openArgs.PushBack(arguments[1]); //args
	StringArray saveArgs;
	saveArgs.PushBack(arguments[2]); //Filename
	saveArgs.PushBack(arguments[3]); //args

	Tu::Status status;

	//********************************
	// Build checking.
	//********************************
	Util::C_String filePath = openArgs[0].c_str();
	Util::C_String inFileName = Util::ExtractFilename(openArgs[0].c_str());
	Util::C_String outFileName = Util::ExtractFilename(saveArgs[0].c_str());

	bool forceConversion = false;
	//TODO: Implement a generic args parser, move away from this hacky rubbish.
	if (arguments.Size() > 4)
	{
		if (!_stricmp(arguments[4].c_str(), "forceconversion=true"))
		{
			GlobalTasks::Print("Force conversion specified.\n");
			forceConversion = true;
		}
	}

	// Obtain the modification time for the exe
	// Use this value as the build time for Nexus
	CHAR exeFileName[256];
	GetModuleFileName(0,exeFileName,256);
	WIN32_FILE_ATTRIBUTE_DATA exeFileAttribs;
	if (!GetFileAttributesEx(exeFileName, GetFileExInfoStandard, &exeFileAttribs))
		forceConversion = true;
	
	if (!forceConversion && !Tools::MetaMismatch(saveArgs[0].c_str(), exeFileName))
	{
		GlobalTasks::Print("Metadata match - no need for conversion.\n");
		return Tu::Status::SUCCESS;
	}

	status = OpenScene(pProgress, openArgs);
	if (!status) return status;

	status = SaveScene(pProgress, saveArgs);
	if (!status) return status;

	if (!SaveMeta(openArgs[0].c_str(), saveArgs[0].c_str()))
		GlobalTasks::Print("Failed to write metadata!\n");

	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::RepairScene(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
		pProgressSystem->SetTitle("Repairing scene...");

	Nexus::g_pCore->GetAftkScenePtr()->Repair((Aftk::I_Progress*)pProgress);
	
	Nexus::g_pCore->OnSceneGeometryChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::WeldMesh(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	if (arguments.Size() > 0)
	{
		if (arguments.Size() != 1)
		{
			Nexus::g_pCore->Unlock();
			return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);
		}
	}

	F32 weldThreshold = AFTK_EPSILON;
	if (arguments.Size() == 1)
	{
		//Copy welding threshold.
		{
			std::stringstream ss; ss << arguments[0];
			ss >> weldThreshold;
		}
	}

	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		std::stringstream ss; ss << "Welding mesh with tolerance" << weldThreshold << "...";

		pProgressSystem->SetTitle(ss.str().c_str());
	}

	{
		Aftk::C_MeshWeld::C_WeldVisitor visitor;
		visitor.positionThreshold	= weldThreshold;
		visitor.normalThreshold		= weldThreshold;
		visitor.uvThreshold			= weldThreshold;
		visitor.tangentThreshold	= weldThreshold;

		Aftk::C_MeshWeldMemoryManager weldMemoryManager;

		Aftk::C_SceneNodeVisit::Traverse(
			g_pCore->GetAftkScenePtr(),
			Aftk::C_MeshWeld::C_WeldVisitor(),
			weldMemoryManager, 
			(Aftk::I_Progress*)pProgressSystem);
	}

	Nexus::g_pCore->OnSceneGeometryChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::WeldMeshComponents(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Welding mesh components...");
	}

	{
		Aftk::C_MeshWeldMemoryManager weldMemoryManager;

		Aftk::C_SceneNodeVisit::Traverse(
			g_pCore->GetAftkScenePtr(),
			Aftk::C_MeshWeld::C_WeldComponentsVisitor(),
			weldMemoryManager, 
			(Aftk::I_Progress*)pProgress);
	}

	Nexus::g_pCore->OnSceneGeometryChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::WeldMeshPolygonVertices(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Welding mesh polygon vertices...");
	}

	{
		Aftk::C_MeshWeldMemoryManager weldMemoryManager;

		Aftk::C_SceneNodeVisit::Traverse(
			g_pCore->GetAftkScenePtr(),
			Aftk::C_MeshWeld::C_WeldPolygonVerticesVisitor(),
			weldMemoryManager, 
			(Aftk::I_Progress*)pProgress);
	}

	Nexus::g_pCore->OnSceneGeometryChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::SaveScene(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;

	Aftk::C_Status status;
	Aftk::E_FileType format;
	status = Aftk::C_Global::GetFileType(arguments[0], format);
	if (!status)
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	
	//Perform additional pre-processing by default if we're to be saving to Maya.
	if (format == Aftk::E_FileType_Mb || format == Aftk::E_FileType_Ma)
		PrepareSceneForMayaConversion(pProgressSystem);

	if (pProgressSystem)
		pProgressSystem->SetTitle("Saving scene...");
	status = Nexus::g_pCore->GetAftkScenePtr()->Write(
		arguments[0],
		format,
		0,
		0,
		(Aftk::I_Progress*)pProgress);

	//We didn't cancel the task, and we didn't succeed either.
	if (!status)
	{
		Nexus::g_pCore->Unlock();
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	Nexus::g_pCore->SetSceneFileName(arguments[0]);
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::SaveExternalScene(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;

	Aftk::C_Status status;

	//Determine where the scene is to be saved.
	Aftk::E_ExternalScene target;
	if (!_stricmp(arguments[0].c_str(), "maya"))
		target = Aftk::E_ExternalScene_Maya;
	else
		target = Aftk::E_ExternalScene_Unknown;

	#ifdef NEXUS_MAYA
	if (target == Aftk::E_ExternalScene_Maya)
	{
		if (pProgress)
			pProgressSystem->Begin();
		//Perform additional pre-processing by default if we're to be saving to Maya.
		PrepareSceneForMayaConversion(pProgressSystem);
		if (pProgress)
			pProgressSystem->End();

		Tu::Status tuStatus;
		if (!Nexus::g_pCore->IsMayaInitialised())
		{
			tuStatus = Nexus::g_pCore->InitialiseMaya();
			if (!tuStatus) return tuStatus;
		}
	}
	#endif

	if (pProgress)
		pProgressSystem->Begin();
	
	if (pProgressSystem)
		pProgressSystem->SetTitle("Saving to external scene...");

	status = Nexus::g_pCore->GetAftkScenePtr()->WriteExternalScene(
		target,
		0,
		(Aftk::I_Progress*)pProgress);

	//We didn't cancel the task, and we didn't succeed either.
	if (!status)
	{
		Nexus::g_pCore->Unlock();
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::OpenExternalScene(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();

	Tu::Status status;
	status = Nexus::OpenExternalScene(
		pProgress, 
		Nexus::g_pCore->GetAftkScenePtr(),
		arguments);
	if (!status) return status;

	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}


Tu::Status AftkTasks::ValidateScene(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
		pProgressSystem->SetTitle("Validating scene...");

	U32 errorCount = Nexus::g_pCore->GetAftkScenePtr()->Validate(
		(Aftk::I_Progress*)pProgress);
	if (errorCount != 0)
	{
		Nexus::g_pCore->Unlock();
		return Tu::Status(
			Tu::Status::FAILURE, 
			1,
			NEXUS_CORE_VALIDATE_SCENE_FAIL);
	}

	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

Tu::Status AftkTasks::MergeMeshes(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Merging meshes...");
		pProgressSystem->OnPrintProgress("\n");
	}

	U32 workToDo = 0;
	{
		Aftk::C_SceneNodeIterator it(Nexus::g_pCore->GetAftkScenePtr());
		for ( ; it; it++)
		{
			Aftk::C_SceneNode* pNode = it;
			if (pNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				//Ignore instanced nodes.
				if (pNode->GetNumInstancesOfThisNode() != 0)
					continue;

				//Ignore nodes with blind data.
				if (pNode->GetBlindDataResourcePtr() != 0)
					continue;

				//Ignore nodes that have influence clusters.
				if (pNode->GetNumInfluenceClusters() != 0)
					continue;

				workToDo++;
			}
		}
	}
	workToDo *= 2;
	workToDo--;
	U32 workDone = 0;

	{
		Aftk::C_SceneMeshNode* pTargetMesh = 0;
		Aftk::C_SceneNodeIterator it(Nexus::g_pCore->GetAftkScenePtr());
		Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneMeshNode*> > toDelete;

		for ( ; it; it++)
		{
			Aftk::C_SceneNode* pNode = it;
			if (pNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				//Ignore instanced nodes.
				if (pNode->GetNumInstancesOfThisNode() != 0)
					continue;

				//Ignore nodes with blind data.
				if (pNode->GetBlindDataResourcePtr() != 0)
					continue;

				//Ignore nodes that have influence clusters.
				if (pNode->GetNumInfluenceClusters() != 0)
					continue;


				if (!pTargetMesh)
					pTargetMesh = (Aftk::C_SceneMeshNode*)pNode;
				else
				{
					Aftk::C_SceneMeshNode* pMeshToAdd = (Aftk::C_SceneMeshNode*)pNode;
					Aftk::Matrix44 destInv;
					UtMaths::Invert4x4(&destInv, pTargetMesh->GetWorldMatrix());
					Aftk::Matrix44 matrixToApply = pMeshToAdd->GetWorldMatrix() * destInv;

					pMeshToAdd->ApplyMatrix(matrixToApply);
					pTargetMesh->AddMesh(pMeshToAdd);
					toDelete.PushBack(pMeshToAdd);
				}

				workDone++;
				if (pProgress)
				{
					pProgressSystem->OnProgress(((F32)workDone / (F32)workToDo) * 100.f);
				}
			}
		}
		for (U32 i = 0; i < (U32)toDelete.Size(); i++)
		{
			Nexus::g_pCore->GetAftkScenePtr()->DeleteSceneNodeAndReparentChildren(toDelete[i], true);
			workDone++;
			if (pProgress)
				pProgressSystem->OnProgress(((F32)workDone / (F32)workToDo) * 100.f);
		}
	}

	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

void AftkTasks::PrintSceneBounds()
{
	Aftk::C_Bounds bounds = Nexus::g_pCore->GetAftkScenePtr()->GetEvaluatedBounds(&Nexus::g_pCore->GetBoundsFilter());

	std::stringstream ss;
	Aftk::Position3 min = bounds.GetMin();
	Aftk::Position3 max = bounds.GetMax();
	Aftk::Vector3F32 delta = (Aftk::Vector3F32&)max - (Aftk::Vector3F32&)min;
	ss << "Scene dimensions X: " << delta[0] << "units\n";
	ss << "Scene dimensions Y: " << delta[1] << "units\n";
	ss << "Scene dimensions Z: " << delta[2] << "units\n";

	GlobalTasks::Print(ss.str());
}

void AftkTasks::PrintSceneGraph()
{
	g_pCore->GetAftkScenePtr()->OutputSceneGraph();
}

void AftkTasks::PrintGenericAttribute(
	std::stringstream& ss,
	const Aftk::C_GenericAttribute& attr)
{
	Util::C_String str;
	const char* type = g_aftkAttrTable[attr.GetDataType()];
	switch (attr.GetDataType())
	{
	case Aftk::E_GenericAttributeType_Vector2F32:
		{
			Aftk::Vector2F32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%.2f,%.2f].\n", 
				attr.GetName().c_str(), type, val[0], val[1]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2F64:
		{
			Aftk::Vector2F64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%.2f,%.2f].\n", 
				attr.GetName().c_str(), type, val[0], val[1]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2S8:
		{
			Aftk::Vector2S8 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2S16:
		{
			Aftk::Vector2S16 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2S32:
		{
			Aftk::Vector2S32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2S64:
		{
			Aftk::Vector2S64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s.\n", 
				attr.GetName().c_str(), type);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2U8:
		{
			Aftk::Vector2U8 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2U16:
		{
			Aftk::Vector2U16 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2U32:
		{
			Aftk::Vector2U32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector2U64:
		{
			Aftk::Vector2U64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s.\n", 
				attr.GetName().c_str(), type);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3F32:
		{
			Aftk::Vector3F32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%.2f,%.2f,%.2f].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3F64:
		{
			Aftk::Vector2F64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%.2f,%.2f,%.2f].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3S8:
		{
			Aftk::Vector3S8 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3S16:
		{
			Aftk::Vector3S16 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3S32:
		{
			Aftk::Vector3S32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3S64:
		{
			Aftk::Vector3S64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s.\n", 
				attr.GetName().c_str(), type);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3U8:
		{
			Aftk::Vector3U8 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3U16:
		{
			Aftk::Vector3U16 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3U32:
		{
			Aftk::Vector3U32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector3U64:
		{
			Aftk::Vector3U64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s.\n", 
				attr.GetName().c_str(), type);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4F32:
		{
			Aftk::Vector4F32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%.2f,%.2f,%.2f,%.2f].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2], val[3]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4F64:
		{
			Aftk::Vector4F64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%.2f,%.2f,%.2f,%.2f].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2], val[3]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4S8:
		{
			Aftk::Vector4S8 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2], val[3]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4S16:
		{
			Aftk::Vector4S16 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2], val[3]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4S32:
		{
			Aftk::Vector4S32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2], val[3]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4S64:
		{
			Aftk::Vector4S64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s].\n", 
				attr.GetName().c_str(), type);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4U8:
		{
			Aftk::Vector4U8 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2], val[3]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4U16:
		{
			Aftk::Vector4U16 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2], val[3]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4U32:
		{
			Aftk::Vector4U32 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s, Value [%i,%i,%i,%i].\n", 
				attr.GetName().c_str(), type, val[0], val[1], val[2], val[3]);
		}
		break;

	case Aftk::E_GenericAttributeType_Vector4U64:
		{
			Aftk::Vector4U64 val;
			attr.GetValue(val);
			str.Printf("Name [%s] Type %s.\n", 
				attr.GetName().c_str(), type);
		}
		break;

	default:
		PrintGenericAttributeOrPlug(ss, attr);
		break;
	}

	ss << str.Chars();
};

void PrintAttributeType(
	std::stringstream& ss,
	Aftk::E_GenericAttributeType type)
{
	const char* typeStr = g_aftkAttrTable[type];
	ss << "[" << typeStr << "]";
};

void AftkTasks::PrintPlug(
	std::stringstream& ss,
	const Aftk::I_Plug& plug,
	U32 depth)
{
	for (U32 j = 0; j < depth; j++)	ss << "   ";
	PrintGenericAttributeOrPlug(ss, plug);
	for (U32 i = 0; i < plug.GetNumChildren(); i++)
	{
		PrintPlug(ss, *plug.GetChildPtr(i), depth+1);
	}
}

void AftkTasks::PrintMaterial(
	std::stringstream& ss,
	const Aftk::C_MaterialNode& mtl)
{
	ss << " Material " << mtl.GetName() << "\n";
	std::string originName;
	switch (mtl.GetOrigin())
	{
	case Aftk::E_MaterialOrigin_Editor:
		originName = "The Editor";
		break;
	case Aftk::E_MaterialOrigin_Maya:
		originName = "Maya";
		break;
	case Aftk::E_MaterialOrigin_Maya_CgFX:
		originName = "Maya 6.0 CgFX";
		break;
	case Aftk::E_MaterialOrigin_XSI:
		originName = "XSI";
		break;
	case Aftk::E_MaterialOrigin_SI:
		originName = "SI";
		break;
	case Aftk::E_MaterialOrigin_Maya_CgFX7:
		originName = "Maya 7.0 CgFX";
	case Aftk::E_MaterialOrigin_Maya_Hlsl_2008:
		originName = "Maya 2008 HLSL";
		break;
	default:
		originName = "unknown";
	};
	ss << " Origin " << originName << "\n";
	ss << " Type Id " << mtl.GetMaterialTypeId() << "\n";

	ss << " Material plugs:\n";
	for (U32 i = 0; i < mtl.GetNumPlugs(); i++)
	{
		ss << " ";
		PrintGenericAttributeOrPlug(ss, *mtl.GetPlugPtr(i));
		for (U32 j = 0; j < mtl.GetPlugPtr(i)->GetNumChildren(); j++)
		{
			PrintPlug(ss, *mtl.GetPlugPtr(i)->GetChildPtr(j), 0);
		}
	}

	ss << " Shaders:\n";

	for (U32 i = 0; i < mtl.GetNumShaderNodes(); i++)
	{
		const Aftk::C_ShaderNode* pShaderNode = mtl.GetShaderNodePtr(i);
		ss << " " << pShaderNode->GetName() << "\n";
		for (U32 j = 0; j < pShaderNode->GetNumPlugs(); j++)
		{
			PrintPlug(ss, *pShaderNode->GetPlugPtr(j), 0);
		}
	}
}

void AftkTasks::PrintBlindData(
	std::stringstream& ss,
	const Aftk::C_BlindDataNode& bd)
{
	ss << " Blind data " << bd.GetName() << "\n";
	for (U32 i = 0; i < bd.GetNumAttributes(); i++)
	{
		ss << " ";
		PrintGenericAttribute(ss, bd.GetAttribute(i));
	}
}

void AftkTasks::PrintSelectedFaceInfo()
{
	AftkSceneNodeMap& selectedSceneNodes = g_pCore->GetSelected();
	for (AftkSceneNodeMap::Iterator it = selectedSceneNodes.Begin();
		it != selectedSceneNodes.End();
		++it)
	{
		PrintSelectedFaceInfo(it->GetValue());
	}
}

void AftkTasks::PrintSelectedFaceInfo(const C_SelectedNodeInfo& selectedNodeInfo)
{
	Aftk::C_SceneNode* pNode;

	if (selectedNodeInfo.pSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
	{
		pNode = 
			((Aftk::C_SceneInstanceNode*)selectedNodeInfo.pSceneNode)->GetTopLevelSourceNodePtr();
	}
	else
	{
		pNode = selectedNodeInfo.pSceneNode;
	}

	if (!pNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
		return;

	std::stringstream ss;

	Aftk::C_SceneMeshRemappableNode* pMesh = 
		(Aftk::C_SceneMeshRemappableNode*)pNode;

	const Ut::UtSet<C_ComponentSelection>& selectedComponents = 
		selectedNodeInfo.selectedComponents;
	if (selectedComponents.Size() == 0)
		return;

	ss << "Face info for " << pMesh->GetName() << "\n";

	for (Ut::UtSet<C_ComponentSelection>::ConstIterator compIt = selectedComponents.Begin();
		compIt != selectedComponents.End();
		++compIt)
	{
		const Aftk::C_Polygon& poly = 
			pMesh->GetPolygon(
				compIt->GetKey().lodLevel, 
				compIt->GetKey().componentIndex);

		if (compIt->GetKey().lodLevel == -1 || compIt->GetKey().componentIndex == -1)
			continue;

		if (poly.GetBlindDataMappingIndex() != -1)
		{
			const Aftk::C_BlindDataResource* pRes = 
				pMesh->GetPolygonBlindDataMappingPtr(poly.GetBlindDataMappingIndex());

			PrintBlindData(ss, pRes->GetContent());
		}
		if (poly.GetMaterialMappingIndex() != -1)
		{
			const Aftk::C_MaterialResource* pRes =
				pMesh->GetPolygonMaterialMappingPtr(poly.GetMaterialMappingIndex());

			PrintMaterial(ss, pRes->GetContent());
		}

		ss << " LOD " 
			<< compIt->GetKey().lodLevel << " Index " 
			<< compIt->GetKey().componentIndex << "\n";

		ss << " Base poly index " << poly.GetBasePolygonIndex() << "\n";

		if (poly.GetClusterPtr())
			ss << " Cluster " + poly.GetClusterPtr()->GetName() << "\n\n";

		GlobalTasks::Print(ss.str());
	}
}

void AftkTasks::PrintSelectedNodeInfo(const C_SelectedNodeInfo& selectedNodeInfo)
{
	const Aftk::C_Bounds& bounds = selectedNodeInfo.pSceneNode->GetWorldBounds();
	std::stringstream ss;
	ss << "Node [" << selectedNodeInfo.pSceneNode->GetName() << "]." << " Type [" << selectedNodeInfo.pSceneNode->GetType() << "]";

	for (U32 jLayer = 0; jLayer < selectedNodeInfo.pSceneNode->GetNumLayers(); jLayer++)
	{
		ss << " Layer [" << selectedNodeInfo.pSceneNode->GetLayerPtr(jLayer)->GetName() << "].\n";
	}
	ss << " Unique ID [" << selectedNodeInfo.pSceneNode->GetUniqueId() << "]\n";

	Aftk::C_SceneNode* pSource = 0;
	if (selectedNodeInfo.pSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
	{
		Aftk::C_SceneInstanceNode* pInst = (Aftk::C_SceneInstanceNode*)selectedNodeInfo.pSceneNode;
		pSource = pInst->GetSourceNodePtr();
		if (!pSource)
			ss << "This is a null instance.\n";
		else
			ss << "Is an instance of [" << pInst->GetSourceNodePtr()->GetName() << "]\n";
	}
	else if (selectedNodeInfo.pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
	{
		Aftk::C_SceneMeshRemappedInstanceNode* pInst =
			(Aftk::C_SceneMeshRemappedInstanceNode*)selectedNodeInfo.pSceneNode;
		pSource = pInst->GetSourceNodePtr();
		if (!pSource)
			ss << "This is a null remapped instance.\n";
		else
			ss << "Is a remapped instance of [" << pInst->GetSourceNodePtr()->GetName() << "]\n";
	}


	ss << " World dimensions X: " << bounds.GetMax()[0] - bounds.GetMin()[0] << "units\n";
	ss << " World dimensions Y: " << bounds.GetMax()[1] - bounds.GetMin()[1] << "units\n";
	ss << " World dimensions Z: " << bounds.GetMax()[2] - bounds.GetMin()[2] << "units\n";

	const Aftk::Matrix44& localMatrix = selectedNodeInfo.pSceneNode->GetLocalMatrix();
	ss << " Local Transformation\n";
	ss << "  Matrix AX: " << localMatrix[0][0] << " " << localMatrix[0][1] << " " << localMatrix[0][2] << " " << localMatrix[0][3] << "\n";
	ss << "  Matrix AY: " << localMatrix[1][0] << " " << localMatrix[1][1] << " " << localMatrix[1][2] << " " << localMatrix[1][3] << "\n";
	ss << "  Matrix AZ: " << localMatrix[2][0] << " " << localMatrix[2][1] << " " << localMatrix[2][2] << " " << localMatrix[2][3] << "\n";
	ss << "  Matrix AW: " << localMatrix[3][0] << " " << localMatrix[3][1] << " " << localMatrix[3][2] << " " << localMatrix[3][3] << "\n";


	const Aftk::Matrix44& worldMatrix = selectedNodeInfo.pSceneNode->GetWorldMatrix();
	ss << " World Transformation\n";
	ss << "  Matrix AX: " << worldMatrix[0][0] << " " << worldMatrix[0][1] << " " << worldMatrix[0][2] << " " << worldMatrix[0][3] << "\n";
	ss << "  Matrix AY: " << worldMatrix[1][0] << " " << worldMatrix[1][1] << " " << worldMatrix[1][2] << " " << worldMatrix[1][3] << "\n";
	ss << "  Matrix AZ: " << worldMatrix[2][0] << " " << worldMatrix[2][1] << " " << worldMatrix[2][2] << " " << worldMatrix[2][3] << "\n";
	ss << "  Matrix AW: " << worldMatrix[3][0] << " " << worldMatrix[3][1] << " " << worldMatrix[3][2] << " " << worldMatrix[3][3] << "\n";

	const Aftk::Matrix44& basePose = selectedNodeInfo.pSceneNode->GetBasePose();
	ss << " Base Pose World Transformation\n";
	ss << "  Matrix AX: " << basePose[0][0] << " " << basePose[0][1] << " " << basePose[0][2] << " " << basePose[0][3] << "\n";
	ss << "  Matrix AY: " << basePose[1][0] << " " << basePose[1][1] << " " << basePose[1][2] << " " << basePose[1][3] << "\n";
	ss << "  Matrix AZ: " << basePose[2][0] << " " << basePose[2][1] << " " << basePose[2][2] << " " << basePose[2][3] << "\n";
	ss << "  Matrix AW: " << basePose[3][0] << " " << basePose[3][1] << " " << basePose[3][2] << " " << basePose[3][3] << "\n";

	if (selectedNodeInfo.pSceneNode->HasType(Aftk::E_ObjType_SceneJoint))
	{
		ss << " Segment scale compensation is ";
		if (((Aftk::C_SceneJointNode*)selectedNodeInfo.pSceneNode)->GetSegmentScaleCompensation())
			ss << "on.\n";
		else
			ss << "off.\n";
	}

	{
		if (selectedNodeInfo.pSceneNode->GetBlindDataResourcePtr())
		{
			ss << " Blind data:\n";
			PrintBlindData(ss, selectedNodeInfo.pSceneNode->GetBlindDataResourcePtr()->GetContent());
		}
		else
		{
			ss << " No blind data.\n";
		}

		if (selectedNodeInfo.pSceneNode->GetNumDynamicPlugs() != 0)
		{
			ss << " Dynamic plugs:\n";
			for (U32 i = 0; i < selectedNodeInfo.pSceneNode->GetNumDynamicPlugs(); i++)
			{
				ss << "  ";
				PrintGenericAttribute(ss, selectedNodeInfo.pSceneNode->GetDynamicPlugPtr(i)->GetAttribute());
			}
		}
		else
		{
			ss << " No Dynamic plugs.\n";
		}
	}

	if (selectedNodeInfo.pSceneNode->HasType(Aftk::E_ObjType_SceneLodGroup))
	{
		ss << " LOD Group Info Table:\n";
		Aftk::C_SceneLodGroupNode* pLodGroup = (Aftk::C_SceneLodGroupNode*)selectedNodeInfo.pSceneNode;
		for (U32 i = 0; i < pLodGroup->GetNumLodLevels(); i++)
		{
			Aftk::C_GroupLodLevelInfo& info = pLodGroup->GetLodLevelInfo(i);
			std::string childName;
			if (info.GetSceneNodePtr())
				childName = info.GetSceneNodePtr()->GetName();
			ss << "  LOD " << i << " threshold for child [" << childName << "] " << info.GetThreshold() << "\n";
		}
	}

	if (selectedNodeInfo.pSceneNode->HasType(Aftk::E_ObjType_ISceneMesh))
	{
		ss << " Cluster Table:\n";
		const Aftk::I_SceneMeshNode* pMesh = (Aftk::I_SceneMeshNode*)selectedNodeInfo.pSceneNode;
		for (U32 i = 0; i < pMesh->GetNumPolygonClusters(); i++)
		{
			const Aftk::C_PolygonCluster* pCluster = pMesh->GetPolygonClusterPtr(i);
			if (!pCluster)
				continue;

			if ( pCluster->GetNumLodLevels() )
			{
				ss 	<< "Cluster " << i << " " << pCluster->GetName() 
					<< " num polys (lod 0) " << pCluster->GetNumPolygonIndices(0) << "\n";
			}
			else
				ss 	<< "Cluster " << i << " " << pCluster->GetName() << "\n";
		}
	}

	if (selectedNodeInfo.pSceneNode->HasType(Aftk::E_ObjType_ISceneMesh))
	{
		ss << " Vertex Data Channel Table:\n";
		const Aftk::I_SceneMeshNode* pMesh = (Aftk::I_SceneMeshNode*)selectedNodeInfo.pSceneNode;
		for (U32 i = 0; i < Maths::Min(U32(10), pMesh->GetNumVertexDataChannels()); i++)
		{
			const Aftk::C_VertexDataChannelInfo& channel = pMesh->GetVertexDataChannelInfo(i);

			ss 	<< "Channel " << i << " " << channel.GetName() << " type ";
			PrintAttributeType(ss, channel.GetDataType());
			ss << "\n";
		}
	}

	GlobalTasks::Print(ss.str());
}

void AftkTasks::PrintSelectedNodeInfo()
{
	AftkSceneNodeMap& selectedSceneNodes = g_pCore->GetSelected();
	{
		std::stringstream ss;
		Aftk::C_Bounds bounds;
		for (AftkSceneNodeMap::Iterator it = selectedSceneNodes.Begin();
			it != selectedSceneNodes.End();
			++it)
		{
			bounds = bounds + it->GetKey()->GetEvaluatedWorldBounds();
		}
		Aftk::Position3 min = bounds.GetMin();
		Aftk::Position3 max = bounds.GetMax();
		Aftk::Vector3F32 delta = (Aftk::Vector3F32&)max - (Aftk::Vector3F32&)min;
		ss << "\n";
		ss << "Selection dimensions X: " << delta[0] << "units\n";
		ss << "Selection dimensions Y: " << delta[1] << "units\n";
		ss << "Selection dimensions Z: " << delta[2] << "units\n";
		GlobalTasks::Print(ss.str());
	}
	
	for (AftkSceneNodeMap::Iterator it = selectedSceneNodes.Begin();
		it != selectedSceneNodes.End();
		++it)
	{
		PrintSelectedNodeInfo(it->GetValue());
	}
}

void CountMeshStats(
	U32& polygonCount,
	U32& pvCount,
	U32& posCount,
	U32& normCount,
	U32& uvCount,
	U32& tangentCount,
	U32& binormalCount,
	const Aftk::I_SceneMeshNode* pMesh)
{
	for (U32 iLod = 0; iLod < pMesh->GetNumLodLevels(); iLod++)
		polygonCount += pMesh->GetNumPolygons(iLod);

	pvCount += pMesh->GetNumPolygonVertices();
	posCount += pMesh->GetNumVertexPositions();
	normCount += pMesh->GetNumVertexNormals();
	for (U32 iUV = 0; iUV < pMesh->GetNumTextureCoordinateSets(); iUV++)
	{
		uvCount += pMesh->GetNumTextureCoordinates(iUV);
		tangentCount += pMesh->GetNumVertexTangents(iUV);
		binormalCount += pMesh->GetNumVertexBinormals(iUV);
	}	
}

void AftkTasks::PrintSceneInfo()
{
	U32 polygonCount = 0;
	U32 pvCount = 0;
	U32 posCount = 0;
	U32 normCount = 0;
	U32 uvCount = 0;
	U32 tangentCount = 0;
	U32 binormalCount = 0;

	U32 instPolygonCount = 0;
	U32 instPvCount = 0;
	U32 instPosCount = 0;
	U32 instNormCount = 0;
	U32 instUvCount = 0;
	U32 instTangentCount = 0;
	U32 instBinormalCount = 0;

	U32 numMeshes = 0;
	U32 numMeshInstances = 0;

	Aftk::C_SceneNodeConstIterator it(g_pCore->GetAftkScenePtr());
	for (; it; it++)
	{
		const Aftk::C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			CountMeshStats(
				polygonCount,
				pvCount,
				posCount,
				normCount,
				uvCount,
				tangentCount,
				binormalCount,
				(Aftk::I_SceneMeshNode*)pSceneNode);
			numMeshes++;
		}
		else if (pSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			Aftk::C_SceneInstanceNode* pInst = (Aftk::C_SceneInstanceNode*)pSceneNode;
			if (pInst->GetSourceNodePtr())
			{
				if (pInst->HasType(Aftk::E_ObjType_ISceneMesh))
				{
					CountMeshStats(
						instPolygonCount,
						instPvCount,
						instPosCount,
						instNormCount,
						instUvCount,
						instTangentCount,
						instBinormalCount,
						(Aftk::I_SceneMeshNode*)pInst->GetSourceNodePtr());
					numMeshInstances++;
				}
			}
		}
		else if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		{
			const Aftk::C_SceneMeshRemappedInstanceNode* pInst = (Aftk::C_SceneMeshRemappedInstanceNode*)pSceneNode;
			if (pInst->GetSourceNodePtr())
			{
				if (pInst->HasType(Aftk::E_ObjType_ISceneMesh))
				{
					CountMeshStats(
						instPolygonCount,
						instPvCount,
						instPosCount,
						instNormCount,
						instUvCount,
						instTangentCount,
						instBinormalCount,
						(Aftk::I_SceneMeshNode*)pInst->GetSourceNodePtr());
					numMeshInstances++;
				}
			}
		}
	}

	std::stringstream ss;
	ss << "Num meshes: " << numMeshes << "\n";
	ss << "Num mesh instances: " << numMeshInstances << "\n\n";

	ss << "Mesh polygon count: " << polygonCount << "\n";
	ss << "Mesh polygon vertex count: " << pvCount << "\n";
	ss << "Mesh position count: " << posCount << "\n";
	ss << "Mesh normal count: " << normCount << "\n";
	ss << "Mesh uv count: " << uvCount << "\n";
	ss << "Mesh tangent count: " << tangentCount << "\n";
	ss << "Mesh binormal count: " << binormalCount << "\n\n";

	ss << "Instanced mesh polygon count: " << instPolygonCount << "\n";
	ss << "Instanced mesh polygon vertex count: " << instPvCount << "\n";
	ss << "Instanced mesh position count: " << instPosCount << "\n";
	ss << "Instanced mesh normal count: " << instNormCount << "\n";
	ss << "Instanced mesh uv count: " << instUvCount << "\n";
	ss << "Instanced mesh tangent count: " << instTangentCount << "\n";
	ss << "Instanced mesh binormal count: " << instBinormalCount << "\n\n";

	ss << "Total expanded mesh polygon count: " << polygonCount+instPolygonCount << "\n";
	ss << "Total expanded mesh polygon vertex count: " << pvCount+instPvCount << "\n";
	ss << "Total expanded mesh position count: " << posCount+instPosCount << "\n";
	ss << "Total expanded mesh normal count: " << normCount+instNormCount << "\n";
	ss << "Total expanded mesh uv count: " << uvCount+instUvCount << "\n";
	ss << "Total expanded mesh tangent count: " << tangentCount+instTangentCount << "\n";
	ss << "Total expanded mesh binormal count: " << binormalCount+instBinormalCount << "\n";

	GlobalTasks::Print(ss.str());
}

Tu::Status AftkTasks::TriangulateMesh(void* pProgress, const StringArray& arguments)
{
	Aftk::C_SceneNodeVisit::Traverse(
		g_pCore->GetAftkScenePtr(),
		Aftk::C_MeshTriangulate::C_TriangulateVisitor());

	return Tu::Status::SUCCESS;
}

int AftkTasks::OpenSceneLua(lua_State* pLuaState)
{
	Tu::Status status;
	int size = lua_gettop(pLuaState);

	StringArray temp;
	for (int i = 0; i < size; i++)
	{
		const char* pStr = lua_tostring(pLuaState, -1);
		temp.PushBack(pStr);
		lua_pop(pLuaState, 1);
	}
	//Reverse arg order
	StringArray arguments;
	for (int i = 0; i < temp.Size(); i++)
	{
		arguments.PushBack(temp[temp.Size()-i-1]);
	}

	if (g_pProgressCallback)
		status = g_pProgressCallback->ExecuteTask(OpenScene, arguments);

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::ConvertSceneLua(lua_State* pLuaState)
{
	Tu::Status status;
	int size = lua_gettop(pLuaState);

	StringArray temp;
	for (int i = 0; i < size; i++)
	{
		const char* pStr = lua_tostring(pLuaState, -1);
		temp.PushBack(pStr);
		lua_pop(pLuaState, 1);
	}
	//Reverse arg order
	StringArray arguments;
	for (int i = 0; i < temp.Size(); i++)
	{
		arguments.PushBack(temp[temp.Size()-i-1]);
	}

	if (g_pProgressCallback)
		status = g_pProgressCallback->ExecuteTask(ConvertScene, arguments);

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}


int AftkTasks::OpenExternalSceneLua(lua_State* pLuaState)
{
	Tu::Status status;
	StringArray arguments;
	LuaUtility::GetArguments(arguments, pLuaState);

	if (g_pProgressCallback)
		status = g_pProgressCallback->ExecuteTask(OpenExternalScene, arguments);

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::SaveExternalSceneLua(lua_State* pLuaState)
{
	Tu::Status status;
	StringArray arguments;
	LuaUtility::GetArguments(arguments, pLuaState);

	if (g_pProgressCallback)
		status = g_pProgressCallback->ExecuteTask(SaveExternalScene, arguments);

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::ImportSceneLua(lua_State* pLuaState)
{
	Tu::Status status;
	int size = lua_gettop(pLuaState);

	StringArray temp;
	for (int i = 0; i < size; i++)
	{
		const char* pStr = lua_tostring(pLuaState, -1);
		temp.PushBack(pStr);
		lua_pop(pLuaState, 1);
	}
	//Reverse arg order
	StringArray arguments;
	for (int i = 0; i < temp.Size(); i++)
	{
		arguments.PushBack(temp[temp.Size()-i-1]);
	}

	if (g_pProgressCallback)
		status = g_pProgressCallback->ExecuteTask(ImportScene, arguments);

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::ValidateSceneLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	LuaUtility::GetArguments(arguments, pLuaState);
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(ValidateScene, arguments);
	}
	
	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::RepairSceneLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	LuaUtility::GetArguments(arguments, pLuaState);
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(RepairScene, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::WeldMeshPolygonVerticesLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	LuaUtility::GetArguments(arguments, pLuaState);
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(WeldMeshPolygonVertices, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::WeldMeshComponentsLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	LuaUtility::GetArguments(arguments, pLuaState);
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(WeldMeshComponents, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::WeldMeshLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	LuaUtility::GetArguments(arguments, pLuaState);
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(WeldMesh, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::TriangulateMeshLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	LuaUtility::GetArguments(arguments, pLuaState);
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(TriangulateMesh, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::SaveSceneLua(lua_State* pLuaState)
{
	Tu::Status status;
	const char* pStr = lua_tostring(pLuaState, 1);
	lua_pop(pLuaState, 1);

	StringArray arguments;
	arguments.PushBack(pStr);
	if (g_pProgressCallback)
		status = g_pProgressCallback->ExecuteTask(SaveScene, arguments);

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::PrintSceneBoundsLua(lua_State* pLuaState)
{
	PrintSceneBounds();
	return 0;
}

int AftkTasks::PrintSceneGraphLua(lua_State* pLuaState)
{
	PrintSceneGraph();
	return 0;
}

int AftkTasks::PrintSelectedNodeInfoLua(lua_State* pLuaState)
{
	PrintSelectedNodeInfo();
	return 0;
}

int AftkTasks::PrintSelectedFaceInfoLua(lua_State* pLuaState)
{
	PrintSelectedFaceInfo();
	return 0;
}

int AftkTasks::PrintSceneInfoLua(lua_State* pLuaState)
{
	PrintSceneInfo();
	return 0;
}

int AftkTasks::NewSceneLua(lua_State* pLuaState)
{
	NewScene();
	return 0;
}

int AftkTasks::CalculateTangentsAndBinormalsLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(CalculateTangentsAndBinormals, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkTasks::MergeMeshesLua(lua_State* pLuaState)
{
	Tu::Status status;
	StringArray arguments;
	U32 argIndex = 1;
	while(true)
	{
		const char* pStr = lua_tostring(pLuaState, argIndex);
		if (pStr == 0)
			break;
		arguments.PushBack(pStr);
		argIndex++;
	}
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(MergeMeshes, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

void AftkTasks::Register(lua_State* pLuaState)
{
	//Register Lua functions
	lua_register( pLuaState, "OpenScene",				&AftkTasks::OpenSceneLua);
	lua_register( pLuaState, "ImportScene",				&AftkTasks::ImportSceneLua);
	lua_register( pLuaState, "SaveScene",				&AftkTasks::SaveSceneLua);
	lua_register( pLuaState, "ConvertScene",			&AftkTasks::ConvertSceneLua);
	lua_register( pLuaState, "RepairScene",				&AftkTasks::RepairSceneLua);
	lua_register( pLuaState, "OpenExternalScene",		&AftkTasks::OpenExternalSceneLua);
	lua_register( pLuaState, "SaveExternalScene",		&AftkTasks::SaveExternalSceneLua);
	lua_register( pLuaState, "ValidateScene",			&AftkTasks::ValidateSceneLua);
	lua_register( pLuaState, "TriangulateMesh",			&AftkTasks::TriangulateMeshLua);
	lua_register( pLuaState, "WeldMeshPolygonVertices", &AftkTasks::WeldMeshPolygonVerticesLua);
	lua_register( pLuaState, "WeldMeshComponents",		&AftkTasks::WeldMeshComponentsLua);
	lua_register( pLuaState, "WeldMesh",				&AftkTasks::WeldMeshLua);
	lua_register( pLuaState, "PrintSceneBounds",		&AftkTasks::PrintSceneBoundsLua);
	lua_register( pLuaState, "PrintSceneGraph",			&AftkTasks::PrintSceneGraphLua);
	lua_register( pLuaState, "PrintSelectedNodeInfo",	&AftkTasks::PrintSelectedNodeInfoLua);
	lua_register( pLuaState, "PrintSelectedFaceInfo",	&AftkTasks::PrintSelectedFaceInfoLua);
	lua_register( pLuaState, "PrintSceneInfo",			&AftkTasks::PrintSceneInfoLua);
	lua_register( pLuaState, "CalculateTangentsAndBinormals",		&AftkTasks::CalculateTangentsAndBinormalsLua);
	lua_register( pLuaState, "MergeMeshes",				&AftkTasks::MergeMeshesLua);
}

}