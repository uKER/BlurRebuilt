#include "stdafx.h"
#include <nexus_core/core.h>
#include <tools_shared/VersionStrings.inl>

//Disable warning C4005: 'M_PI' : macro redefinition
#ifdef NEXUS_MAYA
#pragma warning(push)
	#pragma warning(disable : 4005)
	#include <maya/MLibrary.h>
	#include <maya/MString.h>
	#include <maya/MGlobal.h>
	#include <Aftk/MayaIo/MayaMessageBinder.h>
#pragma warning(pop)
#endif

#ifdef NEXUS_ALIENBRAIN
#include <nexus_core/AlienBrainTasks.h>
#endif

#include <algorithm>
using namespace Nexus;

Nexus::Core* Nexus::g_pCore = 0;
Nexus::Progress* Nexus::g_pProgressCallback = 0;
MayaIo::C_MessageCallbackBinder* g_pMayaMessageBinder = 0;

Core::Core()
{
	m_pAftkScene = 0;
	m_isCreated = false;
	m_sceneLoaded = false;
	//m_pSpatialHierachy = 0;
	m_pPrintMessageCallback = 0;
	m_pPrintWarningCallback = 0;
	m_pPrintErrorCallback = 0;
	m_pSelectCallback = 0;

	m_pSceneChangedCallback = 0;
	m_pSceneGeometryChangedCallback = 0;
	m_pSceneCropSheetChangedCallback = 0;
#ifdef NEXUS_MAYA
	m_mayaStarted = false;
#endif
	m_pLuaState = 0;
	m_pProgressCallback = 0;

	g_pCore = this;
	g_pProgressCallback = 0;
	
	Clear();
}

Core::~Core()
{
}

Tu::Status Core::Create()
{
	#define DEF(e, id, str) g_aftkAttrTable[id] = str;
		#include <Aftk/AftkAttributeTypeMapping.h>
	#undef DEF

	Tu::Status status;
	if (m_isCreated) 
	{
		return Tu::Status::SUCCESS;
	}
	Aftk::C_Global::Create();
	m_pAftkScene = new Aftk::C_Scene();
	if (!m_pAftkScene)
		return Tu::Status::FAILURE;

	//Initialise Lua
	status = CreateLua();
	if (!status) return status;

	m_isCreated = true;
	InitializeCriticalSection(&m_criticalSection);

	//Filter out nulls from bounding box calculations.
	m_boundsFilter.PushBack(Aftk::E_ObjType_SceneNode);

	return Tu::Status::SUCCESS;
}

Tu::Status Core::CreateLua()
{
	m_pLuaState = lua_open();  /* create state */
	if (!m_pLuaState)
	{
		return Tu::Status(Tu::Status::OUT_OF_MEMORY, 1, "Cannot create Lua state: not enough memory");
	}

	//Register Lua functions
	AftkTasks::Register(m_pLuaState);
	AftkUtTasks::Register(m_pLuaState);
	GlobalTasks::Register(m_pLuaState);
	CSGenTasks::Register(m_pLuaState);
#ifdef NEXUS_MAYA
	MayaTasks::Register(m_pLuaState);
#endif
#ifdef NEXUS_ALIENBRAIN
	AlienBrainTasks::Register(m_pLuaState);
#endif
	return Tu::Status::SUCCESS;
}

void Core::Clear()
{
	if (m_pAftkScene)
		m_pAftkScene->Clear();
	m_sceneLoaded = false;

	memset(&m_selectedMeshStatistics, 0, sizeof(MeshStatistics));
	memset(&m_globalMeshStatistics, 0, sizeof(MeshStatistics));

	m_selectedNodeInfo.Clear();
	m_sceneFileName.clear();
	m_sceneViewDirty = true;
	m_status = Tu::Status::SUCCESS;
	m_exitCode = 0;
}

void Core::Destroy()
{
	Clear();
	DestroyLua();
	m_isCreated = false;
	delete m_pAftkScene;
	m_pAftkScene = 0;
	DeleteCriticalSection(&m_criticalSection);
	m_boundsFilter.Clear();
	Aftk::C_Global::Destroy();
	SAFE_DELETE_PTR(g_pMayaMessageBinder);
}

