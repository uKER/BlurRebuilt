
namespace Tools
{

template <typename R>
S32 C_MenuBuilderBase::AddMenuEntry(
	const char* pMenuEntry, 
	wxEvtHandler* pEventHandler, 
	R (*fn)(),
	void (*updateFn)(wxUpdateUIEvent&),
	E_MenuItemType type)
{
	S32 result = OnAddMenuEntry(pMenuEntry, type);
	if (result != -1)
	{
		Ut::Delegate<R()> d0(fn);
		EventConnection c0 = ConnectEvent0<wxCommandEvent, Ut::Delegate<R()>,ConsumeEvent>(pEventHandler, wxEVT_COMMAND_MENU_SELECTED, d0, result);
		m_MenuConnections.PushBack(c0);

		if (updateFn)
		{
			Ut::Delegate<void(wxUpdateUIEvent&)> d1(updateFn);
			EventConnection c1 = ConnectEvent1<wxUpdateUIEvent, Ut::Delegate<void(wxUpdateUIEvent&)>>(pEventHandler, wxEVT_UPDATE_UI, d1, result);
			m_MenuConnections.PushBack(c1);
		}

	}
	return (result != -1);
}

template <typename R, typename T>
S32 C_MenuBuilderBase::AddMenuEntry(
	const char* pMenuEntry, 
	wxEvtHandler* pEventHandler,
	T* obj, R (T::*fn)(), 
	void (T::*updateFn)(wxUpdateUIEvent&),
	E_MenuItemType type)
{
	S32 result = OnAddMenuEntry(pMenuEntry, type);
	if (result != -1)
	{
		Ut::Delegate<R()> d0(obj, fn);
		EventConnection c0 = ConnectEvent0<wxCommandEvent, Ut::Delegate<R()>,ConsumeEvent>(pEventHandler, wxEVT_COMMAND_MENU_SELECTED, d0, result);
		m_MenuConnections.PushBack(c0);

		if (updateFn)
		{
			Ut::Delegate<void(wxUpdateUIEvent&)> d1(obj, updateFn);
			EventConnection c1 = ConnectEvent1<wxUpdateUIEvent, Ut::Delegate<void(wxUpdateUIEvent&)>>(pEventHandler, wxEVT_UPDATE_UI, d1, result);
			m_MenuConnections.PushBack(c1);
		}
	}
	return result;
}

template <typename R, typename EventT>
S32 C_MenuBuilderBase::AddMenuEntry(
	const char* pMenuEntry, 
	wxEvtHandler* pEventHandler,
	R (*fn)(EventT), 
	void (*updateFn)(wxUpdateUIEvent&),
	E_MenuItemType type)
{
	S32 result = OnAddMenuEntry(pMenuEntry, type);
	if (result != -1)
	{
		Ut::Delegate<R(EventT)> d0(fn);
		EventConnection c0 = ConnectEvent1<wxCommandEvent, Ut::Delegate<R(EventT)>>(pEventHandler, wxEVT_COMMAND_MENU_SELECTED, d0, result);
		m_MenuConnections.PushBack(c0);

		if (updateFn)
		{
			Ut::Delegate<void(wxUpdateUIEvent&)> d1(updateFn);
			EventConnection c1 = ConnectEvent1<wxUpdateUIEvent, Ut::Delegate<void(wxUpdateUIEvent&)>>(pEventHandler, wxEVT_UPDATE_UI, d1, result);
			m_MenuConnections.PushBack(c1);
		}
	}
	return result;
}

template <typename R, typename T, typename EventT>
S32 C_MenuBuilderBase::AddMenuEntry(
	const char* pMenuEntry, 
	wxEvtHandler* pEventHandler,
	T* obj, 
	R (T::*fn)(EventT),
	void (T::*updateFn)(wxUpdateUIEvent&),
	E_MenuItemType type)
{
	S32 result = OnAddMenuEntry(pMenuEntry, type);
	if (result != -1)
	{
		Ut::Delegate<R(EventT)> d0(obj, fn);
		EventConnection c0 = ConnectEvent1<wxCommandEvent, Ut::Delegate<R(EventT)>>(pEventHandler, wxEVT_COMMAND_MENU_SELECTED, d0, result);
		m_MenuConnections.PushBack(c0);

		if (updateFn)
		{
			Ut::Delegate<void(wxUpdateUIEvent&)> d1(obj, updateFn);
			EventConnection c1 = ConnectEvent1<wxUpdateUIEvent, Ut::Delegate<void(wxUpdateUIEvent&)>>(pEventHandler, wxEVT_UPDATE_UI, d1, result);
			m_MenuConnections.PushBack(c1);
		}
	}
	return result;
}

inline S32 C_MenuBuilderBase::AddMenuEntry(const char* pMenuName, E_MenuItemType type)
{
	return OnAddMenuEntry(pMenuName, type);
}

inline bool C_MenuBuilderBase::DeleteMenuEntry(const char *pMenuEntry)
{
	S32 result = OnDeleteMenuEntry(pMenuEntry);
	if (result != -1)
	{
		for (S32 i=0; i<m_MenuConnections.Size(); ++i)
		{
			if (m_MenuConnections[i].m_WinID == result)
			{
				m_MenuConnections[i].Disconnect();
				m_MenuConnections.EraseNonOrdered(i);
			}	
		}
	}
	return (result != -1);
}

}
