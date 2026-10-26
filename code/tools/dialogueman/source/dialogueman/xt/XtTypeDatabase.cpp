#include "stdafx.h"

#include <DialogueMan/Xt/XtTypeDatabase.h>
#include <DialogueMan/Xt/Types/IXtType.h>
#include <DialogueMan/Xt/Types/XtSxlRecord.h>
#include <DialogueMan/Xt/Types/XtSxlRow.h>

namespace DMan
{

const char* C_XtTypeDatabase::m_pDgXtTypeStringTable[] =
{
#define DEF(e, str) str,
	#include <DialogueMan/Xt/Types/XtTypeMapping.h>
#undef DEF
};

C_XtTypeDatabase::C_XtTypeDatabase()
{
}

C_XtTypeDatabase::~C_XtTypeDatabase()
{
	UnregisterTypes();
}

void C_XtTypeDatabase::RegisterTypes(Xt::DomDb* pDb)
{
	assert(pDb);
	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&pDb->schema->types); it; ++it)
	{
		LarvaType* pType = (LarvaType*)it;
		RegisterType(pType);
	}
}

void C_XtTypeDatabase::UnregisterTypes()
{
	//Free our types.
	for (XtTypeHashMap::Iterator it = m_mapping.Begin(); it != m_mapping.End(); ++it)
		delete it->GetValue();

	m_mapping.Clear();
}

E_XtType C_XtTypeDatabase::GetType(const char* type)
{
	if (strlen(type) == 0)
		return E_XtType_Unknown;

	Util::C_String searchString;
	for (U32 i = 0; i < E_XtType_Count; i++)
	{
		if (!strcmp(m_pDgXtTypeStringTable[i], type))
			return E_XtType(i);
	}

	return E_XtType_Unknown;
}

E_XtType C_XtTypeDatabase::GetType(const LarvaType* _pType)
{
	LarvaType* pType = const_cast<LarvaType*>(_pType);

	E_XtType eThisType = GetType(pType->attrs.GetString(kXtSemanticAttr, kXtTypeParam));
	if (eThisType != E_XtType_Unknown)
		return eThisType;

	if (pType->kind == kLtStruct) {
		//Examine base class.
		LarvaBaseType* pBase = ((LtStruct*)pType)->base;
		
		if(pBase)
		{
			E_XtType eParentType;
			LtStruct* pBaseType = pBase->type;
			eParentType = GetType(pBaseType->attrs.GetString(kXtSemanticAttr, kXtTypeParam));
			if (eParentType != E_XtType_Unknown)
				return eParentType;

			//Recurse.
			eParentType = GetType(pBaseType);
			if (eParentType != E_XtType_Unknown)
				return eParentType;
		}
		
	}

	return E_XtType_Unknown;
}

LarvaType* C_XtTypeDatabase::GetType(E_XtType type)
{
	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&wxGetApp().GetDb()->schema->types); it; ++it)
	{
		LarvaType* pType = (LarvaType*)it;
		if(type != E_XtType_Unknown && type == GetType(pType))
			return pType;
	}
	return NULL;
}

const char* C_XtTypeDatabase::GetTypeName(E_XtType type)
{
	return m_pDgXtTypeStringTable[type];
}

void C_XtTypeDatabase::BuildTypeList(Btl::Vector<const I_XtType*>& types) const
{
	types.Reserve(m_mapping.Size());
	for (XtTypeHashMap::ConstIterator it = m_mapping.Begin(); it != m_mapping.End(); ++it)
		types.PushBack(it->GetValue());
}

I_XtType* C_XtTypeDatabase::FindType(const Btl::String& typeName)
{
	XtTypeHashMap::Iterator it = m_mapping.Find(typeName);
	if (it != m_mapping.End())
		return it->GetValue();

	return 0;
}

void C_XtTypeDatabase::RegisterType(LarvaType* pSchemaType)
{
	if (pSchemaType->kind != kLtStruct)
		return;

	Btl::String name = pSchemaType->name;

	XtTypeHashMap::Iterator it = m_mapping.Find(name);
	if (it != m_mapping.End())
	{
		//Duplicate command in registry!
		return;
	}

	//Filter out anything that's not of a known type.
	E_XtType xtType = GetType(pSchemaType);

	I_XtType* pXtType;
	switch(xtType)
	{
	case E_XtType_SxlRecord:
		pXtType = new C_XtDialogueType((LtStruct*)pSchemaType);
		break;
	case E_XtType_SxlRow:
		pXtType = new C_XtDialogueItemType((LtStruct*)pSchemaType);
		break;
	default:
		return;
	};

	//Add to registry.
	m_mapping[name] = pXtType;
	return;
}

}
