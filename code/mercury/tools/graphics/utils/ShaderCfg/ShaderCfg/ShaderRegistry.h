#ifndef SHADER_REGISTRY_H
#define SHADER_REGISTRY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Containers.h>
#include <System/DebugLog.h>
#include <System/Xml/XmlBlockParser.h>
#include <system/Serialise.h>

#include <system/serialise/TextSerialise.h>
#include "WorldShaderConfig.h"

namespace ShaderCfg
{

extern Debug::Logger &g_Log;

const S32 kMaxEnums = 32;

// C_FragmentList

class C_FragmentList
{
public:
	void Add( S32 fragmentIndex );
	const S32 Size() { return m_FragmentList.Size(); }
public:
	Ut::Vector<S32> m_FragmentList;
};

// C_DefineList

class C_DefineList
{
public:
	void operator = (const C_DefineList& other);
	const bool operator ==( const C_DefineList& other ) const;

	const bool HasDefine( const S32 defineIndex ) const;

	void Add( S32 defineIndex );
	const S32 Size() { return m_DefineList.Size(); }

	const U32 GetDefineBits();
public:
	Ut::Vector<S32> m_DefineList;
};

// C_ShaderEnumItem

class C_ShaderEnumItem
{
public:
	C_ShaderEnumItem();
	~C_ShaderEnumItem();

public:
	Util::C_String m_Name;
};

// C_ShaderEnum

class C_ShaderEnum
{
public:
	C_ShaderEnum();
	~C_ShaderEnum();

	void Dump();

public:
	S32								m_Index;
	Util::C_String					m_Name;
	Ut::Vector<C_ShaderEnumItem>	m_ItemList;
};

// C_AxisCombinationEnum

class C_AxisCombinationEnum
{
public:
	C_AxisCombinationEnum();
	~C_AxisCombinationEnum();

	void Dump();

public:
	C_ShaderEnum* m_pEnum;
	S32 m_EnumType;
	S32 m_IndexMultiplier;
};

// C_AxisCombination

class C_AxisCombination
{
public:
	C_AxisCombination();
	~C_AxisCombination();

	void Dump();

public:
	Util::C_String m_Name;
	S32 m_IndexStart;
	Ut::Vector<C_AxisCombinationEnum> m_EnumList;
};

// C_ShaderTableAxis

class C_ShaderTableAxis
{
public:
	C_ShaderTableAxis();
	~C_ShaderTableAxis();

	void Dump();

	void CalcInfo();

	// total size of array indexed by this index list
	const S32 Size();

private:
	void BuildIndexNameList();

public:

	Util::C_String m_Name;

	S32 m_Index;

	S32 m_Size;

	Ut::Vector<C_AxisCombination>	m_AxisCombinationList;

	Ut::Vector<Util::C_String>		m_IndexNameList;
};

// Used to access the table axis

class C_TableAxisIndex
{
public:	
	C_TableAxisIndex( C_ShaderTableAxis* pAxis );
	~C_TableAxisIndex();

	void Set( const S32 index );

	void Reset( );
	void Increment( );

	Util::C_String GetEnumString();

	Util::C_String GetString(  );

private:
	void ResetCombination( const S32 index );

public:	
	S32	m_CombinationIndex;
	S32 m_EnumValues[kMaxEnums];

	S32 m_Index;
	bool m_Finished;

	C_ShaderTableAxis* m_pAxis;
};

// C_Shader

class C_Shader
{
public:
	Util::C_String GetName() { return m_TextId; }
public:	
	Util::C_String		m_TextId;
	C_DefineList		m_DefineList;
};

// class C_TableItem

class C_TableItem
{
public:
	S32		m_VertexShaderIndex;
	S32		m_PixelShaderIndex;
};

// C_ShaderRegistry

class C_ShaderRegistry
{
public:
	C_ShaderRegistry( C_ShaderConfig* pShaderConfig );
	~C_ShaderRegistry();

	void Dump();

	const S32 LookupEnum( const Util::C_String& name );
	const S32 LookupAxis( const Util::C_String& name );

	void CalcInfo();
	void BuildTable();

	C_ShaderTableAxis& GetOfflineAxis();
	C_ShaderTableAxis& GetIngameAxis();

	C_TableItem& GetTableItem(const S32 offlineIndex, const S32 ingameIndex);

	void BuildFragmentList(C_FragmentList& fragmentList, const Util::C_String& enumString);
	void BuildDefineLists(C_FragmentList& fragmentList, C_DefineList& vertexDefines, C_DefineList& pixelDefines);

	S32 GetVertexShader(C_DefineList& defineList);
	S32 GetPixelShader(C_DefineList& defineList);

	void CreateGameRegistry( ShaderRegistry::C_Registry& reg );

private:
	void SetupShader( C_Shader& shader, C_DefineList& defineList, const Util::C_String& prefix );
	void CreateGameRegistryAxis( Util::C_StringTableBuilder& stringTableBuilder, ShaderRegistry::C_Registry& reg, ShaderRegistry::C_TableAxis& regTableAxis, C_ShaderTableAxis& srcTableAxis );

public:
	C_ShaderConfig*					m_pShaderConfig;
	Ut::Vector<C_ShaderEnum>		m_ShaderEnumList;

	Ut::Vector<C_ShaderTableAxis>	m_ShaderTableAxisList;

	S32								m_OfflineAxis;
	S32								m_IngameAxis;

	Ut::Vector<C_TableItem>			m_TableItemList;

	Ut::Vector<C_Shader>			m_VertexShaderList;

	Ut::Vector<C_Shader>			m_PixelShaderList;

};

} // namespace ShaderCfg

#endif