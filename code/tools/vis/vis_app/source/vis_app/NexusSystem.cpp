#include <vis_app/stdafx.h>

//Nexus includes
#include <vis_app/NexusSystem.h>
#include <vis_app/NexusRender.h>

#include "../../resource.h"
#include <vis_app/NexusProgressDialog.h>

#include "..\..\include\vis_app\mainfrm.h"

using namespace Nexus;
using namespace VecMat;
NexusSystem::NexusSystem()
{
	m_isCreated = false;
	m_sceneLoaded = false;
	//m_pSpatialHierachy = 0;
	m_pLogCallbackFunc = 0;
	g_pNexusSystem = this;

	Clear();
}

NexusSystem::~NexusSystem()
{
}

Tu::Status NexusSystem::Create()
{
	Tu::Status status;
	if (m_isCreated) 
	{
		return Tu::Status::SUCCESS;
	}

	m_isCreated = true;

	return Tu::Status::SUCCESS;
}

void NexusSystem::Clear()
{
	m_aftk.Clear();
	m_sceneLoaded = false;

	// set arbitrary bounds
	m_sceneBounds.Clear();
	m_sceneBounds.AddVertex(HVec3(100,100,100));
	m_sceneBounds.AddVertex(HVec3(-100,-100,-100));
}

void NexusSystem::Destroy()
{
	Clear();
	m_isCreated = false;
}

bool NexusSystem::IsCreated() const
{
	return m_isCreated;
}

const Aftk::C_Scene* NexusSystem::GetScenePtr() const
{
	return &m_aftk;
}

Aftk::C_Scene* NexusSystem::GetScenePtr()
{
	return &m_aftk;
}

void NexusSystem::CleanUpAfterFailure()
{
}

Aftk::C_Scene* NexusSystem::AftkPtr()
{
	return &m_aftk;
}

const Aftk::C_Scene* NexusSystem::AftkPtr() const
{
	return &m_aftk;
}

void NexusSystem::NewScene()
{
	//Create renderer if needed.
	if (!Nexus::g_pNexusRender->IsCreated())
	{
		Tu::Status status = Nexus::g_pNexusRender->Create(
			(CView*)g_pView, 
			Nexus::g_pNexusSystem);
		if (!status)
		{
			g_pMainFrame->OnCriticalError(status);
		}
		Nexus::g_pNexusRender->SetTime(0);
	}
	else
	{
		Nexus::g_pNexusRender->Clear();
	}

	m_sceneLoaded = false;
	m_aftk.Clear();

	m_sceneBounds.Clear();
	m_sceneBounds.AddVertex(HVec3(100,100,100));
	m_sceneBounds.AddVertex(HVec3(-100,-100,-100));

	m_pSelectedSceneNode = 0;
}

Tu::Status OutputSceneBoundsQuickTask(void* pArgument)
{
	const Tu::Bounds* pBounds = (Tu::Bounds*)pArgument;
	std::stringstream ss;
	HVec3 min = pBounds->GetMin();
	HVec3 max = pBounds->GetMax();
	HVec3 delta = max - min;
	ss << "Scene dimensions X: " << delta.X() << "m\n";
	ss << "Scene dimensions Y: " << delta.Y() << "m\n";
	ss << "Scene dimensions Z: " << delta.Z() << "m\n";

	g_pMainFrame->OutputLogText(ss.str());

	return Tu::Status::SUCCESS;
}

void NexusSystem::OutputSceneBounds()
{
	OutputSceneBoundsQuickTask(&m_sceneBounds);
}

