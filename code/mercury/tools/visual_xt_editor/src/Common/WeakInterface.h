#ifndef WEAK_INTERFACE_H
#define WEAK_INTERFACE_H

#pragma once

//
// WeakContext
//

class I_WeakInterface
{
public:
	virtual bool AddItem(LtStruct* type, Xt::DomBlock* block, LarvaField* labelField, Xt::DomBlock* labelWatch) { return false; }

	virtual void AddWeakPtr(LfWeakPtr* pWeakPtr, Xt::DomField* field) {};

	XtString	m_StructId;
};


#endif // WEAK_INTERFACE_H
