#include "stdafx.h"
#include <EditorLib/Dg/IDgObject.h>

namespace EditorLib
{

I_DgObject::I_DgObject() : Sys::RefObj()
{
}

I_DgObject::~I_DgObject()
{
}

bool I_DgObject::IsA(E_ObjType type) const
{
	if (type == E_ObjType_IDgObject)
		return true;
	return false;
}

}
