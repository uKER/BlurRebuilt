#ifndef NEXUS_NEXUSSYSTEM_H
#define NEXUS_NEXUSSYSTEM_H

namespace Nexus
{
	class NexusProgressDialog;

	class NexusSystem
	{
	public:
		NexusSystem();

		~NexusSystem();

		Tu::Status Create();

		void Destroy();

		void Clear();

		void NewScene();

		bool IsCreated() const;

		Tu::Status OpenScene(const std::string& fileName, bool import, bool& cancelled);

		Tu::Status SaveScene(const std::string& fileName, bool& cancelled);

		void SetLogCallback(void (*pLogCallbackFunc)(const std::string& text));

		static void OnSelect(E_SelectionMode mode, const C_SelectedNodeInfo& selection);

		const Core* GetCorePtr() const;

		Core* GetCorePtr();

		void UpdateSceneNodeSelection();

		C_ApplicationState& GetState();

		const C_ApplicationState& GetState() const;

		void SetTime(F32 time);

		void UpdateTime(F32 time);
	
		bool AnimationPaused() const;

		void ToggleAnimation();

		void ResetAnimation();

		void FrameSkip(S32 skip);

		E_ViewportState GetViewportState() const;

		void MouseButtonEvent(
			U32 viewportIndex,
			int buttonId, 
			E_MouseButtonAction action,
			UINT nFlags,
			CPoint point);

		void MouseButtonStateChanged(
			U32 viewportIndex,
			int buttonId);

		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

		void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

		E_SelectionState GetSelectionState() const;

		U32 GetActiveViewportIndex() const;

		LARGE_INTEGER GetFrameStartTime() const;

		LARGE_INTEGER GetTimerFrequency() const;

		void SetFrameStartTime(LARGE_INTEGER startTime);

	private:
		NexusSystem(const NexusSystem& rhs);

		const NexusSystem& operator=(const NexusSystem& rhs);

		void KeyboardEvent(
			UINT nChar, 
			UINT nRepCnt, 
			UINT nFlags, 
			bool pressed);

		Tu::Status OnResetDevice();

		void CleanUpAfterFailure();

		bool				m_isCreated;
		bool				m_sceneLoaded;
		HWND				m_renderWindowHandle;
		
		bool				m_keyboardState[256];
		bool				m_mouseButtonState[E_MouseButton_Count];
		E_SelectionState	m_selectionState;
		E_ViewportState		m_viewportState;

		Core				m_nexusCore;
		
		NexusProgressDialog* m_pProgressDialog;

    	//Pointer to a function that provides a logging facility.
		void (*m_pLogCallbackFunc)(const std::string& text);

		bool				m_animationPaused;
		//Frequency of the PC's performance counter.
		LARGE_INTEGER		m_timerFrequency;
		LARGE_INTEGER		m_startTime;

		C_ApplicationState	m_state;
		
		U32					m_activeViewportIndex;
	};
}

#endif
