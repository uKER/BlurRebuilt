#ifndef WORLD_SHADER_CONFIG_H
#define WORLD_SHADER_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/Serialise.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <core/core.h>
#include <core/containers.h>

namespace ShaderCfg
{
	class C_ShaderRegistry;
}

namespace ShaderRegistry
{

// C_AxisEnum

class C_AxisEnum
{
public:
	C_AxisEnum();
	C_AxisEnum(const S32 nameId);
	~C_AxisEnum();

	template <typename Archive>
	void Serialise( Archive& ar ) const
	{
		ar << m_NameId;
		ar << m_ItemNameIdList;
	}
 
	template <typename Archive>
	void Deserialise( Archive& ar )
	{
		ar >> m_NameId;
		ar >> m_ItemNameIdList;
	}

public:
	S32					m_NameId;
	Ut::Vector<S32>		m_ItemNameIdList;
};

// C_Shader

class C_Shader
{
public:
	C_Shader();
	C_Shader( const S32 nameId );
	~C_Shader();
	
	template <typename Archive>
	void Serialise( Archive& ar ) const
	{
		ar << m_NameId;
		ar << m_DefineBits;
	}
 
	template <typename Archive>
	void Deserialise( Archive& ar )
	{
		ar >> m_NameId;
		ar >> m_DefineBits;
	}

public:
	S32 m_NameId;
	U32 m_DefineBits;
};

// C_TableItem

class C_TableItem
{
public:
	C_TableItem();
	C_TableItem( const S32 vertexShaderIndex, const S32 pixelShaderIndex );
	~C_TableItem();

	template <typename Archive>
	void Serialise( Archive& ar ) const
	{
		ar << m_VertexShaderIndex;
		ar << m_PixelShaderIndex;
		ar << m_DefineBits;
	}
 
	template <typename Archive>
	void Deserialise( Archive& ar )
	{
		ar >> m_VertexShaderIndex;
		ar >> m_PixelShaderIndex;
		ar >> m_DefineBits;
	}

public:
	S32		m_VertexShaderIndex;
	S32		m_PixelShaderIndex;

	U32		m_DefineBits;
};

// C_AxisCombinationEnum

class C_AxisCombinationEnum
{
public:
	C_AxisCombinationEnum();
	C_AxisCombinationEnum( const S32 enumType, const S32 multiplier );
	~C_AxisCombinationEnum();
	
	template <typename Archive>
	void Serialise( Archive& ar ) const
	{
		ar << m_EnumType;
		ar << m_IndexMultiplier;
	}
 
	template <typename Archive>
	void Deserialise( Archive& ar )
	{
		ar >> m_EnumType;
		ar >> m_IndexMultiplier;
	}

public:
	S32 m_EnumType;
	S32 m_IndexMultiplier;
};

// C_AxisCombination

class C_AxisCombination
{
public:
	C_AxisCombination();
	C_AxisCombination( const S32 nameId );
	~C_AxisCombination();

	template <typename Archive>
	void Serialise( Archive& ar ) const
	{
		ar << m_NameId;
		ar << m_IndexStart;
		ar << m_EnumList;
	}
 
	template <typename Archive>
	void Deserialise( Archive& ar )
	{
		ar >> m_NameId;
		ar >> m_IndexStart;
		ar >> m_EnumList;
	}

public:
	S32 m_NameId;
	S32 m_IndexStart;
	Ut::Vector<C_AxisCombinationEnum> m_EnumList;
};

// C_TableAxis

class C_TableAxis
{
public:
	C_TableAxis();
	~C_TableAxis();
	
	const S32 Size() const;

	const S32 GetIndex( const S32 combinationIndex, S32* enumValues );

	template <typename Archive>
	void Serialise( Archive& ar ) const
	{
		ar << m_Size;
		ar << m_AxisCombinationList;
		ar << m_IndexNameIdList;
	}
 
	template <typename Archive>
	void Deserialise( Archive& ar )
	{
		ar >> m_Size;
		ar >> m_AxisCombinationList;
		ar >> m_IndexNameIdList;
	}

public:
	S32			m_Size;
	Ut::Vector<C_AxisCombination>	m_AxisCombinationList;
	Ut::Vector<S32>					m_IndexNameIdList;

};

// C_Define
class C_Define
{
public:
	C_Define();
	C_Define( const S32 nameId, const S32 textId );
	~C_Define();

	template <typename Archive>
	void Serialise( Archive& ar ) const
	{
		ar << m_NameId;
		ar << m_TextId;
	}
 
	template <typename Archive>
	void Deserialise( Archive& ar )
	{
		ar >> m_NameId;
		ar >> m_TextId;
	}

public:
	S32			m_NameId;
	S32			m_TextId;
};

// C_Registry

class C_Registry
{
friend class ShaderCfg::C_ShaderRegistry;

public:
	C_Registry();
	~C_Registry();

	void Load( Sys::I_File& file  );

	void Save( Sys::I_File& file  );

	void Dump();

	C_TableItem& GetTableItem(const S32 offlineIndex, const S32 ingameIndex);

	C_TableAxis& GetOfflineAxis();

	C_TableAxis& GetIngameAxis();

	C_AxisEnum& GetAxisEnum(const S32 enumIndex);

	Util::C_StringTable& GetStringTable() { return m_StringTable; }

	Ut::Vector<C_Shader>& GetVertexShaderList() { return m_VertexShaderList; }

	Ut::Vector<C_Shader>& GetPixelShaderList() { return m_PixelShaderList; }

	Ut::Vector<C_TableItem>& GetTableItemList() { return m_TableItemList; }

	const S32 LookupAxisIndex( C_TableAxis& axis, const Util::C_String& name );

	const S32 LookupDefineIndex( const Util::C_String& name );

	const S32 LookupAxisEnumIndex( const Util::C_String& name );

	const S32 LookupAxisEnumValue( const S32 axisEnumIndex, const Util::C_String& name );

private:
	void Dump( C_TableAxis& tableAxis );

private:
	Util::C_StringTable			m_StringTable;

	Ut::Vector<C_Define>		m_DefineList;

	Ut::Vector<C_AxisEnum>		m_AxisEnumList;

	Ut::Vector<C_Shader>		m_VertexShaderList;

	Ut::Vector<C_Shader>		m_PixelShaderList;

	C_TableAxis					m_OfflineAxis;

	C_TableAxis					m_IngameAxis;

	Ut::Vector<C_TableItem>		m_TableItemList;
};


} // namespace ShaderRegistry


#endif
