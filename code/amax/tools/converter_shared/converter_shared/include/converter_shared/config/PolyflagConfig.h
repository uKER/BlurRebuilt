#ifndef CONVERTER_SHARED_POLYFLAG_CONFIG_H
#define CONVERTER_SHARED_POLYFLAG_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

namespace Aftk
{
	class C_Polygon;
}

namespace Polyflags
{

typedef U32 C_Value;

class C_Polyflags;

// C_Flag

class C_Flag
{
friend class C_Polyflags;
public:
	C_Flag();
	~C_Flag();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode );

	void Dump();

public:
	Util::C_String	m_Id;
	C_Value			m_Value;
};

// C_EnumItem

class C_EnumItem
{
friend class C_Polyflags;
public:
	C_EnumItem();
	~C_EnumItem();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode );

	void Dump();

public:
	U32				m_Index;
	Util::C_String	m_Id;
	C_Value			m_Value;
};

// C_Enum

class C_Enum
{
friend class C_Polyflags;
public:
	C_Enum();
	~C_Enum();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode );

	void Dump();

public:
	Util::C_String	m_Id;
	Btl::Vector<C_EnumItem> m_EnumItemList;
};

// C_LayerFlag

class C_LayerFlag
{
friend class C_Polyflags;
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
friend class C_Polyflags;
public:
	C_Item();
	C_Item( const Util::C_String& name );
	~C_Item();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode );

	const Util::C_String& GetName() const;

	const Util::C_String GetFirstFlagId() const;

	void Dump();

private:
	Util::C_String m_Name;

	Btl::Vector<C_Flag> m_FlagList;
	Btl::Vector<C_Enum> m_EnumList;
	Btl::Vector<C_LayerFlag> m_LayerFlagList;
};

// C_Config

class C_Config
{
friend class C_Polyflags;
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

// C_Polyflags 

class C_Polyflags
{
public:
	C_Polyflags();
	C_Polyflags( const C_Config& config, const Aftk::C_Polygon* pPolygon );
	~C_Polyflags();

	C_Value GetValue( const S32 index );

private:
	Btl::Vector<C_Value> m_ValueList;
};

} // namespace Polyflags

#endif
