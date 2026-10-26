#ifndef EDITOR_LIB_CORE_H
#define EDITOR_LIB_CORE_H

#ifdef _MSC_VER_
#pragma once
#endif

#pragma warning(push)
#pragma warning(disable:4355)
#include <xt/xttool.h>
#pragma warning(pop)

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Context/DeleteContextFactory.h>
#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Definitions.h>

namespace EditorLib
{
	class C_CopyCmd;
	class C_PasteCmd;
	class C_SelectCmd;
	class C_DeleteCmd;

	class C_ViewDisabler
	{
	public:
		C_ViewDisabler();
		~C_ViewDisabler();
	};

	class C_Core : public Ut::Singleton< C_Core, Ut::CreateUsingNew >
	{
	public:
		C_Core();
		virtual ~C_Core();

		C_Status Initialise(
			E_AppType appType,
			const Btl::String& configFilePath);

		void Shutdown(bool quick = false);

		C_Status ExportLevel();
		
		C_GuidManager& GetGuidManager();

		C_CommandFactory& GetCommandFactory();

		C_DgNodeFactory& GetDgNodeFactory();

		C_DgManager& GetDgManager();

		C_CommandProcessor& GetCommandProcessor();

		C_XtManager& GetXtManager();

		C_XtTypeDatabase& GetXtTypeDatabase();

		C_XtVisualisationData& GetXtVisualisationData();

		C_XtUserData& GetXtUserData();

		C_AftkRefManager& GetAftkRefManager();

		C_LayerManager& GetLayerManager();

		C_CollisionManager& GetCollisionManager();

		C_PythonManager& GetPythonManager();

		C_NetworkManager& GetNetworkManager();

		C_VerificationSystem& GetVerificationSystem();

		C_UndoManager& GetUndoManager();

		C_GlobalSignals& GetGlobalSignals() const;

		void GetClipboard(C_Clipboard& clipboard) const;

		void GetSelection(C_Selection& selection) const;

		C_LevelManager* GetLevelManager();

		const C_CoreConfig& GetConfig() const;

		Debug::Logger& GetLogger() const;

		void Quit();

		void DirtyViews();

		void SetContext(const Btl::String& context, const void* contextData=0);

		const Btl::String& GetContext();

		// TODO: Consider the placement of this selection function for when Undo is implemented
		void SetSelection(const C_Selection& selection);

		void IdleTick();

		//These are temporary until we've got a generic implementation of Cakey's async invoke.
		void SetWindowHandle(void* hwnd);
		void* GetWindowHandle() const;

	private:
		static C_Status PostCommandReg(const char* pName);

		void RegisterInternalCommands();
		void RegisterInternalNodes();
		bool ParseConfig(const Btl::String& config);

		void SetClipboard(const C_Clipboard& clipboard);
		
	private:
		void EnableViews(bool enable);

	private:
		//Event hooks.
		void OnOpenXtDb();
		void OnCloseXtDb();

		C_GuidManager*			m_pGuidManager;
		C_CommandFactory*		m_pCommandFactory;
		C_DgManager*			m_pDgManager;
		C_DgNodeFactory*		m_pDgNodeFactory;
		C_CommandProcessor*		m_pCommandProcessor;
		C_XtManager*			m_pXtManager;
		C_XtTypeDatabase*		m_pXtTypeDatabase;
		C_CollisionManager*		m_pCollisionManager;
		C_XtVisualisationData*	m_pXtVisualisationData;
		C_XtUserData*			m_pXtUserData;
		C_AftkRefManager*		m_pAftkRefManager;
		C_LayerManager*			m_pLayerManager;
		C_PythonManager*		m_pPythonManager;
		C_NetworkManager*		m_pNetworkManager;
		C_VerificationSystem*	m_pVerificationSystem;
		C_UndoManager*			m_pUndoManager;

		C_LevelManager*			m_pLevelManager;

		mutable C_GlobalSignals*	
								m_pGlobalSignals;

		volatile S32			m_viewDisabledRefCount;

		bool					m_isInitialised;
		C_CoreConfig*			m_pConfig;
		mutable Debug::Logger*	m_pLogger;

		Btl::String				m_EditorContext;
				
		void*					m_hwnd;

		FRIEND_CLASS C_UndoManager;
		FRIEND_CLASS C_XtManager;
		FRIEND_CLASS C_CopyCmd;
		FRIEND_CLASS C_PasteCmd;
		FRIEND_CLASS C_SelectCmd;
		FRIEND_CLASS C_DeleteCmd;
		FRIEND_CLASS C_ViewDisabler;
	};
}

template<typename TYPE>
INLINE void RegisterCommand() { EditorLib::C_Core::Instance().GetCommandFactory().Register<TYPE>(); }
template<typename TYPE>
INLINE void RegisterDgNode() { 	EditorLib::C_Core::Instance().GetDgNodeFactory().Register<TYPE>(); }
template<typename TYPE>
INLINE void RegisterDeleteContext() { EditorLib::C_DeleteContextFactory::Instance().Register<TYPE>(); }

#define REGISTER_COMMAND(type)				RegisterCommand<type>();
#define REGISTER_DG_NODE(type)				RegisterDgNode<type>();
#define REGISTER_DELETE_CONTEXT(type)		RegisterDeleteContext<type>();

#define REGISTER_COMMANDS(name) { void Register##name##Commands(); Register##name##Commands(); }
#define REGISTER_DG_NODES(name) { void Register##name##DgNodes(); Register##name##DgNodes(); }

#endif
