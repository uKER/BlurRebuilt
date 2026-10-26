#ifndef EDITOR_LIB_XT_OBJECT_H
#define EDITOR_LIB_XT_OBJECT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Misc/Status.h>
#include "XtDirty.h"

namespace EditorLib
{
	///Mixin class for dg nodes that are backed onto XT
	class C_ViewDisabler;
	class C_XtDirty;

	class C_XtObject
	{
	public:
		class C_Unbinder
		{
		public:
			C_Unbinder(C_XtObject* pObj);
			~C_Unbinder();

		private:
			C_XtObject*		m_pObj;
			C_ViewDisabler* m_pViewDisabler; 
		};

		C_XtObject();
		virtual ~C_XtObject();

		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

		virtual void XtBind() const;

		virtual void XtUnbind() const;

		bool IsBound() const;

		Xt::DomElement* GetXtData();

		const Xt::DomElement* GetXtData() const;

		void SetXtData(Xt::DomElement* pData);

		LarvaType* GetXtType();

		const LarvaType* GetXtType() const;

		C_XtDirty& GetXtDirty();

		const C_XtDirty& GetXtDirty() const;

		const char* GetXtTypeName() const;

	protected:
		//For use by monitors inside of C_XtObject.
		static void StaticOnXtDomInternalMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent, 
			C_XtObject* pThis);

		//For use by monitors outside of this C_XtObject.
		static void StaticOnXtDomExternalMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent, 
			C_XtObject* pThis);

		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged) {}

	private:
		LarvaType*		m_pLarvaType;

		Xt::DomMonitor	m_rootMonitor;
		struct
		{
			mutable S32		m_unbinder : 7;
		};
		C_XtDirty		m_xtDirty;
		XtString		m_componentType;
	};
}

#endif
