#ifndef DMAN_CHANGE_ROWS_H
#define DMAN_CHANGE_ROWS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace ChangeRows
{
	//bool Do();
	bool Change(LarvaType* pRowType, Xt::DomArray* pDialogueArray, Btl::Map<wxString, C_MutantValue>& row, size_t index);
}
}

#endif // DMAN_CHANGE_ROWS_H
