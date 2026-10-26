#ifndef DMAN_XT_TYPE_DATABASE_H
#define DMAN_XT_TYPE_DATABASE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_XtTypeDatabase
	{
	public:
		C_XtTypeDatabase();

		~C_XtTypeDatabase();

		void RegisterTypes(Xt::DomDb* pDb);

		void UnregisterTypes();

		I_XtType* FindType(const Btl::String& typeName);

		static E_XtType GetType(const char* type);

		static E_XtType GetType(const LarvaType* pType);

		static LarvaType* GetType(E_XtType type);

		static const char* GetTypeName(E_XtType type);

		void BuildTypeList(Btl::Vector<const I_XtType*>& types) const;

	private:
		void RegisterType(LarvaType* pSchemaType);

		typedef Btl::OpenHashMap<Btl::String, I_XtType*, Btl::HashKeyOpBtlString > XtTypeHashMap;

		static const char* m_pDgXtTypeStringTable[E_XtType_Count];

		XtTypeHashMap m_mapping;
	};
}

#endif //DMAN_XT_TYPE_DATABASE_H
