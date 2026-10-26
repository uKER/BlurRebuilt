#ifndef EDITOR_PROPERTY_VISIBILITY_MANAGER_H
#define EDITOR_PROPERTY_VISIBILITY_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "core/utility/Singleton.h"

#include <EditorApp/Frame/Menus/FrameDisplayMenu.h>

namespace EditorLib
{
	class C_DgXtPropNodeBase;
}

namespace Editor
{

class C_FrameDisplayMenu;

enum E_PropertyVisibilityType
{
	E_PropertyVisibilityType_None,
	E_PropertyVisibilityType_All,
	E_PropertyVisibilityType_Selection
};

class C_PropertyVisibilityData;

class C_PropertyVisibilityManager : public Ut::Singleton<C_PropertyVisibilityManager, Ut::CreateUsingNew>
{
public:
	C_PropertyVisibilityManager();
	~C_PropertyVisibilityManager();

	template<typename T>
	bool Register(const Btl::String& displayName);

	bool IsVisible(EditorLib::C_DgXtPropNodeBase* pPropNode);

	C_PropertyVisibilityData* GetVisibilityData(const Btl::String& typeName);

	void SetMenu(C_FrameDisplayMenu* menu);

	void UpdateViews();

private:
	Btl::OpenHashMap<Btl::String,C_PropertyVisibilityData, Btl::HashKeyOpBtlString> m_RegisteredProperties;

	C_FrameDisplayMenu* m_pMenu;
};

class C_PropertyVisibilityData
{
public:
	C_PropertyVisibilityData()
	{
		m_Manager = 0;
	}

	C_PropertyVisibilityData(C_PropertyVisibilityManager* manager)
	{
		m_Manager = manager;
	}

	void SetVisibilityType(E_PropertyVisibilityType visibilityType)
	{
		m_VisibilityType = visibilityType;
		if (m_Manager)
			m_Manager->UpdateViews();
	}

	void SetTypeName(const Btl::String& typeName)
	{
		m_TypeName = typeName;
	}

	Btl::String& GetTypeName()
	{
		return m_TypeName;
	}

	bool IsVisible()
	{
		return m_VisibilityType != E_PropertyVisibilityType_None;
	}

	E_PropertyVisibilityType GetVisibilityType()
	{
		return m_VisibilityType;
	}

private:
	C_PropertyVisibilityManager* m_Manager;
	E_PropertyVisibilityType m_VisibilityType;
	Btl::String	m_TypeName;
};

#include "PropertyVisibilityManager.inl"

}

#endif