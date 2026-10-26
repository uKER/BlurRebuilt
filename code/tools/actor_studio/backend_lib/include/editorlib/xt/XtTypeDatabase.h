#ifndef EDITOR_LIB_DG_XT_TYPE_DATABASE_H
#define EDITOR_LIB_DG_XT_TYPE_DATABASE_H

#pragma once

#include <Btl/Containers/Vector.h>
#include <Btl/Containers/OpenHashMap.h>
#include <Btl/Utility/String.h>
#include <Btl/Utility/Hashops.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Xt/XtDefinitions.h>

namespace EditorLib
{
	class C_XtTypeDatabase
	{
	public:
		C_XtTypeDatabase();

		~C_XtTypeDatabase();

		C_Status RegisterTypes();

		C_Status UnregisterTypes();

		//This should be made const, but for XT :(
		I_XtType* FindType(const Btl::String& typeName);

		bool IsA(const Btl::String& typeName, E_XtType type);

		static E_XtType GetType(const char* type);

		static E_XtType GetType(const LarvaType* pType);

		static const char* GetTypeName(E_XtType type);

		void BuildTypeList(Btl::Vector<const I_XtType*>& types) const;

	private:
		void RegisterType(LarvaType* pSchemaType);

		typedef Btl::OpenHashMap<Btl::String, I_XtType*, Btl::HashKeyOpBtlString > XtTypeHashMap;

		static const char* m_pDgXtTypeStringTable[E_XtType_Count];

		XtTypeHashMap m_mapping;
	};
}

#endif