Tu::Status OpenSceneTask(void* pThis, void* pArgument)
{
	Tu::Status tuStatus = Tu::Status::SUCCESS;

	NexusProgressDialog* pDialog = (NexusProgressDialog*)pThis;
	pDialog->SetTitle("Loading scene...");

	const std::string* pFileName = (const std::string*)pArgument;

	bool nonFatalErrorsEncountered;
	Aftk::C_Status status = Nexus::g_pNexusSystem->AftkPtr()->Read(
		*pFileName,
		AFTK_FILE_TYPE_BEST_GUESS,
		&nonFatalErrorsEncountered,
		AftkTranslationParameters(),
 		(AftkProgressNotification*)pDialog);

	if (!status)
	{
		if (!pDialog->Cancelled())
		{
			MessageBox(
				pDialog->m_hWnd, 
				status.GetErrorString().c_str(),
				"Failed to load scene.",
				MB_OK);
		}
		return Tu::Status(Tu::Status::FAILURE, status.GetErrorString());
	}

	pDialog->SetTitle("Validating scene...");
	if (Nexus::g_pNexusSystem->AftkPtr()->Validate() != 0)
	{
		tuStatus = Tu::Status(Tu::Status::FAILURE, "This scene contains errors which could not be fixed automatically.");
		MessageBox(
			pDialog->m_hWnd, 
			tuStatus.ErrorString().c_str(),
			"Failed to load scene.",
			MB_OK);
		pDialog->SetTaskResult(Tu::Status::FAILURE);
		return tuStatus;
	}

	pDialog->SetTaskResult(tuStatus);
	return Tu::Status::SUCCESS;
}

Tu::Status RepairSceneTask(void* pThis, void* pArgument)
{
	return Tu::Status::SUCCESS;
}

Tu::Status WeldMeshNodeComponentsTask(void* pThis, void* pArgument)
{
	NexusProgressDialog* pDialog = (NexusProgressDialog*)pThis;

	pDialog->SetTitle("Welding mesh components...");
	{
		std::stringstream ss;
		ss << "\nOriginal mesh component count:\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshVertexPositions() << " vertex positions.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshVertexNormals() << " vertex normals.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshVertexTangents() << " vertex tangents.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshVertexBinormals() << " vertex binormals.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshTextureCoordinates() << " texture coordinates.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshPolygonVertices() << " polygon vertices.\n";
		g_pMainFrame->OutputLogText(ss.str());
	}

	{
		std::stringstream ss;
		U32 numWelds = Nexus::g_pNexusSystem->AftkPtr()->WeldMeshComponents(
			AFTK_EPSILON,
			(AftkProgressNotification*)pDialog);
		ss << numWelds << " mesh components welded.\n";
		g_pMainFrame->OutputLogText(ss.str());
	}

	Tu::Status tuStatus = Tu::Status::SUCCESS;
	pDialog->SetTaskResult(tuStatus);

	{
		std::stringstream ss;
		ss << "Welding mesh component results:\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshVertexPositions() << " vertex positions.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshVertexNormals() << " vertex normals.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshVertexTangents() << " vertex tangents.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshVertexBinormals() << " vertex binormals.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshTextureCoordinates() << " texture coordinates.\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshPolygonVertices() << " polygon vertices.\n";
		ss << "\n";
		g_pMainFrame->OutputLogText(ss.str());
	}
	return Tu::Status::SUCCESS;
}

#pragma warning(disable:4800)

Tu::Status WeldMeshNodePolygonVerticesTask(void* pThis, void* pArgument)
{
	NexusProgressDialog* pDialog = (NexusProgressDialog*)pThis;

	pDialog->SetTitle("Welding mesh polygon vertices...");
	{
		std::stringstream ss;
		ss << "Original polygon vertex count:\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshPolygonVertices() << " polygon vertices.\n";
		g_pMainFrame->OutputLogText(ss.str());
	}

	U32 numWelds = Nexus::g_pNexusSystem->AftkPtr()->WeldMeshPolygonVertices(
		AFTK_EPSILON,
		(AftkProgressNotification*)pDialog);
	{
		std::stringstream ss;
		ss << numWelds << " polygon vertices welded.\n";
		g_pMainFrame->OutputLogText(ss.str());
	}

	Tu::Status tuStatus = Tu::Status::SUCCESS;
	pDialog->SetTaskResult(tuStatus);

	{
		std::stringstream ss;
		ss << "Welding polygon vertex results:\n";
		ss << Nexus::g_pNexusSystem->AftkPtr()->GetNumMeshPolygonVertices() << " polygon vertices.\n";
		g_pMainFrame->OutputLogText(ss.str());
	}

	return Tu::Status::SUCCESS;
}


