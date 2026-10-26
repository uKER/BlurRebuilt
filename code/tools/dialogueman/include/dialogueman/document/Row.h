#ifndef DMAN_ROW_H
#define DMAN_ROW_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_Row : public XtShared::C_XtObject
	{
	public:
		typedef Btl::Map<wxString, C_MutantValue> ColumnMap;

		C_Row(C_RowSet* pRowSet, XtId xtId, S32 xtArrayIndex);
		virtual ~C_Row();

		XtId GetXtId() const;

		void SetXtArrayIndex(S32 index);

		S32 GetXtArrayIndex() const;

		const ColumnMap& GetColumns() const;

		ColumnMap& GetColumns();

		virtual bool XtInitialise(
			XtShared::C_XtType xtType, 
			bool isPolymorphic,
			Xt::DomElement* pData);

		void SetDirty(bool dirty = true);

		virtual void XtDeserialise(Xt::DomElement* pFrom = 0);

		virtual void XtDeserialiseDeferred();

	private:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent);

		struct C_Flags
		{
			bool dirty		 : 1;
		};

		C_RowSet*		m_pRowSet;
		S32				m_xtArrayIndex;
		ColumnMap		m_columns;
		XtId			m_xtId;
		C_Flags			m_flags;
	};
}

#endif //DMAN_ROW_H
