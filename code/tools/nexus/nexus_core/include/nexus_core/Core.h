#ifndef NEXUS_CORE_CORE_H
#define NEXUS_CORE_CORE_H

struct lua_State;

namespace Nexus
{
	class Core
	{
	public:
		class LuaExitException
		{
		public:
			LuaExitException(S32 code)
			{
				exitCode = code;
			}
			int exitCode;
		};

		struct MeshStatistics
		{
			U32 numVertexPositions;
			U32 numVertexNormals;
			U32 numPolygonVertices;
			U32	numTriangles;
			U32	numPolygons;
			U32 numUVs;
			U32 numVertexTangents;
			U32 numVertexBinormals;
			U32 highestUVSet;
		};

		Core();

		~Core();

		Tu::Status Create();

		void Destroy();

		void Clear();

		bool IsSceneLoaded() const;

        bool IsCreated() const;

		const Aftk::C_Scene* GetAftkScenePtr() const;

		Aftk::C_Scene* GetAftkScenePtr();

		Tu::Status ExecuteString(const char* string);

		Tu::Status ExecuteFile(const char* file);

		void PrintMessage(const std::string& text);

		void PrintError(const std::string& text);

		void PrintWarning(const std::string& text);

		Progress* ProgressCallback();

		void SetProgressCallback(Progress* pProgress);

		void SetPrintMessageCallback(void (*pPrintCallbackFunc)(const std::string& text));

		void SetPrintWarningCallback(void (*pPrintCallbackFunc)(const std::string& text));

		void SetPrintErrorCallback(void (*pPrintCallbackFunc)(const std::string& text));

		void SetOnSceneChangedCallback(Tu::Status (*pCallbackFunc)(Nexus::Progress* pProgress));

		void SetOnSceneGeometryChangedCallback(Tu::Status (*pCallbackFunc)(Nexus::Progress* pProgress));

		void SetOnSceneCropSheetDatabaseChangedCallback(Tu::Status (*pCallbackFunc)(Nexus::Progress* pProgress));

		Aftk::TextCallback GetPrintMessageCallback();

		Aftk::TextCallback GetPrintWarningCallback();

		Aftk::TextCallback GetPrintErrorCallback();

		OnChangedCallback OnSceneChangedCallback();

		OnChangedCallback OnSceneGeometryChangedCallback();

		OnChangedCallback OnSceneCropSheetDatabaseChangedCallback();

		void SetSelectCallback(OnSelectCallback pCallback);

		void DeselectAll();

		void SelectLayer(U32 index);

		void DeselectLayer(U32 index);

		void Select(E_SelectionMode mode, const C_SelectedNodeInfo& info);

		void Deselect(E_SelectionMode mode, const C_SelectedNodeInfo& info);

		void ToggleSelected(E_SelectionMode mode, const C_SelectedNodeInfo& info);

		const AftkSceneNodeMap& GetSelected() const;

		AftkSceneNodeMap& GetSelected();

		U32 GetNumSelected() const;

		const MeshStatistics& GetGlobalMeshStatistics() const;
		
		const MeshStatistics& GetSelectedMeshStatistics() const;

		bool TryLock();

		void Lock();

		void Unlock();

		const std::string& GetSceneFileName() const;

		void SetIsSceneViewDirty(bool isDirty);

		bool IsSceneViewDirty() const;

		void SetStatus(const Tu::Status& status);

		const Tu::Status& GetStatus() const;

		Aftk::ObjTypeArray& GetBoundsFilter();

		const Aftk::ObjTypeArray& GetBoundsFilter() const;

		S32 GetExitCode() const;

#ifdef NEXUS_MAYA
		Tu::Status InitialiseMaya();

		bool IsMayaInitialised() const;

		void SetMayaInitialised(bool flag);
#endif

	private:
		Core(const Core& rhs);

		const Core& operator=(const Core& rhs);

		void OnSceneChanged();

		void OnSceneGeometryChanged();

		void OnSceneCropSheetChanged();

		void AddMeshStatistics(
			MeshStatistics& meshStatistics,
			const C_SelectedNodeInfo& nodeInfo);

		void ComputeGlobalMeshStatistics();

		void ComputeSelectedMeshStatistics();

		void SetIsSceneLoaded(bool flag);

		void CleanUpAfterFailure();

		Tu::Status CreateLua();

		void DestroyLua();

		void SetSceneFileName(const std::string& fileName);

		bool				m_isCreated;
#ifdef NEXUS_MAYA
		bool				m_mayaStarted;
#endif
		bool				m_sceneLoaded;

		Aftk::C_Scene*		m_pAftkScene;

		Ut::UtMap<Aftk::C_SceneNode*, C_SelectedNodeInfo> 
							m_selectedNodeInfo;

		lua_State*			m_pLuaState;

    	//Pointer to a function that provides a logging facility.
		void (*m_pPrintMessageCallback)(const std::string& text);
		void (*m_pPrintWarningCallback)(const std::string& text);
		void (*m_pPrintErrorCallback)(const std::string& text);

		OnChangedCallback	m_pSceneChangedCallback;
		OnChangedCallback	m_pSceneGeometryChangedCallback;
		OnChangedCallback	m_pSceneCropSheetChangedCallback;
		OnSelectCallback	m_pSelectCallback;

		Progress*			m_pProgressCallback;

		bool				m_sceneViewDirty;

		MeshStatistics		m_selectedMeshStatistics;
		MeshStatistics		m_globalMeshStatistics;

		//Lock for thread-safety.
		CRITICAL_SECTION	m_criticalSection;
		std::string			m_sceneFileName;

		Aftk::ObjTypeArray	m_boundsFilter;

		Tu::Status			m_status;
		S32					m_exitCode;

		friend class AftkTasks;
		friend class AftkUtTasks;
		friend class CSGenTasks;
	};
}

#endif
