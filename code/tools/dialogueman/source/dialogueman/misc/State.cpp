#include "stdafx.h"

#include <dialogueman/Misc/DomId.h>
#include <dialogueman/Misc/Selection.h>
#include <dialogueman/Misc/State.h>
#include <dialogueman/App.h>

namespace DMan
{

C_NonXtState::C_NonXtState()
{
	pSelection = new C_Selection;
}

C_NonXtState::C_NonXtState(const C_NonXtState& rhs)
{
	pSelection = new C_Selection;

	*this = rhs;
}

C_NonXtState::~C_NonXtState()
{
	delete pSelection;
}

const C_NonXtState& C_NonXtState::operator=(const C_NonXtState& rhs)
{
	if (&rhs == this)
		return *this;

	*pSelection = *rhs.pSelection;

	return *this;
}

bool C_NonXtState::operator==(const C_NonXtState& rhs) const
{
	assert(pSelection && rhs.pSelection);
	return *pSelection == *rhs.pSelection;
}

bool C_NonXtState::operator!=(const C_NonXtState& rhs) const
{
	return !(*this==rhs);
}

void C_NonXtState::Clear()
{
	pSelection->Clear();
}

}
