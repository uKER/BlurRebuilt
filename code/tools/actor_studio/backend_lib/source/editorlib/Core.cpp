#include "stdafx.h"

#include <Core/Threads/Atomic.h>
#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/GlobalSignals.h>

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRefHelpers.h>

#include <EditorLib/Collision/Collision.h>

#include <EditorLib/Python/PythonManager.h>
#include <EditorLib/Network/NetworkManager.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/CommandFactory.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/Commands.h>

#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgXtGameDataNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneAftkActorNode.h>
#include <EditorLib/Dg/SceneXtNode.h>

#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/GuidManager.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Misc/Verification.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtVisualisation.h>
#include <EditorLib/Xt/XtUserData.h>
#include <EditorLib/Xt/Export/XtBuildLevelPack.h>

#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_ViewDisabler::C_ViewDisabler()
{
	C_Core::Instance().EnableViews(false);
}

C_ViewDisabler::~C_ViewDisabler()
{
	C_Core::Instance().EnableViews(true);
}

C_Core::C_Core()
{
	m_pLogger = &Debug::GetLogger("tools.editorlib");

	m_isInitialised = false;
	m_hwnd = 0;

	m_pCommandFactory = new C_CommandFactory(&C_Core::PostCommandReg);
	m_pDgNodeFactory = new C_DgNodeFactory();
}

C_Core::~C_Core()
{
}

C_Status C_Core::Initialise(
	E_AppType appType,
	const Btl::String& configFilePath)
{
	m_pConfig = new C_CoreConfig;
	m_pConfig->appType = appType;

	if (!m_pConfig->Load(configFilePath.Chars()))
		return E_Status_Failure;

	RegisterInternalNodes();

	m_pGlobalSignals = new C_GlobalSignals();
	m_pGuidManager = new C_GuidManager();
	m_pDgManager = new C_DgManager();
	m_pCommandProcessor = new C_CommandProcessor();
	m_pXtManager = new C_XtManager();
	m_pXtTypeDatabase = new C_XtTypeDatabase();
	m_pXtVisualisationData = new C_XtVisualisationData();
	m_pNetworkManager = new C_NetworkManager();
	m_pVerificationSystem = new C_VerificationSystem();
	m_pXtUserData = new C_XtUserData();
	m_pAftkRefManager = new C_AftkRefManager();
	m_pLayerManager = new C_LayerManager();
	m_pCollisionManager = new C_CollisionManager();
	m_pPythonManager = new C_PythonManager();
	m_pLevelManager = new C_LevelManager();
	m_pUndoManager = new C_UndoManager();

	m_viewDisabledRefCount = 0;

	m_isInitialised = true;
	
	m_pAftkRefManager->SetCacheDirectory(m_pConfig->a3dCachePath);
	m_pAftkRefManager->SetConversionTool(
		m_pConfig->conversionToolPath,
		m_pConfig->conversionToolParams,
		m_pConfig->conversionToolMetaQueryParams);

	if (!m_pXtUserData->Open(m_pConfig->rootTempPath + "/actorstudio/userdata"))
	{
		LOGERROR(GetLogger(), "Unable to open user data database at location %s%s",
			m_pConfig->rootTempPath.Chars(), "/actorstudio/userdata");
	}

	//Bootstrap python.
	if (!m_pPythonManager->Initialise())
		return E_Status_Failure;

	RegisterInternalCommands();
	return E_Status_Success;
}