Tu::Status SaveSceneTask(void* pThis, void* pArgument)
{
	NexusProgressDialog* pDialog = (NexusProgressDialog*)pThis;
	pDialog->SetTitle("Saving scene...");

	const std::string* pFileName = (const std::string*)pArgument;

	bool nonFatalErrorsEncountered;
	Aftk::C_Status status = Nexus::g_pNexusSystem->AftkPtr()->Write(
		*pFileName,
		AFTK_FILE_TYPE_A3D,
		0,
		&nonFatalErrorsEncountered,
		AftkTranslationParameters(),
		(AftkProgressNotification*)pDialog);

	if (!status)
	{
		if (!pDialog->Cancelled())
		{
			MessageBox(
				pDialog->m_hWnd, 
				status.GetErrorString().c_str(),
				"Failed to save scene.",
				MB_OK);
		}
		return Tu::Status(Tu::Status::FAILURE, status.GetErrorString());
	}

	return Tu::Status::SUCCESS;
}

Tu::Status NexusSystem::OpenScene(const std::string& fileName)
{
	Tu::Status status;

	//Create renderer if needed.
	if (!Nexus::g_pNexusRender->IsCreated())
	{
		Tu::Status status = Nexus::g_pNexusRender->Create(
			(CView*)g_pView, 
			Nexus::g_pNexusSystem);
		if (!status)
		{
			g_pMainFrame->OnCriticalError(status);
		}
		Nexus::g_pNexusRender->SetTime(0);
	}
	else
	{
		Nexus::g_pNexusRender->Clear();
	}
	{
		NexusProgressDialog dlg;
		dlg.SetTaskCallbackProc(OpenSceneTask);
		dlg.SetTaskArgument((void*)&fileName);
		dlg.DoModal();
		dlg.WaitForThreadToFinish();

		if (!dlg.TaskResult())
		{
			m_sceneLoaded = false;
			m_aftk.Clear();
			return Tu::Status::SUCCESS;
		}

//		m_aftk.WeldMeshComponents(AFTK_EPSILON);
//		m_aftk.WeldMeshPolygonVertices(AFTK_EPSILON);
	}

//	m_aftk.CalculateTangentsAndBinormals();
/*
	{
		NexusProgressDialog dlg;
		dlg.SetTaskCallbackProc(RepairSceneTask);
		dlg.SetTaskArgument((void*)&fileName);
		dlg.DoModal();
		dlg.WaitForThreadToFinish();

		if (!dlg.TaskResult())
		{
			m_sceneLoaded = false;
			m_aftk.Clear();
			return Tu::Status::FAILURE;
		}
	}
	
	
	{
		NexusProgressDialog dlg;
		dlg.SetTaskCallbackProc(WeldMeshNodeComponentsTask);
		dlg.SetTaskArgument((void*)&fileName);
		dlg.DoModal();
		dlg.WaitForThreadToFinish();

		if (!dlg.TaskResult())
		{
			m_sceneLoaded = false;
			m_aftk.Clear();
			return Tu::Status::FAILURE;
		}
	}
	{
		NexusProgressDialog dlg;
		dlg.SetTaskCallbackProc(WeldMeshNodePolygonVerticesTask);
		dlg.SetTaskArgument((void*)&fileName);
		dlg.DoModal();
		dlg.WaitForThreadToFinish();

		if (!dlg.TaskResult())
		{
			m_sceneLoaded = false;
			m_aftk.Clear();
			return Tu::Status::FAILURE;
		}
	}
*/
	ComputeSceneBounds();

	OutputSceneBounds();

	status = Nexus::g_pNexusRender->OnUpdateScene();
	if (!status)
	{
		m_aftk.Clear();
		return status;
	}

	m_sceneLoaded = true;
	
/*
	if (m_pSpatialHierachy)
	{
		delete m_pSpatialHierachy;
	}
*/


//	m_pSpatialHierachy = new SD::KdTree;
//	m_pSpatialHierachy->Build(&m_aftk, 0);
	m_pSelectedSceneNode = 0;

	return Tu::Status::SUCCESS;
}

