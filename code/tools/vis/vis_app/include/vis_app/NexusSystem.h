#pragma once


namespace Nexus
{
	class NexusSystem
	{
	public:
		NexusSystem();

		~NexusSystem();

		Tu::Status Create();

		void Destroy();

		void Clear();

		bool RendererNeedsRebuilt() const;

		bool IsSceneLoaded() const;

        bool IsCreated() const;

		void NewScene();

		void OutputSceneBounds();

		Tu::Status OpenScene(const std::string& fileName);

		Tu::Status SaveScene(const std::string& fileName);

		const AftkScene* GetScenePtr() const;

		AftkScene* GetScenePtr();

		const Tu::Bounds& GetSceneBounds() const;

		void SetSelectedSceneNodePtr(AftkSceneNode* pAftkSceneNode);

		void SetLogCallback(void (*pLogCallbackFunc)(const std::string& text));

		AftkSceneNode* GetSelectedSceneNodePtr();

		AftkScene* AftkPtr();
		const AftkScene* AftkPtr() const;

	private:
		NexusSystem(const NexusSystem& rhs);
		const NexusSystem& operator=(const NexusSystem& rhs);

		void ComputeSceneBounds();

		void CleanUpAfterFailure();

		bool				m_isCreated;
		bool				m_sceneLoaded;
		HWND				m_renderWindowHandle;

		AftkScene			m_aftk;
		Tu::Bounds			m_sceneBounds;

		AftkSceneNode*		m_pSelectedSceneNode;

    	//Pointer to a function that provides a logging facility.
		void (*m_pLogCallbackFunc)(const std::string& text);
	};
}