void C_Core::Shutdown(bool quick)
{
	assert(m_isInitialised);

	LOGINFO(GetLogger(), "Shutting down core...");

	//Perform reverse of initialisation.
	if (!quick)
	{
		LOGINFO(GetLogger(), "Uninitialising : Level Manager...");
		m_pLevelManager->Close();
		SAFE_DELETE_PTR(m_pLevelManager);
		LOGINFO(GetLogger(), "Uninitialising : Python Manager...");
		SAFE_DELETE_PTR(m_pPythonManager);
		LOGINFO(GetLogger(), "Uninitialising : Collision Manager...");
		SAFE_DELETE_PTR(m_pCollisionManager);
		LOGINFO(GetLogger(), "Uninitialising : Aftk Ref Manager...");
		SAFE_DELETE_PTR(m_pAftkRefManager);
		LOGINFO(GetLogger(), "Uninitialising : Layer Manager...");
		SAFE_DELETE_PTR(m_pLayerManager);
		LOGINFO(GetLogger(), "Uninitialising : Xt Visualisation Data...");
		SAFE_DELETE_PTR(m_pXtVisualisationData);
		LOGINFO(GetLogger(), "Uninitialising : Xt Type Database...");
		SAFE_DELETE_PTR(m_pXtTypeDatabase);
		LOGINFO(GetLogger(), "Uninitialising : Xt Manager...");
		SAFE_DELETE_PTR(m_pXtManager);
	}
	LOGINFO(GetLogger(), "Uninitialising : Xt User Data...");
	SAFE_DELETE_PTR(m_pXtUserData);
	if (!quick)
	{
		LOGINFO(GetLogger(), "Uninitialising : Network Manager...");
		m_pNetworkManager->Destroy();
		SAFE_DELETE_PTR(m_pNetworkManager);
		LOGINFO(GetLogger(), "Uninitialising : Verification System...");
		SAFE_DELETE_PTR(m_pVerificationSystem);
		LOGINFO(GetLogger(), "Uninitialising : Command Processor...");
		SAFE_DELETE_PTR(m_pCommandProcessor);
		LOGINFO(GetLogger(), "Uninitialising : Dg Manager...");
		SAFE_DELETE_PTR(m_pDgManager);
		LOGINFO(GetLogger(), "Uninitialising : Dg Node Factory...");
		SAFE_DELETE_PTR(m_pDgNodeFactory);
		LOGINFO(GetLogger(), "Uninitialising : Command Factory...");
		SAFE_DELETE_PTR(m_pCommandFactory);
		LOGINFO(GetLogger(), "Uninitialising : Guid Manager...");
		SAFE_DELETE_PTR(m_pGuidManager);
		LOGINFO(GetLogger(), "Uninitialising : Undo Manager...");
		SAFE_DELETE_PTR(m_pUndoManager);
		LOGINFO(GetLogger(), "Uninitialising : Global Signals...");
		SAFE_DELETE_PTR(m_pGlobalSignals);
		LOGINFO(GetLogger(), "Uninitialising : Config...");
		SAFE_DELETE_PTR(m_pConfig);
		LOGINFO(GetLogger(), "Core has shut down.");
	}
	else
		LOGINFO(GetLogger(), "Core has shut down with extreme prejudice.");

	m_isInitialised = false;
}

C_Status C_Core::ExportLevel()
{
	if (m_pLevelManager->GetActiveLevel())
	{
		// Run verification on current level
		/*
		if (!GetVerificationSystem().VerifyLevel(m_pLevelManager->GetActiveLevel()->GetXtRecordId()))
			LOGWARN(GetLogger(), "Verification failed...");
		*/

		//Ensure uniqueness within dirty records.
		GetXtManager().EnsureUniqueUids(true);

		C_PackExportParams params;
		params.packId = m_pLevelManager->GetActiveLevel()->GetXtPackId();
		return XtBuildLevelPack::Build((HWND)m_hwnd, &params) ? E_Status_Success : E_Status_Failure;
	}
	else
		return E_Status_Failure;

	return E_Status_Success;
}

C_GuidManager& C_Core::GetGuidManager()
{
	return *m_pGuidManager;
}

C_CommandFactory& C_Core::GetCommandFactory()
{
	return *m_pCommandFactory;
}

C_DgNodeFactory& C_Core::GetDgNodeFactory()
{
	return *m_pDgNodeFactory;
}

C_DgManager& C_Core::GetDgManager()
{
	return *m_pDgManager;
}

C_CommandProcessor& C_Core::GetCommandProcessor()
{
	return *m_pCommandProcessor;
}

C_XtManager& C_Core::GetXtManager()
{
	return *m_pXtManager;
}

C_XtTypeDatabase& C_Core::GetXtTypeDatabase()
{
	return *m_pXtTypeDatabase;
}

C_XtVisualisationData& C_Core::GetXtVisualisationData()
{
	return *m_pXtVisualisationData;
}

C_XtUserData& C_Core::GetXtUserData()
{
	return *m_pXtUserData;
}

C_AftkRefManager& C_Core::GetAftkRefManager()
{
	return *m_pAftkRefManager;
}

C_LayerManager& C_Core::GetLayerManager()
{
	return *m_pLayerManager;
}

C_CollisionManager& C_Core::GetCollisionManager()
{
	return *m_pCollisionManager;
}

C_PythonManager& C_Core::GetPythonManager()
{
	return *m_pPythonManager;
}

C_NetworkManager& C_Core::GetNetworkManager()
{
	return *m_pNetworkManager;
}

C_VerificationSystem& C_Core::GetVerificationSystem()
{
	return *m_pVerificationSystem;
}

C_GlobalSignals& C_Core::GetGlobalSignals() const
{
	return *m_pGlobalSignals;
}

C_LevelManager* C_Core::GetLevelManager()
{
	return m_pLevelManager;
}

C_UndoManager& C_Core::GetUndoManager()
{
	return *m_pUndoManager;
}

