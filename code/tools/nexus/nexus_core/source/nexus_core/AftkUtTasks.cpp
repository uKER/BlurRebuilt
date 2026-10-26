#include "stdafx.h"
#include <nexus_core/core.h>

#include <aftkut/aftkut.h>
#include <nexus_core/LuaUtility.h>

namespace Nexus
{

Tu::Status AftkUtTasks::RemoveDuplicateMeshes(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Removing Duplicate Meshes...");
	}

	AftkUt::C_ProcessDuplicateNodes::RemoveDuplicateNodes(
		Nexus::g_pCore->GetAftkScenePtr(),
		AftkUt::C_ProcessDuplicateNodes::AFTKUT_EQUALITY_MESHES_ONLY,
		Nexus::g_pCore->GetPrintMessageCallback(),
		Nexus::g_pCore->GetPrintWarningCallback(),
		Nexus::g_pCore->GetPrintErrorCallback(),
		(Aftk::I_Progress*)pProgress);
	
	Nexus::g_pCore->OnSceneChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

int AftkUtTasks::RemoveDuplicateMeshesLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(RemoveDuplicateMeshes, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

Tu::Status AftkUtTasks::CreateInstances(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Creating instances...");
	}

	AftkUt::C_ProcessDuplicateNodes::CreateInstances(
		Nexus::g_pCore->GetAftkScenePtr(),
		Nexus::g_pCore->GetPrintMessageCallback(),
		Nexus::g_pCore->GetPrintWarningCallback(),
		Nexus::g_pCore->GetPrintErrorCallback(),
		(Aftk::I_Progress*)pProgress);
	
	Nexus::g_pCore->OnSceneChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

int AftkUtTasks::CreateInstancesLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(CreateInstances, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}


Tu::Status AftkUtTasks::CalculateNormals(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Calculating normals...");
	}

	Aftk::C_Status status = Aftk::C_SceneNodeVisit::Traverse(
		Nexus::g_pCore->GetAftkScenePtr(), 
		Aftk::C_MeshCalculateNormals::C_CalculateNormalsVisitor(),
		(Aftk::I_Progress*)pProgress);

	if ((!status) && (!pProgressSystem->Cancelled()))
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

Tu::Status AftkUtTasks::CalculateSmoothTangentsAndBinormals(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	if (arguments.Size() > 0)
	{
		if (arguments.Size() != 4)
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

	F32 mag = 0.2f;
	F32 angle = 80.f;
	F32 tolerance = 1e-6f;
	bool normalise = true;
	if (arguments.Size() == 4)
	{
		{
			std::stringstream ss; ss << arguments[0];
			ss >> mag;
		}
		{
			std::stringstream ss; ss << arguments[1];
			ss >> angle;
		}
		{
			std::stringstream ss; ss << arguments[2];
			ss >> tolerance;
		}
		if (arguments[3] == "true")
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

	if ((!status) && (!pProgressSystem->Cancelled()))
	{
		Nexus::g_pCore->Unlock();
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	//Smooth our tangent space...
	U32 workToDo = Nexus::g_pCore->GetAftkScenePtr()->GetNumDgNodes(Aftk::E_ObjType_SceneMesh);
	U32 workDone = 0;
	if (pProgressSystem)
	{
		pProgressSystem->OnPrintProgress("Smoothing tangents...\n");
		pProgressSystem->OnProgress(0);
	}
	{
		Aftk::C_SceneNodeIterator it(Nexus::g_pCore->GetAftkScenePtr());
		while (it)
		{
			Aftk::C_SceneNode* pAftkSceneNode = it;
			if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				status = AftkUt::C_SmoothTangents::Smooth(
					(Aftk::C_SceneMeshNode*)pAftkSceneNode,
					mag, 
					angle,
					normalise);
				if (!status) 
				{
					Nexus::g_pCore->Unlock();
					return Tu::Status(
						Tu::Status::FAILURE, 
						status.GetSeverity(),
						status.GetErrorString());
				}
				workDone++;
				if (pProgressSystem)
				{
					pProgressSystem->OnProgress((F32)workDone / (F32)workToDo);
				}				
			}
			it++;
		}
	}

	if ((!status) && (!pProgressSystem->Cancelled()))
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

Tu::Status AftkUtTasks::CalculateSmoothNormals(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	if (arguments.Size() > 0)
	{
		if (arguments.Size() != 2)
		{
			Nexus::g_pCore->Unlock();
			return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);
		}
	}
	Progress* pProgressSystem = (Progress*)pProgress;
	if (pProgress)
	{
		pProgressSystem->SetTitle("Calculating normals...");
	}

	F32 tolerance;
	F32 angle;
	if (arguments.Size() == 2)
	{
		{
			std::stringstream ss; 
			ss << arguments[0];
			ss >> tolerance;
		}
		{
			std::stringstream ss; 
			ss << arguments[1];
			ss >> angle;
		}
	}

	Aftk::C_Status status = Aftk::C_SceneNodeVisit::Traverse(
		Nexus::g_pCore->GetAftkScenePtr(), 
		Aftk::C_MeshCalculateNormals::C_CalculateNormalsVisitor(),
		(Aftk::I_Progress*)pProgress);

	if ((!status) && (!pProgressSystem->Cancelled()))
	{
		Nexus::g_pCore->Unlock();
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	//Smooth our tangent space...
	U32 workToDo = Nexus::g_pCore->GetAftkScenePtr()->GetNumDgNodes(Aftk::E_ObjType_SceneMesh);
	U32 workDone = 0;
	if (pProgressSystem)
	{
		pProgressSystem->OnPrintProgress("Smoothing normals...\n");
		pProgressSystem->OnProgress(0);
	}
	{
		Aftk::C_SceneNodeIterator it(Nexus::g_pCore->GetAftkScenePtr());
		while (it)
		{
			Aftk::C_SceneNode* pAftkSceneNode = it;
			if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				status = AftkUt::C_SmoothNormals::Smooth(
					(Aftk::C_SceneMeshNode*)pAftkSceneNode,
					angle);
				if (!status) 
				{
					Nexus::g_pCore->Unlock();
					return Tu::Status(
						Tu::Status::FAILURE, 
						status.GetSeverity(),
						status.GetErrorString());
				}
				workDone++;
				if (pProgressSystem)
				{
					pProgressSystem->OnProgress((F32)workDone / (F32)workToDo);
				}				
			}
			it++;
		}
	}

	if ((!status) && (!pProgressSystem->Cancelled()))
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

Tu::Status AftkUtTasks::SmoothNormals(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Aftk::C_Status status;
	if (arguments.Size() > 0)
	{
		if (arguments.Size() != 2)
		{
			Nexus::g_pCore->Unlock();
			return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);
		}
	}
	Progress* pProgressSystem = (Progress*)pProgress;

	F32 tolerance;
	F32 angle;
	if (arguments.Size() == 2)
	{
		{
			std::stringstream ss; 
			ss << arguments[0];
			ss >> tolerance;
		}
		{
			std::stringstream ss; 
			ss << arguments[1];
			ss >> angle;
		}
	}

	//Smooth our tangent space...
	U32 workToDo = Nexus::g_pCore->GetAftkScenePtr()->GetNumDgNodes(Aftk::E_ObjType_SceneMesh);
	U32 workDone = 0;
	if (pProgressSystem)
	{
		pProgressSystem->OnPrintProgress("Smoothing normals...\n");
		pProgressSystem->OnProgress(0);
	}
	{
		Aftk::C_SceneNodeIterator it(Nexus::g_pCore->GetAftkScenePtr());
		while (it)
		{
			Aftk::C_SceneNode* pAftkSceneNode = it;
			if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				status = AftkUt::C_SmoothNormals::Smooth(
					(Aftk::C_SceneMeshNode*)pAftkSceneNode,
					angle);
				if (!status) 
				{
					Nexus::g_pCore->Unlock();
					return Tu::Status(
						Tu::Status::FAILURE, 
						status.GetSeverity(),
						status.GetErrorString());
				}
				workDone++;
				if (pProgressSystem)
				{
					pProgressSystem->OnProgress((F32)workDone / (F32)workToDo);
				}				
			}
			it++;
		}
	}

	if ((!status) && (!pProgressSystem->Cancelled()))
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

Tu::Status AftkUtTasks::SmoothTangentsAndBinormals(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Aftk::C_Status status;
	if (arguments.Size() > 0)
	{
		if (arguments.Size() != 4)
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

	F32 mag = 0.2f;
	F32 angle = 90.f;
	F32 tolerance = 1e-6f;
	bool normalise = true;
	if (arguments.Size() == 4)
	{
		{
			std::stringstream ss; ss << arguments[0];
			ss >> mag;
		}
		{
			std::stringstream ss; ss << arguments[1];
			ss >> angle;
		}
		{
			std::stringstream ss; ss << arguments[2];
			ss >> tolerance;
		}
		if (arguments[3] == "true")
			normalise = true;
		else
			normalise = false;
	}

	//Smooth our tangent space...
	U32 workToDo = Nexus::g_pCore->GetAftkScenePtr()->GetNumDgNodes(Aftk::E_ObjType_SceneMesh);
	U32 workDone = 0;
	if (pProgressSystem)
	{
		pProgressSystem->OnPrintProgress("Smoothing tangents...\n");
		pProgressSystem->OnProgress(0);
	}
	{
		Aftk::C_SceneNodeIterator it(Nexus::g_pCore->GetAftkScenePtr());
		while (it)
		{
			Aftk::C_SceneNode* pAftkSceneNode = it;
			if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				status = AftkUt::C_SmoothTangents::Smooth(
					(Aftk::C_SceneMeshNode*)pAftkSceneNode,
					mag, 
					angle,
					normalise);
				if (!status) 
				{
					Nexus::g_pCore->Unlock();
					return Tu::Status(
						Tu::Status::FAILURE, 
						status.GetSeverity(),
						status.GetErrorString());
				}
				workDone++;
				if (pProgressSystem)
				{
					pProgressSystem->OnProgress((F32)workDone / (F32)workToDo);
				}				
			}
			it++;
		}
	}

	if ((!status) && (!pProgressSystem->Cancelled()))
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

Tu::Status AftkUtTasks::ExpandUniqueRemappedInstances(void* pProgress, const StringArray& arguments)
{
	Nexus::g_pCore->Lock();
	Aftk::C_Status status;
	if (arguments.Size() > 0)
		if (arguments.Size() != 2)
		{
			Nexus::g_pCore->Unlock();
			return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);
		}

	Progress* pProgressSystem = (Progress*)pProgress;
	status = AftkUt::C_ExpandUniqueRemappedInstances::Expand(Nexus::g_pCore->GetAftkScenePtr());
	if ((!status) && (!pProgressSystem->Cancelled()))
	{
		Nexus::g_pCore->Unlock();
		return Tu::Status(
			Tu::Status::FAILURE, 
			status.GetSeverity(),
			status.GetErrorString());
	}

	Nexus::g_pCore->OnSceneChanged();
	Nexus::g_pCore->Unlock();
	return Tu::Status::SUCCESS;
}

int AftkUtTasks::ExpandUniqueRemappedInstancesLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray args;
	
	LuaUtility::GetArguments(args, pLuaState);
	if (g_pProgressCallback)
		status = g_pProgressCallback->ExecuteTask(ExpandUniqueRemappedInstances, args);

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkUtTasks::CalculateNormalsLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(CalculateNormals, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkUtTasks::CalculateSmoothTangentsAndBinormalsLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(CalculateSmoothTangentsAndBinormals, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkUtTasks::CalculateSmoothNormalsLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(CalculateSmoothNormals, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkUtTasks::SmoothTangentsAndBinormalsLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(SmoothTangentsAndBinormals, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

int AftkUtTasks::SmoothNormalsLua(lua_State* pLuaState)
{
	Tu::Status status;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		status = g_pProgressCallback->ExecuteTask(SmoothNormals, arguments);
	}

	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}


void AftkUtTasks::Register(lua_State* pLuaState)
{
	lua_register( pLuaState, "RemoveDuplicateMeshes",	&AftkUtTasks::RemoveDuplicateMeshesLua);
	lua_register( pLuaState, "CreateInstances",			&AftkUtTasks::CreateInstancesLua);
	lua_register( pLuaState, "CalculateNormals",		&AftkUtTasks::CalculateNormalsLua);
	lua_register( pLuaState, "SmoothNormals",			&AftkUtTasks::SmoothNormalsLua);
	lua_register( pLuaState, "CalculateSmoothNormals",	&AftkUtTasks::CalculateSmoothNormalsLua);
	lua_register( pLuaState, "CalculateSmoothTangentsAndBinormals",	&AftkUtTasks::CalculateSmoothTangentsAndBinormalsLua);
	lua_register( pLuaState, "ExpandUniqueRemappedInstances", &AftkUtTasks::ExpandUniqueRemappedInstancesLua);
}
}