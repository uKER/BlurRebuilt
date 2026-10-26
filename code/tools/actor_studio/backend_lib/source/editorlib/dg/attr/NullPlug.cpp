#include <stdafx.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>

namespace EditorLib
{
 
C_NullPlug::C_NullPlug(
	I_DgNode* pParent,
	E_PlugType type,
	const char* name) : 
	C_PlugBase<C_NamedPlugPolicy, C_ChildPolicy, C_NonConnectablePolicy>(pParent, 0, type, name)
{
}

C_NullPlug::C_NullPlug(
	I_Plug* pParent,
	E_PlugType type,
	const char* name) : 
	C_PlugBase<C_NamedPlugPolicy, C_ChildPolicy, C_NonConnectablePolicy>(pParent, 0, type, name)
{
}


C_NullPlug::~C_NullPlug()
{
}

}
