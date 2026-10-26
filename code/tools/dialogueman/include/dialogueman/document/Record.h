#ifndef DMAN_RECORD_H
#define DMAN_RECORD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_Record : public XtShared::C_XtObject
	{
	public:
		C_Record(XtId xtRecordId);
		virtual ~C_Record();

		XtId			GetRecordId();

		C_RowSet*		GetRowSet(S32 index);

		U32				GetNumRowSets() const;

		XtString		GetName() const;

		virtual void	XtBind() const;

		virtual void	XtUnbind() const;

		bool			XtInitialise();

		virtual void	XtDeserialise(
			Xt::DomElement* pFrom = 0);

	private:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent);

		XtId			m_xtRecordId;
		
		C_ColumnSet*	m_pAllColumnSet;
		Btl::Vector<C_RowSet*>
						m_rowSets;
	};

}

#endif //DMAN_RECORD_H
