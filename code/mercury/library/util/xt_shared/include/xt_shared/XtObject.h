#ifndef XT_SHARED_XT_OBJECT_H
#define XT_SHARED_XT_OBJECT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XtShared
{
	///Mixin class for objects that are backed onto Xt data.
	class C_ViewDisabler;
	class C_XtObjectImpl;

	class C_XtType
	{
	public:
		C_XtType() { m_xtType.pType = 0; }
		C_XtType(LarvaType* pType) { m_xtType.pType = pType; }
		C_XtType(LarvaField* pField) { m_xtType.pField = pField; }

		LarvaType* AsType() { return m_xtType.pType; }
		LarvaField* AsField() { return m_xtType.pField; }
		const LarvaType* AsType() const { return m_xtType.pType; } 
		const LarvaField* AsField() const { return m_xtType.pField; } 

	private:
		union U_XtType
		{
			LarvaType*		pType;
			LarvaField*		pField;
		};

		U_XtType m_xtType;
	};



	class C_XtObject
	{
	public:
		enum E_Monitor
		{
			E_Monitor_Root,
			E_Monitor_Type,

			E_Monitor_Count
		};

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

		virtual bool XtInitialise(
			C_XtType xtType, 
			bool isPolymorphic,
			Xt::DomElement* pData);

		virtual void XtBind() const;

		virtual void XtUnbind() const;

		bool IsBound() const;

		Xt::DomElement* GetXtData();

		C_XtType GetXtType() const;

		virtual void XtDeserialise(Xt::DomElement* pFrom = 0);

		virtual void XtDeserialiseDeferred();

		Xt::DomMonitor* GetXtMonitor(E_Monitor monitor);

		C_XtObjectImpl* GetXtObjectImpl();

		void AttachXtObjectImpl(C_XtObjectImpl* pImpl);
		void DetachXtObjectImpl();

	protected:
		static void StaticOnXtDomExternalMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent, 
			C_XtObject* pThis);

		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent);

		virtual void DeleteXtData();

	private:
		C_XtObjectImpl* m_pXtObjectImpl;
		FRIEND_CLASS	C_XtObjectImpl;
	};
}

#endif
