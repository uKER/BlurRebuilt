#ifndef XT_SHARED_XT_OBJECT_IMPL_H
#define XT_SHARED_XT_OBJECT_IMPL_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XtShared
{
	class C_XtObjectImpl
	{
	public:
		C_XtObjectImpl();
		~C_XtObjectImpl();

		void XtInitialise(
			C_XtType xtType, 
			bool isPolymorphic,
			Xt::DomElement* pData);

		void XtDeserialiseDeferred();

		void XtBind() const;

		void XtUnbind() const;

		bool IsBound() const;

		Xt::DomElement* GetXtData();

		C_XtType GetXtType() const;

		void DeleteXtData();

		Xt::DomMonitor* GetXtMonitor(C_XtObject::E_Monitor monitor);

		void Attach(C_XtObject* pXtObject);

		void Detach();

		bool TypeDirty() const;

	protected:
		static void StaticOnXtDomInternalMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent, 
			C_XtObjectImpl* pThis);

		C_XtType			m_xtType;
		C_XtType			m_initialType;
		Xt::DomMonitor		m_monitors[C_XtObject::E_Monitor_Count];
		struct
		{
			mutable S32		m_unbinder : 7;
			bool			m_typeMonitorDirty : 1;
			bool			m_isPolymorphic : 1;
		};
		C_XtObject*			m_pXtObject;
	};

	extern void OnChangeXtObjectType(C_XtObjectImpl* pImpl);
	extern void AddXtObjectForDeferredUpdate(C_XtObjectImpl* pImpl);
	extern void RemoveXtObjectFromDeferredUpdate(C_XtObjectImpl* pImpl);
}

#endif
