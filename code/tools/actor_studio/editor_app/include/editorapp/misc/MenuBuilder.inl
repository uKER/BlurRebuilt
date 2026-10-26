
namespace Editor
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
		wxEventConnection c0 = wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(pEventHandler, fn, false, result);
		m_MenuConnections.PushBack(c0);

		if (updateFn)
		{
			wxEventConnection c1 = wxConnect<wxEVENT_UPDATE_UI>(pEventHandler, updateFn, result);
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
		wxEventConnection c0 = wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(pEventHandler, fn, obj, false, result);
		m_MenuConnections.PushBack(c0);

		if (updateFn)
		{
			Ut::Delegate<void(wxUpdateUIEvent&)> d1(obj, updateFn);
			wxEventConnection c1 = wxConnect<wxEVENT_UPDATE_UI>(pEventHandler, updateFn, obj, result);
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
		wxEventConnection c0 = wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(pEventHandler, fn, result);
		m_MenuConnections.PushBack(c0);

		if (updateFn)
		{
			wxEventConnection c1 = wxConnect<wxEVENT_UPDATE_UI>(pEventHandler, updateFn, result);
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
		wxEventConnection c0 = wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(pEventHandler, fn, obj, result);
		m_MenuConnections.PushBack(c0);

		if (updateFn)
		{
			Ut::Delegate<void(wxUpdateUIEvent&)> d1(obj, updateFn);
			wxEventConnection c1 = wxConnect<wxEVENT_UPDATE_UI>(pEventHandler, updateFn, obj, result);
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
			if (m_MenuConnections[i].m_firstId == result)
			{
				m_MenuConnections[i].Disconnect();
				m_MenuConnections.EraseNonOrdered(i);
			}	
		}
	}
	return (result != -1);
}

}
