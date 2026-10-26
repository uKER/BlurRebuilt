#include "stdafx.h"

#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>

#include <EditorLib/Core.h>

namespace EditorLib
{
namespace XtTypeDatabaseHelpers
{

void BuildActorTypeList(Btl::Vector<const C_XtActorType*>& actorTypes)
{
	Btl::Vector<const I_XtType*> types;
	C_Core::Instance().GetXtTypeDatabase().BuildTypeList(types);

	for (S32 i = 0; i < types.Size(); i++)
	{
		const I_XtType* pType = types[i];
		if (pType->IsA(E_XtType_SceneXtActorNode))
			actorTypes.PushBack((const C_XtActorType*)pType);
	}
}

}
}