Tu::Status NexusSystem::SaveScene(const std::string& fileName)
{
	NexusProgressDialog dlg;
	dlg.SetTaskCallbackProc(SaveSceneTask);
	dlg.SetTaskArgument((void*)&fileName);
	dlg.DoModal();
	dlg.WaitForThreadToFinish();

	return dlg.TaskResult();
}

bool NexusSystem::IsSceneLoaded() const
{
	return m_sceneLoaded;
}

const Tu::Bounds& NexusSystem::GetSceneBounds() const
{
	return m_sceneBounds;
}

void NexusSystem::ComputeSceneBounds()
{
	m_sceneBounds.Clear();
	AftkSceneNodeIterator it(&m_aftk);
    
	for ( ; it ; it++)
	{
		AftkSceneNode* pAftkSceneNode = it;

		if (pAftkSceneNode->GetType() == AFTK_NODE_TYPE_SCENE_INSTANCE)
		{
			AftkSceneInstanceNode* pAftkInstance = (AftkSceneInstanceNode*)pAftkSceneNode;
			if (
				(pAftkInstance->GetSourceNodePtr()->GetType() != E_ObjType_SceneMeshRemappedInstance)
				&&
				(pAftkInstance->GetSourceNodePtr()->GetType() != E_ObjType_SceneMeshRemappedInstance_REMAPPED_INSTANCE)
				&&
				(pAftkInstance->GetSourceNodePtr()->GetType() != E_ObjType_SceneMeshRemappedInstance_BONED_INSTANCE)
				&&
				(pAftkInstance->GetSourceNodePtr()->GetType() != AFTK_NODE_TYPE_SCENE_SPLINE)
				)
				continue;
		}
			
		else if (
			(pAftkSceneNode->GetType() != E_ObjType_SceneMeshRemappedInstance)
			&&
			(pAftkSceneNode->GetType() != E_ObjType_SceneMeshRemappedInstance_REMAPPED_INSTANCE)
			&&
			(pAftkSceneNode->GetType() != E_ObjType_SceneMeshRemappedInstance_BONED_INSTANCE)
			&&
			(pAftkSceneNode->GetType() != AFTK_NODE_TYPE_SCENE_SPLINE)
			)
			continue;

		AftkBounds aftkBounds = pAftkSceneNode->GetWorldBounds();

		m_sceneBounds.AddVertex(Tu::TemplateToNightmare(aftkBounds.GetMin()));
		m_sceneBounds.AddVertex(Tu::TemplateToNightmare(aftkBounds.GetMax()));

#ifdef _DEBUG
		if (sqrtf(m_sceneBounds.GetBoundingBoxRadiusSquared()) > 3000)
		{
//			assert(false);
		}
#endif
	}
}

void NexusSystem::SetSelectedSceneNodePtr(AftkSceneNode* pAftkSceneNode)
{
	m_pSelectedSceneNode = pAftkSceneNode;
}

AftkSceneNode* NexusSystem::GetSelectedSceneNodePtr()
{
	return m_pSelectedSceneNode;
}

void NexusSystem::SetLogCallback(void (*pLogCallbackFunc)(const std::string& text))
{
	m_pLogCallbackFunc = pLogCallbackFunc;
	#ifndef _DEBUG
	m_aftk.SetOutputNotificationCallback(pLogCallbackFunc);
	#endif
}
