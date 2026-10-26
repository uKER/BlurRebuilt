#ifndef DMAN_DB_MANAGER_H
#define DMAN_DB_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_DbManager
	{
	public:
		//Look up columns by dialogue item type.
		typedef Btl::OpenHashMap<LarvaType*, C_ColumnSet*> ColumnSetHashTable;

		C_DbManager();
		~C_DbManager();

		C_ColumnSet*	GetTypeColumnSet(LarvaType* pDialogueItemType);

		const ColumnSetHashTable& GetTypeColumnSets();

		C_Record*		GetRecord(XtId recordId);

		void			OnDbOpened();
		void			OnDbClose();

		C_Record*		AcquireRecord(LarvaType* pDialogueType, XtId recordId);
		bool			RemoveRecord(XtId recordId);			

	private:
		//Look up rows by record id.
		typedef Btl::OpenHashMap<XtId, C_Record*> RecordRowsHashTable;

		void			Clear();

		void			CreateTypeColumnSets();
		void			CreateTypeColumnSet(LarvaType* pDialogueItemType, C_ColumnSet* pColumnSet = 0);
		

		void			XtDeserialise();

		ColumnSetHashTable		m_typeColumns;
		RecordRowsHashTable		m_records;
	};
}

#endif //DMAN_DB_MANAGER_H
