#ifndef DMAN_ROW_SET_H
#define DMAN_ROW_SET_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

//TODO: Remove array of rows and just use the hash table?

class C_RowSet : public XtShared::C_XtObject
{
public:
	typedef Btl::Vector< C_Row* > RowArray;

	C_RowSet(LarvaField* pArrayField);
	virtual ~C_RowSet();

	//Return a pointer set capable of handling all of the fields for all possible item types.
	C_ColumnSet*	GetAllColumnSet();

	S32				GetNumRows();

	//Return a pointer to the start of a row of data.
	C_Row*			GetRow(U32 rowIndex);
	const C_Row*	GetRow(U32 rowIndex) const;

	RowArray&		GetRows();

	const RowArray&	GetRows() const;

	virtual void	XtBind() const;

	virtual void	XtUnbind() const;

	virtual bool	XtInitialise(
		XtShared::C_XtType xtType, 
		bool isPolymorphic,
		Xt::DomElement* pData);

	virtual void	XtDeserialise(
		Xt::DomElement* pFrom = 0);

private:
	typedef Btl::OpenHashMap<XtId, C_Row*> XtIdHashTable;
	typedef Btl::OpenHashSet<XtId> XtIdHashSet;

	void OnRowRemoved(Xt::DomElement* pFrom);

	bool OnPotentialRowReorder(Xt::DomElement* pFrom, S32 index);

	bool OnPotentialRowAdd(Xt::DomElement* pFrom, S32 index);

	virtual void OnXtDomMonitorEvent(
		Xt::DomMonitor* pMonitor, 
		Xt::DomEvent* pEvent);

	bool AddNewRow(
		S32 xtArrayIndex, 
		Xt::DomElement* pData);

	LarvaField*		m_pArrayField;
	C_ColumnSet*	m_pAllColumnSet;
	RowArray		m_rows;
	XtIdHashTable	m_xtIdHashTable;
};

}

#endif //DMAN_ROW_SET_H