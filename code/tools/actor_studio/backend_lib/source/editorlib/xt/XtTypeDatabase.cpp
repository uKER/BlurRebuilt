#include "stdafx.h"

#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/Types/XtActorType.h>
#include <EditorLib/Xt/Types/XtActorComponentType.h>
#include <EditorLib/Xt/Types/XtGameDataType.h>
#include <EditorLib/Xt/Types/XtSceneActorType.h>
#include <EditorLib/Xt/Types/XtSceneNodeType.h>
#include <EditorLib/Xt/Types/XtActorPropType.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

const char* C_XtTypeDatabase::m_pDgXtTypeStringTable[] =
{
#define DEF(e, str) str,
	#include <EditorLib/Xt/Types/XtTypeMapping.h>
#undef DEF
};

C_XtTypeDatabase::C_XtTypeDatabase()
{
}

C_XtTypeDatabase::~C_XtTypeDatabase()
{
	UnregisterTypes();
}

C_Status C_XtTypeDatabase::RegisterTypes()
{
	Xt::DomDb* pDb = C_Core::Instance().GetXtManager().GetDb();

	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&pDb->schema->types); it; ++it)
	{
		LarvaType* pType = (LarvaType*)it;
		RegisterType(pType);
	}

	return E_Status_Success;
}

C_Status C_XtTypeDatabase::UnregisterTypes()
{
	//Free our types.
	for (XtTypeHashMap::Iterator it = m_mapping.Begin(); it != m_mapping.End(); ++it)
		delete it->GetValue();

	m_mapping.Clear();
	return E_Status_Success;
}

E_XtType C_XtTypeDatabase::GetType(const char* type)
{
	if (strlen(type) == 0)
		return E_XtType_Unknown;

	Btl::String searchString;
	for (U32 i = 0; i < E_XtType_Count; i++)
	{
		const char* str = m_pDgXtTypeStringTable[i];
		searchString = "Type:";
		searchString = searchString + str;

		if (searchString == type)
			return E_XtType(i);
	}

	return E_XtType_Unknown;
}

E_XtType C_XtTypeDatabase::GetType(const LarvaType* _pType)
{
	LarvaType* pType = const_cast<LarvaType*>(_pType);

	E_XtType eThisType = GetType(pType->attrs.GetString("SceneView", "semantic"));
	if (eThisType != E_XtType_Unknown)
		return eThisType;

	if (pType->kind == kLtStruct) {
		//Examine base classes.
		if (LarvaBaseType* base = ((LtStruct*)pType)->base)
		{
			E_XtType eParentType;

			LtStruct* pBaseType = base->type;
			eParentType = GetType(pBaseType->attrs.GetString("SceneView", "semantic"));
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

bool C_XtTypeDatabase::IsA(const Btl::String& typeName, E_XtType type)
{
	XtTypeHashMap::Iterator it = m_mapping.Find(typeName);
	if (it != m_mapping.End())
	{
		I_XtType* mappedType = it->GetValue();
		if (mappedType->IsA(type))
			return true;

		LtStruct* xtType = AsStruct(mappedType->GetXtType());
		if (xtType->base)
		{
			if (IsA(xtType->base->type->name.Text(), type))
				return true;
		}
	}

	return false;
}

void C_XtTypeDatabase::RegisterType(LarvaType* pSchemaType)
{
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
	case E_XtType_SceneXtActorNode:
		pXtType = new C_XtSceneActorType(AsStruct(pSchemaType));
		break;
	case E_XtType_SceneXtNode:
		pXtType = new C_XtSceneNodeType(AsStruct(pSchemaType));
		break;
	case E_XtType_DgXtCompNode:
		pXtType = new C_XtActorComponentType(AsStruct(pSchemaType));
		break;
	case E_XtType_DgXtPropNode:
		pXtType = new C_XtActorPropType(pSchemaType);
		break;
	case E_XtType_DgXtGameDataNode:
		pXtType = new C_XtGameDataType(AsStruct(pSchemaType));
		break;
	default:
		return;
	};

	//Add to registry.
	m_mapping[name] = pXtType;
	return;
}

}
