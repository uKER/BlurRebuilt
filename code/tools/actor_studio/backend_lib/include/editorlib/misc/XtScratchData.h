#ifndef EDITOR_LIB_XT_SCRATCH_DATA
#define EDITOR_LIB_XT_SCRATCH_DATA

namespace EditorLib
{
	class C_XtScratchData
	{
	public:
		typedef void (*Callback)(Xt::DomMonitor*, Xt::DomEvent*, void*);

		C_XtScratchData();
		~C_XtScratchData();

		void SetCallback(Callback callback, void* userData);
		Xt::DomComponent* GetXtData();

	private:
		void OnXtDomMonitorEvent(Xt::DomMonitor*, Xt::DomEvent*);

		static void StaticOnXtDomMonitorEvent(Xt::DomMonitor*, Xt::DomEvent*, C_XtScratchData*);
		
		Xt::DomComponent*	m_ScratchData;
		Xt::DomMonitor		m_DomMonitor;
		Callback			m_Callback;
		void*				m_UserData;
	};
}

#endif