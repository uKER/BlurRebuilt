#include "stdafx.h"

#include <EditorLib/Misc/Status.h>

#include <EditorApp/editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/editor/manipulators/Manipulator.h>
#include <EditorApp/editor/manipulators/ManipulatorContextMapping.h>

namespace Editor
{

C_ManipulatorContextMapping::C_ManipulatorContextMapping()
{
	//Create clear table.
	m_dispatchTable.Resize(EditorLib::E_ObjType_Count);
	for (S32 i = 0; i < m_dispatchTable.Size(); i++)
	{
		m_dispatchTable[i].Resize(E_ManipulatorType_Count);
		//Set all cells to 'unknown' type.
		memset(&m_dispatchTable[i][0], 
			E_ManipulatorContextType_Unknown, 
			sizeof(E_ManipulatorContextType)*E_ManipulatorType_Count);
	}
}

C_ManipulatorContextMapping::~C_ManipulatorContextMapping()
{
}

E_ManipulatorContextType C_ManipulatorContextMapping::FindContext(
	EditorLib::E_ObjType input1, E_ManipulatorType input2) const
{
	return m_dispatchTable[input1][input2];
}

EditorLib::C_Status C_ManipulatorContextMapping::Register(
	EditorLib::E_ObjType input1, 
	E_ManipulatorType input2, 
	E_ManipulatorContextType output)
{
	if (m_dispatchTable[input1][input2] != E_ManipulatorContextType_Unknown)
	{
		assert(!"Duplicate context in dispatch table!");
		return EditorLib::E_Status_Failure;
	}

	m_dispatchTable[input1][input2] = output;

	return EditorLib::E_Status_Success;
}

}