void C_Core::RegisterInternalCommands()
{
	REGISTER_COMMAND(C_QuitCmd)
	REGISTER_COMMAND(C_FileCmd)
	REGISTER_COMMAND(C_XtDbCmd)
	REGISTER_COMMAND(C_CreateXtNodeCmd)	
	REGISTER_COMMAND(C_CreateAnimationCmd)
	REGISTER_COMMAND(C_CreateGroupCmd)	
	REGISTER_COMMAND(C_XFormCmd)
	REGISTER_COMMAND(C_ParentCmd)
	REGISTER_COMMAND(C_DeleteCmd)
	REGISTER_COMMAND(C_CopyCmd)
	REGISTER_COMMAND(C_PasteCmd)	
	REGISTER_COMMAND(C_DuplicateCmd)
	REGISTER_COMMAND(C_ExportCmd)
	REGISTER_COMMAND(C_SelectCmd)	
	REGISTER_COMMAND(C_UndoCmd)
	REGISTER_COMMAND(C_RedoCmd)	
	REGISTER_COMMAND(C_StoreChangesCmd)
	REGISTER_COMMAND(C_RenameCmd)
	REGISTER_COMMAND(C_ExtActorsRefreshCmd)
	REGISTER_COMMAND(C_ExtActorsOfflineRefreshCmd)
	REGISTER_COMMAND(C_BuildLevelPackCmd)
	REGISTER_COMMAND(C_GetLatestReferencesCmd)
}

void C_Core::RegisterInternalNodes()
{
	REGISTER_DG_NODE(C_DgXtGameDataNode)
	REGISTER_DG_NODE(C_SceneNode)
	REGISTER_DG_NODE(C_SceneAftkNode)
	REGISTER_DG_NODE(C_SceneAftkActorNode)
	REGISTER_DG_NODE(C_SceneXtActorNode)
	REGISTER_DG_NODE(C_SceneXtNode)
	REGISTER_DG_NODE(C_DgXtCompNode)
}

void C_Core::SetClipboard(const C_Clipboard& clipboard)
{
	GetUndoManager().SetClipboard(clipboard);
}

void C_Core::SetSelection(const C_Selection& selection)
{
	GetUndoManager().SetSelection(selection);

	//Objects hooked into the selection changed signal might behave in a quirky way
	//if they're dependent upon updates that have not yet been processed. Flush now.
	C_Core::Instance().GetDgManager().FlushDeferredUpdates();

	C_Core::Instance().GetGlobalSignals().coreSelectionChanged();
}

void C_Core::IdleTick()
{
	GetUndoManager().StoreChanges();
	m_pDgManager->FlushDeferredUpdates();
}

void C_Core::SetWindowHandle(void* hwnd)
{
	m_hwnd = hwnd;
}

void* C_Core::GetWindowHandle() const
{
	return m_hwnd;
}

C_Status C_Core::PostCommandReg(const char* pName)
{
	return C_Core::Instance().GetPythonManager().RegisterCommand(pName);
}

void C_Core::GetClipboard(C_Clipboard& clipboard) const
{
	m_pUndoManager->GetClipboard(clipboard);
}

void C_Core::GetSelection(C_Selection& selection) const
{
	m_pUndoManager->GetSelection(selection);
}

void C_Core::Quit()
{
	m_pGlobalSignals->quit();
}

void C_Core::EnableViews(bool enable)
{
	//Increment count of disabled views.
	if (!enable)
	{
		if (MT::AtomicIncrement(&m_viewDisabledRefCount) == 0)
			//Disable views
			C_Core::Instance().GetGlobalSignals().coreEnableViews(false);
	}
	else 
	{
		U32 val = MT::AtomicDecrement(&m_viewDisabledRefCount);
		if (val == 1)
		{
			//Enable views
			C_Core::Instance().GetGlobalSignals().coreEnableViews(true);
			return;
		}
		if (val <= 0)
			BIZ_ASSERT(!"View enable/disable mismatch!");
	}
}

void C_Core::DirtyViews()
{
	GetGlobalSignals().coreDirtyViews(E_DirtyEventType_Unknown);
}

void C_Core::SetContext(const Btl::String &context, const void* contextData)
{
	m_EditorContext = context;
	C_Core::Instance().GetGlobalSignals().coreContextChanged(context, contextData);
}

const Btl::String& C_Core::GetContext()
{
	return m_EditorContext;
}

const C_CoreConfig& C_Core::GetConfig() const
{
	return *m_pConfig;
}

Debug::Logger& C_Core::GetLogger() const
{
	return *m_pLogger;
}

void C_Core::OnOpenXtDb()
{
	//replace xtdb path specified in coreconfig
	m_pConfig->xtDbPath = m_pXtManager->GetDb()->path.Text();
	m_pXtTypeDatabase->RegisterTypes();
}

void C_Core::OnCloseXtDb()
{
	if (m_pLevelManager)
		m_pLevelManager->Close();

	if (m_pXtTypeDatabase)
		m_pXtTypeDatabase->UnregisterTypes();
}

}
