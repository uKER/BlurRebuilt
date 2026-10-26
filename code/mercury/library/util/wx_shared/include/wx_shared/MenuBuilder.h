#ifndef WX_SHARED_MENU_BUILDER_H
#define WX_SHARED_MENU_BUILDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Tools
{

enum E_MenuItemType
{
	E_MenuItem_Separator = -1,
	E_MenuItem_Normal,
	E_MenuItem_Check,
	E_MenuItem_Radio
};

class C_MenuBuilderBase
{
public:
	template <typename R>
	S32 AddMenuEntry(
		const char* pMenuEntry, 
		wxEvtHandler* pEventHandler,
		R (*fn)(), 
		void (*updateFn)(wxUpdateUIEvent&)=0,
		E_MenuItemType type=E_MenuItem_Normal);

	template <typename R, typename T>
	S32 AddMenuEntry(
		const char* pMenuEntry, 
		wxEvtHandler* pEventHandler,
		T* obj, 
		R (T::*fn)(),
		void (T::*updateFn)(wxUpdateUIEvent&)=0,
		E_MenuItemType type=E_MenuItem_Normal);

	template <typename R, typename EventT>
	S32 AddMenuEntry(
		const char* pMenuEntry, 
		wxEvtHandler* pEventHandler,
		R (*fn)(EventT),
		void (*updateFn)(wxUpdateUIEvent&)=0,
		E_MenuItemType type=E_MenuItem_Normal);

	template <typename R, typename T, typename EventT>
	S32 AddMenuEntry(
		const char* pMenuEntry, 
		wxEvtHandler* pEventHandler,
		T* obj, 
		R (T::*fn)(EventT),
		void (T::*updateFn)(wxUpdateUIEvent&)=0,
		E_MenuItemType type=E_MenuItem_Normal);

	virtual S32 AddMenuEntry(const char* pMenuName, E_MenuItemType type);

protected:

	bool DeleteMenuEntry(const char* pMenuEntry);
	virtual S32 OnAddMenuEntry(const char* pMenuEntry, E_MenuItemType type) { return -1; }
	virtual S32 OnDeleteMenuEntry(const char* pMenuEntry) { return -1; }

	Ut::Vector<EventConnection>	m_MenuConnections;
};



class C_MenuBuilder : public C_MenuBuilderBase
{
public:
	C_MenuBuilder(wxMenuBar* pMenuBar);
	~C_MenuBuilder();

protected:
	S32 OnAddMenuEntry(const char* pMenuEntry, E_MenuItemType type);
	S32 OnDeleteMenuEntry(const char* pMenuEntry);

	wxMenuBar* m_pMenuBar;	
};

}

#include "MenuBuilder.inl"

#endif
