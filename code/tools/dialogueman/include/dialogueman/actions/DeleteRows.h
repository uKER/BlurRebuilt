#ifndef DMAN_DELETE_ROWS_H
#define DMAN_DELETE_ROWS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace DeleteRows
{
	bool Do();
	void DecreaseCompIndices(Btl::Vector<U32>& components);
	bool Delete(Xt::DomArray* pDialogueArray, size_t rowIndex);
}
}

#endif // DMAN_DELETE_ROWS_H
