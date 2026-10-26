#ifndef EDITOR_MANIPULATOR_HELPERS_H
#define EDITOR_MANIPULATOR_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	namespace ManipulatorHelpers
	{
		bool SelectedNodesAreA(
			EditorLib::E_ObjType isAType,
			const EditorLib::C_Selection& selection);

		EditorLib::E_ObjType SelectedNodesGetType(
			const EditorLib::C_Selection& selection);

		EditorLib::E_ObjType DetermineSelectionType();
	}
}

#endif