void Core::DestroyLua()
{
	if (m_pLuaState)
	{
		lua_close(m_pLuaState);
	}
}

Tu::Status Core::ExecuteString(const char* string)
{
	try
	{
		//Execute command.
		int errorCode = lua_dostring(m_pLuaState, string);
		if (errorCode != 0)
		{
			const char* pStr = lua_tostring(m_pLuaState, lua_gettop(m_pLuaState));
			m_exitCode = 1;
			return Tu::Status(Tu::Status::FAILURE, pStr);
		}
	}
	catch (LuaExitException ex)
	{
		m_exitCode = ex.exitCode;
		return Tu::Status::SUCCESS;
	}

	m_exitCode = 0;
	return Tu::Status::SUCCESS;
}

Tu::Status Core::ExecuteFile(const char* file)
{
	try
	{
		//Execute command.
		int errorCode = lua_dofile(m_pLuaState, file);
		if (errorCode != 0)
		{
			const char* pStr = lua_tostring(m_pLuaState, lua_gettop(m_pLuaState));
			m_exitCode = 1;
			return Tu::Status(Tu::Status::FAILURE, pStr);
		}
	}
	catch (LuaExitException ex)
	{
		m_exitCode = ex.exitCode;
		return Tu::Status::SUCCESS;
	}

	m_exitCode = 0;
	return Tu::Status::SUCCESS;
}

void Core::PrintMessage(const std::string& text)
{
	if (m_pPrintMessageCallback)
	{
		m_pPrintMessageCallback(text);
	}
}

void Core::PrintWarning(const std::string& text)
{
	if (m_pPrintWarningCallback)
	{
		m_pPrintWarningCallback(text);
	}
}

void Core::PrintError(const std::string& text)
{
	if (m_pPrintErrorCallback)
	{
		m_pPrintErrorCallback(text);
	}
}

Progress* Core::ProgressCallback()
{
	return m_pProgressCallback;
}

const Tu::Status& Core::GetStatus() const
{
	return m_status;
}

void Core::SetStatus(const Tu::Status& status)
{
	m_status = status;
}

bool Core::TryLock()
{
	BOOL result = TryEnterCriticalSection(&m_criticalSection);
	if (result) 
	{
		return true;
	}
	return false;
}

void Core::Lock()
{
	//Block thread while we're locked.
	EnterCriticalSection(&m_criticalSection);
}

void Core::Unlock()
{
	//Block thread while we're locked.
	LeaveCriticalSection(&m_criticalSection);
}

#ifdef NEXUS_MAYA
Tu::Status Core::InitialiseMaya()
{
	if (!m_mayaStarted)
	{
		MayaIo::C_MessageCallbackBinder* g_pMayaMessageBinder = new
			MayaIo::C_MessageCallbackBinder();
		MStatus mayaStatus = MLibrary::initialize(true, "Nexus", true);
		if (!mayaStatus) 
		{
			std::string errorString;
			errorString += "Failed to initialise Maya engine. [";
			errorString += mayaStatus.errorString().asChar();
			errorString += "].";
			PrintError(errorString);
			m_mayaStarted = false;
		}
		else
		{
			m_mayaStarted = true;
		}
		MStatus mStatus;
		mStatus = MGlobal::executeCommand(
			"scriptEditorInfo -e -suppressErrors true; scriptEditorInfo -e -suppressWarnings true; scriptEditorInfo -e -suppressInfo true; scriptEditorInfo -e -suppressResults true;", false, false);

		std::string command(
			"source biz_batchModeSetup.mel;");
		mayaStatus = MGlobal::executeCommand(MString(command.c_str()));
		if (!mayaStatus)
		{
			PrintError("Failed to perform auto-update.");
			return Tu::Status::FAILURE;
		}
	}
	return Tu::Status::SUCCESS;
}

bool Core::IsMayaInitialised() const
{
	return m_mayaStarted;
}

void Core::SetMayaInitialised(bool flag)
{
	m_mayaStarted = flag;
}

#endif

bool Core::IsCreated() const
{
	return m_isCreated;
}

const Aftk::C_Scene* Core::GetAftkScenePtr() const
{
	return m_pAftkScene;
}

