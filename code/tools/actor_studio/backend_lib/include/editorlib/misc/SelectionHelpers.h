#ifndef EDITOR_LIB_SELECTION_HELPERS_H
#define EDITOR_LIB_SELECTION_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	namespace SelectionHelpers
	{
		void FilterSelection(C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType);

		void FilterSelectionIsA(C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType);

		void FilterPropertySelection(C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType);

		void FilterPropertySelectionIsA(C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType);
	}
}

#endif
