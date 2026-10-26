#ifndef EDITOR_LIB_XT_TYPE_DATABASE_HELPERS_H
#define EDITOR_LIB_XT_TYPE_DATABASE_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtActorType;

	namespace XtTypeDatabaseHelpers
	{
		///Build a list of actor types.
		void BuildActorTypeList(Btl::Vector<const C_XtActorType*>& actorTypes);
	}
}

#endif