Aftk::C_Scene* Core::GetAftkScenePtr()
{
	return m_pAftkScene;
}

const Core::MeshStatistics& Core::GetGlobalMeshStatistics() const
{
	return m_globalMeshStatistics;
}

const Core::MeshStatistics& Core::GetSelectedMeshStatistics() const
{
	return m_selectedMeshStatistics;
}

void Core::CleanUpAfterFailure()
{
}

bool Core::IsSceneLoaded() const
{
	return m_sceneLoaded;
}

void Core::OnSceneGeometryChanged()
{
	ComputeGlobalMeshStatistics();
	ComputeSelectedMeshStatistics();

	if (m_pSceneGeometryChangedCallback)
		m_pSceneGeometryChangedCallback(m_pProgressCallback);
}

void Core::OnSceneChanged()
{
	DeselectAll();
	ComputeGlobalMeshStatistics();

	if (m_pSceneChangedCallback)
		m_pSceneChangedCallback(m_pProgressCallback);
}

void Core::OnSceneCropSheetChanged()
{
}

void Core::AddMeshStatistics(
	Nexus::Core::MeshStatistics& meshStatistics,
	const C_SelectedNodeInfo& nodeInfo)
{
	if (!nodeInfo.pSceneNode)
		return;
	if (!nodeInfo.pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
		return;

	const Aftk::I_SceneMeshNode* pAftkMesh = 
		(const Aftk::I_SceneMeshNode*)nodeInfo.pSceneNode;

	for (U32 jLodLevel = 0; jLodLevel < pAftkMesh->GetNumLodLevels(); jLodLevel++)
	{
		if (nodeInfo.selectedComponents.Size() == 0)
		{
			meshStatistics.numPolygons += pAftkMesh->GetNumPolygons(jLodLevel);
			meshStatistics.numTriangles += pAftkMesh->GetNumTriangles(jLodLevel);
			meshStatistics.numPolygonVertices += pAftkMesh->GetNumPolygonVertices();
			meshStatistics.numVertexPositions += pAftkMesh->GetNumVertexPositions();
			meshStatistics.numVertexNormals += pAftkMesh->GetNumVertexNormals();
			if (meshStatistics.highestUVSet < pAftkMesh->GetNumTextureCoordinateSets())
			{
				meshStatistics.highestUVSet = pAftkMesh->GetNumTextureCoordinateSets();
			}
			for (U32 iSet = 0; iSet < pAftkMesh->GetNumTextureCoordinateSets(); iSet++)
			{
				meshStatistics.numVertexTangents += pAftkMesh->GetNumVertexTangents(iSet);
				meshStatistics.numVertexBinormals += pAftkMesh->GetNumVertexBinormals(iSet);
				meshStatistics.numUVs += pAftkMesh->GetNumTextureCoordinates(iSet);
			}
		}
		else
		{
			meshStatistics.numPolygons += nodeInfo.selectedComponents.Size();
			for (Ut::UtSet<C_ComponentSelection>::ConstIterator polyIt = nodeInfo.selectedComponents.Begin(); 
				polyIt != nodeInfo.selectedComponents.End(); ++polyIt)
			{
				const C_ComponentSelection& selection = polyIt->GetKey();
				const Aftk::C_Polygon& poly = 
					pAftkMesh->GetPolygon(jLodLevel, selection.componentIndex);
				meshStatistics.numTriangles += poly.GetNumTriangles();
				meshStatistics.numPolygonVertices += poly.GetNumPolygonVertices();
			}
		}
	}
}

void Core::ComputeGlobalMeshStatistics()
{
	memset(&m_globalMeshStatistics, 0, sizeof(MeshStatistics));
	Aftk::C_SceneNodeConstIterator it(m_pAftkScene);
	while(it)
	{
		AddMeshStatistics(m_globalMeshStatistics, C_SelectedNodeInfo(it));
		it++;
	}
}

void Core::ComputeSelectedMeshStatistics()
{
	memset(&m_selectedMeshStatistics, 0, sizeof(MeshStatistics));
	for (Ut::UtMap<Aftk::C_SceneNode*, C_SelectedNodeInfo>::Iterator it = m_selectedNodeInfo.Begin();
		it != m_selectedNodeInfo.End(); ++it)
	{
		AddMeshStatistics(m_selectedMeshStatistics, it->GetValue());
	}
}

void Core::DeselectAll()
{
	m_selectedNodeInfo.Clear();
	ComputeSelectedMeshStatistics();
}

void Core::SelectLayer(U32 index)
{
	Aftk::C_LayerNode* pLayer = m_pAftkScene->GetLayerPtr(index);
	for (U32 i = 0; i < pLayer->GetNumMembers(); i++)
	{
		Aftk::C_SceneNode* pSceneNode = pLayer->GetMemberPtr(i);
		if (!pSceneNode)
			continue;
		Select(E_SelectionMode_Object, pSceneNode);
	}
}

void Core::DeselectLayer(U32 index)
{
	Aftk::C_LayerNode* pLayer = m_pAftkScene->GetLayerPtr(index);
	for (U32 i = 0; i < pLayer->GetNumMembers(); i++)
	{
		Aftk::C_SceneNode* pSceneNode = pLayer->GetMemberPtr(i);
		if (!pSceneNode)
			continue;
		Deselect(E_SelectionMode_Object, pSceneNode);
	}
}

void Core::Select(E_SelectionMode mode, const C_SelectedNodeInfo& info)
{
	//Abort selection if we're in face mode with no selected components.
	if (mode == E_SelectionMode_Face && info.selectedComponents.Size() == 0)
		return;

	Ut::UtMap<Aftk::C_SceneNode*, C_SelectedNodeInfo>::Iterator it = 
		m_selectedNodeInfo.Find(info.pSceneNode);

	if (it == m_selectedNodeInfo.End())
	{
		m_selectedNodeInfo[info.pSceneNode] = info;
		AddMeshStatistics(m_selectedMeshStatistics, info);

		if (m_pSelectCallback)
			m_pSelectCallback(mode, info);
		return;
	}
	else
	{
		for (Ut::UtSet<C_ComponentSelection>::ConstIterator it2 = info.selectedComponents.Begin();
			it2 != info.selectedComponents.End(); ++it2)
		{
			Ut::UtSet<C_ComponentSelection>::InsertionReturnType returnType = 
				it->GetValue().selectedComponents.Insert(it2->GetKey());

			AddMeshStatistics(m_selectedMeshStatistics, info);

			if (returnType.second && m_pSelectCallback)
				m_pSelectCallback(mode, info);
		}
	}
}

void Core::Deselect(E_SelectionMode mode, const C_SelectedNodeInfo& info)
{
	Ut::UtMap<Aftk::C_SceneNode*, C_SelectedNodeInfo>::Iterator it = 
		m_selectedNodeInfo.Find(info.pSceneNode);

	if (it != m_selectedNodeInfo.End())
	{
		switch (mode)
		{
			case E_SelectionMode_Object:
				m_selectedNodeInfo.Erase(it);
				break;

			case E_SelectionMode_Face:
				{
					//Deselect faces from this mesh.
					for (
						Ut::UtSet<C_ComponentSelection>::ConstIterator it2 = info.selectedComponents.Begin();
						it2 != info.selectedComponents.End(); ++it2)
					{
						it->GetValue().selectedComponents.Erase(it2->GetKey());
					}
					//If we've no more faces selected, delete reference to this node.
					if (it->GetValue().selectedComponents.Size() == 0)
					{
						m_selectedNodeInfo.Erase(it);
					}
				}
				
				break;

			default:
				break;
		}

		ComputeSelectedMeshStatistics();
		return;
	}
}

void Core::ToggleSelected(E_SelectionMode mode, const C_SelectedNodeInfo& info)
{
	Ut::UtMap<Aftk::C_SceneNode*, C_SelectedNodeInfo>::Iterator it = 
		m_selectedNodeInfo.Find(info.pSceneNode);

	if (it != m_selectedNodeInfo.End())
	{
		C_SelectedNodeInfo toSelect;
		C_SelectedNodeInfo toDeselect;
		toSelect.pSceneNode = info.pSceneNode;
		toDeselect.pSceneNode = info.pSceneNode;
		//Build a list of faces to deselect.
		if (mode == E_SelectionMode_Face)
		{
			for (Ut::UtSet<C_ComponentSelection>::ConstIterator it2 = info.selectedComponents.Begin();
				it2 != info.selectedComponents.End();
				++it2)
			{
				//Determine if any of the given faces are already selected.
				Ut::UtSet<C_ComponentSelection>::ConstIterator it3 = 
					it->GetValue().selectedComponents.Find(it2->GetKey());
				if (it3 != it->GetValue().selectedComponents.End())
				{
					toDeselect.selectedComponents.Insert(it2->GetKey());
				}
				else
				{
					toSelect.selectedComponents.Insert(it2->GetKey());
				}

			}

			Deselect(mode, toDeselect);
			Select(mode, toSelect);
			return;
		}
		else
		{
			Deselect(mode, info);
			return;
		}
	}
	Select(mode, info);
}

const std::string& Core::GetSceneFileName() const
{
	return m_sceneFileName;
}

void Core::SetSceneFileName(const std::string& fileName)
{
	m_sceneFileName = fileName;
}

const AftkSceneNodeMap& Core::GetSelected() const
{
	return m_selectedNodeInfo;
}

AftkSceneNodeMap& Core::GetSelected()
{
	return m_selectedNodeInfo;
}

U32 Core::GetNumSelected() const
{
	return (U32)m_selectedNodeInfo.Size();
}

void Core::SetProgressCallback(Progress* pProgress)
{
	m_pProgressCallback = pProgress;
	g_pProgressCallback = m_pProgressCallback;
}

bool Core::IsSceneViewDirty() const
{
	return m_sceneViewDirty;
}

void Core::SetIsSceneViewDirty(bool isDirty)
{
	m_sceneViewDirty = isDirty;
}

void Core::SetPrintMessageCallback(void (*pCallback)(const std::string& text))
{
	m_pPrintMessageCallback = pCallback;
#pragma message("FIX THIS")
//	m_pAftkScene->SetOutputNotificationCallback(pCallback);
}

void Core::SetPrintWarningCallback(void (*pCallback)(const std::string& text))
{
	m_pPrintWarningCallback = pCallback;
}

void Core::SetPrintErrorCallback(void (*pCallback)(const std::string& text))
{
	m_pPrintErrorCallback = pCallback;
}

void Core::SetOnSceneChangedCallback(Tu::Status (*pCallback)(Nexus::Progress* pProgress))
{
	m_pSceneChangedCallback = pCallback;
}

void Core::SetOnSceneGeometryChangedCallback(Tu::Status (*pCallback)(Nexus::Progress* pProgress))
{
	m_pSceneGeometryChangedCallback = pCallback;
}

void Core::SetOnSceneCropSheetDatabaseChangedCallback(Tu::Status (*pCallback)(Nexus::Progress* pProgress))
{
	m_pSceneCropSheetChangedCallback = pCallback;
}

OnChangedCallback Core::OnSceneChangedCallback()
{
	return m_pSceneChangedCallback;
}

OnChangedCallback Core::OnSceneGeometryChangedCallback()
{
	return m_pSceneGeometryChangedCallback;
}

OnChangedCallback Core::OnSceneCropSheetDatabaseChangedCallback()
{
	return m_pSceneCropSheetChangedCallback;
}

void Core::SetIsSceneLoaded(bool flag)
{
	m_sceneLoaded = flag;
}

Aftk::TextCallback Core::GetPrintMessageCallback()
{
	return m_pPrintMessageCallback;
}

Aftk::TextCallback Core::GetPrintWarningCallback()
{
	return m_pPrintWarningCallback;
}

Aftk::TextCallback Core::GetPrintErrorCallback()
{
	return m_pPrintErrorCallback;
}

Aftk::ObjTypeArray& Core::GetBoundsFilter()
{
	return m_boundsFilter;
}

const Aftk::ObjTypeArray& Core::GetBoundsFilter() const
{
	return m_boundsFilter;
}

S32 Core::GetExitCode() const
{
	return m_exitCode;
}

void Core::SetSelectCallback(OnSelectCallback pCallback)
{
	m_pSelectCallback = pCallback;
}
