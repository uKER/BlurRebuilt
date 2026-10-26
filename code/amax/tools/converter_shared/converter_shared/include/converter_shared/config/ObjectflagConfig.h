#ifndef CONVERTER_SHARED_OBJECTFLAG_CONFIG_H
#define CONVERTER_SHARED_OBJECTFLAG_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

namespace Aftk
{
	class C_SceneNode;
	class C_LayerNode;
}

namespace Objectflags
{

typedef U32 C_Value;

class C_Objectflags;

// C_ObjectNameFlag

class C_ObjectNameFlag
{
	enum E_Type
	{
		E_Type_Infix = 0,
		E_Type_Prefix,
		E_Type_Postfix,
		E_Type_Count
	};

friend class C_Objectflags;
public:
	C_ObjectNameFlag();
	~C_ObjectNameFlag();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode );

	void Dump();

public:
	Util::C_String		m_Id;
	E_Type				m_Type;
	C_Value				m_Value;
};

// C_LayerFlag

class C_LayerFlag
{
friend class C_Objectflags;
public:
	C_LayerFlag();
	~C_LayerFlag();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode );

	void Dump();

public:
	Util::C_String	m_Id;
	C_Value			m_Value;
};

// C_Item

class C_Item
{
friend class C_Objectflags;
public:
	C_Item();
	C_Item( const Util::C_String& name );
	~C_Item();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode );

	const Util::C_String& GetName() const;

	void Dump();

private:
	Util::C_String m_Name;

	Btl::Vector<C_LayerFlag> m_LayerFlagList;
	Btl::Vector<C_ObjectNameFlag> m_ObjectNameFlagList;
};

// C_Config

class C_Config
{
friend class C_Objectflags;
public:
	C_Config();
	~C_Config();

	void Dump();

	bool ReadXml( const Util::C_String& xmlFilePath );

	bool RegisterItems( Btl::Vector<Util::C_String> itemNameList );

	C_Item* LookupItem( const Util::C_String& itemName );

	S32 GetItemCount() const;

	C_Item& GetItem( const S32 itemIndex );

	const C_Item& GetItem( const S32 itemIndex ) const;

private:
	bool ParseXml( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom );
	bool ParseXml( const Sys::C_XmlDomBaseNode* pRootNode );

private:
	Btl::Vector<C_Item> m_ItemList;
};

// C_Objectflags 

class C_Objectflags
{
public:
	C_Objectflags();
	C_Objectflags( const C_Config& config, const Aftk::C_SceneNode* pSceneNode );
	C_Objectflags( const C_Config& config, const Aftk::C_LayerNode* pLayerNode );
	~C_Objectflags();

	C_Value GetValue( const S32 index );

private:
	Btl::Vector<C_Value> m_ValueList;
};

} // namespace ObjectFlags

#endif
