#include "stdafx.h"

#include <EditorLib/Context/DeleteContextMapping.h>

namespace EditorLib
{

C_DeleteContextMapping::C_DeleteContextMapping()
{
	//Create clear table.
	m_dispatchTable.Resize(EditorLib::E_ObjType_Count);
	for (S32 i = 0; i < m_dispatchTable.Size(); i++)
	{
		m_dispatchTable[i] = E_DeleteContextType_Unknown;
	}
}

C_DeleteContextMapping::~C_DeleteContextMapping()
{
}

E_DeleteContextType C_DeleteContextMapping::FindContext(
	EditorLib::E_ObjType input1) const
{
	return m_dispatchTable[input1];
}

EditorLib::C_Status C_DeleteContextMapping::Register(
	EditorLib::E_ObjType input1, 
	E_DeleteContextType output)
{
	int inputType = input1;
	if (m_dispatchTable[input1] != E_DeleteContextType_Unknown)
	{
		assert(!"Duplicate context in dispatch table!");
		return EditorLib::E_Status_Failure;
	}

	m_dispatchTable[input1] = output;

	return EditorLib::E_Status_Success;
}

}


