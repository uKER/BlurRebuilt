#ifndef DMAN_STATE_H
#define DMAN_STATE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	
//Store all undoable non xt state info here.
struct C_NonXtState
{
	C_NonXtState();
	C_NonXtState(const C_NonXtState& rhs);
	~C_NonXtState();

	const C_NonXtState& operator=(const C_NonXtState& rhs);

	bool operator==(const C_NonXtState& rhs) const;

	bool operator!=(const C_NonXtState& rhs) const;

	void Clear();

	C_Selection*	pSelection;
};

}

#endif //DMAN_STATE_H
