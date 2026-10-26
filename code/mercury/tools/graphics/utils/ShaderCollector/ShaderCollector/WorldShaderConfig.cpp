#include "stdafx.h"

#include "WorldShaderConfig.h"

#include <system/serialise/C_FileBlock.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include <System/DebugLog.h>

namespace ShaderRegistry
{

Debug::Logger &g_Log = Debug::GetLogger("WorldRender.ShaderRegistry");

enum E_FileVersion
{
	E_FileVersion_Invalid = 0,
	E_FileVersion_Initial,
	E_FileVersion_Current = E_FileVersion_Initial
};

// C_AxisEnum

C_AxisEnum::C_AxisEnum()
{
}

C_AxisEnum::C_AxisEnum( const S32 nameId ) :
	m_NameId(nameId)
{
}

C_AxisEnum::~C_AxisEnum()
{
}

// C_Shader

C_Shader::C_Shader()
{
}

C_Shader::C_Shader( const S32 nameId ) :
	m_NameId(nameId)
{
}

C_Shader::~C_Shader()
{
}

// C_TableItem

C_TableItem::C_TableItem()
{
}

C_TableItem::C_TableItem( const S32 vertexShaderIndex, const S32 pixelShaderIndex ) :
	m_VertexShaderIndex( vertexShaderIndex ),
	m_PixelShaderIndex( pixelShaderIndex )
{
}

C_TableItem::~C_TableItem()
{
}

// C_AxisCombinationEnum

C_AxisCombinationEnum::C_AxisCombinationEnum()
{
}

C_AxisCombinationEnum::C_AxisCombinationEnum(const S32 enumType, const S32 multiplier) :
	m_EnumType(enumType),
	m_IndexMultiplier(multiplier)
{
}

C_AxisCombinationEnum::~C_AxisCombinationEnum()
{
}

// C_AxisCombination

C_AxisCombination::C_AxisCombination()
{
}

C_AxisCombination::C_AxisCombination( const S32 nameId ) :
	m_NameId(nameId)
{
}

C_AxisCombination::~C_AxisCombination()
{
}

// C_TableAxis

C_TableAxis::C_TableAxis()
{
}

C_TableAxis::~C_TableAxis()
{
}

const S32 C_TableAxis::Size() const
{
	return m_Size;
}

const S32 C_TableAxis::GetIndex( const S32 combinationIndex, S32* enumValues )
{
	if((combinationIndex < 0) || (combinationIndex >= m_AxisCombinationList.Size()))
		return -1;

	C_AxisCombination& combination = m_AxisCombinationList[combinationIndex];

	S32 result = combination.m_IndexStart;
	const S32 enumCount = combination.m_EnumList.Size();
	for(S32 i=0; i<enumCount; i++)
	{
		result += enumValues[i] * combination.m_EnumList[i].m_IndexMultiplier;
	}

	return result;
}


// C_Define

C_Define::C_Define()
{
}

C_Define::C_Define( const S32 nameId, const S32 textId ) :
	m_NameId(nameId), 
	m_TextId(textId)
{
}

C_Define::~C_Define()
{
}

// C_Registry

C_Registry::C_Registry()
{
}

C_Registry::~C_Registry()
{
}

void C_Registry::Load( Sys::I_File& file  )
{
	Sys::C_FileBlock rootBlock( &file );
	if( rootBlock.HeaderId() != "ShdrCfg" )
	{		
		LOGWARN( g_Log, "Expected a ShdrCfg Block" );
		return;
	}

	U32 fileVersion = E_FileVersion_Invalid;

	while ( rootBlock.GetPosition() < rootBlock.Length() )
	{
		Sys::C_FileBlock block( &rootBlock );

		if( block.HeaderId() == "Header" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> fileVersion;
		}
		else
		if( block.HeaderId() == "StrTab" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> m_StringTable;
		}
		else
		if( block.HeaderId() == "Data" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> m_DefineList;
			ar >> m_AxisEnumList;
			ar >> m_VertexShaderList;
			ar >> m_PixelShaderList;
			ar >> m_OfflineAxis;
			ar >> m_IngameAxis;
			ar >> m_TableItemList;
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header %s", block.HeaderId().Chars() );
		}
	}
}

void C_Registry::Save( Sys::I_File& file  )
{
	Sys::C_FileBlock rootBlock( &file, "ShdrCfg" );

	{
		Sys::C_FileBlock headerBlock( &rootBlock, "Header" );
		Sys::BinarySerialiserAutoEndian ar( headerBlock );

		U32 fileVersion = E_FileVersion_Current;
		ar << fileVersion;
	}

	{
		Sys::C_FileBlock stringTableBlock( &rootBlock, "StrTab" );
		Sys::BinarySerialiserAutoEndian ar( stringTableBlock );

		ar << m_StringTable;
	}

	{
		Sys::C_FileBlock dataBlock( &rootBlock, "Data" );
		Sys::BinarySerialiserAutoEndian ar( dataBlock );

		ar << m_DefineList;
		ar << m_AxisEnumList;
		ar << m_VertexShaderList;
		ar << m_PixelShaderList;
		ar << m_OfflineAxis;
		ar << m_IngameAxis;
		ar << m_TableItemList;
	}
}

C_TableAxis& C_Registry::GetOfflineAxis()
{
	return m_OfflineAxis;
}

C_TableAxis& C_Registry::GetIngameAxis()
{
	return m_IngameAxis;
}

C_AxisEnum& C_Registry::GetAxisEnum(const S32 enumIndex)
{
	return m_AxisEnumList[enumIndex];
}

C_TableItem& C_Registry::GetTableItem(const S32 offlineIndex, const S32 ingameIndex)
{
	const S32 ingameCount = GetOfflineAxis().Size();
	return m_TableItemList[ offlineIndex + ingameCount * ingameIndex ];
}

const S32 C_Registry::LookupAxisIndex( C_TableAxis& axis, const Util::C_String& name )
{
	for(S32 i=0; i<axis.m_IndexNameIdList.Size(); i++)
	{
		const S32 nameId = axis.m_IndexNameIdList[i];
		const Util::C_String toMatch( m_StringTable.GetString( nameId ) );
		if( toMatch == name )
			return i;
	}

	return -1;
}

const S32 C_Registry::LookupDefineIndex( const Util::C_String& name )
{
	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		const S32 nameId = m_DefineList[i].m_TextId;
		const Util::C_String toMatch( m_StringTable.GetString( nameId ) );
		if( toMatch == name )
			return i;
	}

	return -1;
}


const S32 C_Registry::LookupAxisEnumIndex( const Util::C_String& name )
{
	for(S32 i=0; i<m_AxisEnumList.Size(); i++)
	{
		const S32 nameId = m_AxisEnumList[i].m_NameId;
		const Util::C_String toMatch( m_StringTable.GetString( nameId ) );
		if( toMatch == name )
			return i;
	}

	return -1;
}

const S32 C_Registry::LookupAxisEnumValue( const S32 axisEnumIndex, const Util::C_String& name )
{
	C_AxisEnum& axisEnum = m_AxisEnumList[axisEnumIndex];
	for(S32 i=0; i<axisEnum.m_ItemNameIdList.Size(); i++)
	{
		const S32 nameId = axisEnum.m_ItemNameIdList[i];
		const Util::C_String toMatch( m_StringTable.GetString( nameId ) );
		if( toMatch == name )
			return i;
	}

	return -1;
}

void C_Registry::Dump()
{
	LOGINFO(g_Log, "");
	LOGINFO(g_Log, "C_Registry::Dump()");
	LOGINFO(g_Log, "");

	LOGINFO(g_Log, "%d Defines:", m_DefineList.Size());
	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		LOGINFO(g_Log, "Define[%d] : \"%s\" (%s)\n", 
			i, 
			m_StringTable.GetString(m_DefineList[i].m_NameId), 
			m_StringTable.GetString(m_DefineList[i].m_TextId) );
	}

	LOGINFO(g_Log, "");

	LOGINFO(g_Log, "%d AxisEnums:", m_AxisEnumList.Size());
	for(S32 i=0; i<m_AxisEnumList.Size(); i++)
	{
		C_AxisEnum& axisEnum = m_AxisEnumList[i];
		const S32 itemCount = axisEnum.m_ItemNameIdList.Size();
		LOGINFO(g_Log, "");

		LOGINFO(g_Log, "   AxisEnum[%d] : \"%s\" %d items\n", 
			i, 
			m_StringTable.GetString(axisEnum.m_NameId), 
			itemCount );

		for(S32 j=0; j<itemCount; j++)
		{
			const S32 nameId = axisEnum.m_ItemNameIdList[j];
			LOGINFO(g_Log, "      Item[%d] : %s\n",
				j,
				m_StringTable.GetString(nameId) );
		}
	}

	LOGINFO(g_Log, "");

	LOGINFO(g_Log, "Offline Axis:");

	Dump( m_OfflineAxis );

	LOGINFO(g_Log, "");

	LOGINFO(g_Log, "Ingame Axis:");

	Dump( m_IngameAxis );

	LOGINFO(g_Log, "");
	LOGINFO(g_Log, "Shader Table [vertex, pixel]");
	LOGINFO(g_Log, "");

	const S32 ingameAxisSize = m_IngameAxis.Size();
	const S32 offlineAxisSize = m_OfflineAxis.Size();

	for(S32 offlineIndex=0; offlineIndex<offlineAxisSize; offlineIndex++)
	{
		Util::C_String string;
		for(S32 ingameIndex=0; ingameIndex<ingameAxisSize; ingameIndex++)
		{
			Util::C_String partial;
			C_TableItem& tableItem = GetTableItem(offlineIndex, ingameIndex);
			S32 vertexShaderIndex = tableItem.m_VertexShaderIndex;
			S32 pixelShaderIndex = tableItem.m_PixelShaderIndex;
			partial.Printf("[%03d,%03d] ", vertexShaderIndex, pixelShaderIndex );
			string << partial;
		}

		//string << " " << m_OfflineAxis.Name(offlineAxisNo);

		if( string.Length() > 1000 )
		{
			string = string(0,1000);
		}

		LOGINFO( g_Log, "%s", string.Chars() );
	}

	LOGINFO(g_Log, "");

	LOGINFO(g_Log, "%d Vertex Shader(s) :", m_VertexShaderList.Size());
	for(S32 i=0; i<m_VertexShaderList.Size(); i++)
	{
		LOGINFO(g_Log, "Vertex Shader[%d] : \"%s\"\n", 
			i, 
			m_StringTable.GetString(m_VertexShaderList[i].m_NameId) );
	}

	LOGINFO(g_Log, "");

	LOGINFO(g_Log, "%d Pixel Shader(s) :", m_PixelShaderList.Size());
	for(S32 i=0; i<m_PixelShaderList.Size(); i++)
	{
		LOGINFO(g_Log, "Pixel Shader[%d] : \"%s\"\n", 
			i, 
			m_StringTable.GetString(m_PixelShaderList[i].m_NameId) );
	}

}

void C_Registry::Dump( C_TableAxis& tableAxis )
{
	LOGINFO(g_Log, "Array Size : %d", tableAxis.m_Size);

	const S32 combinationCount = tableAxis.m_AxisCombinationList.Size();
	LOGINFO(g_Log, "%d Combination(s) :", combinationCount);

	for(S32 combinationNo=0; combinationNo<combinationCount; combinationNo++)
	{
		C_AxisCombination& combination = tableAxis.m_AxisCombinationList[combinationNo];
		LOGINFO(g_Log, "   Combination[%d] \"%s\" IndexStart = %d",
			combinationNo,			
			m_StringTable.GetString( combination.m_NameId ),
			combination.m_IndexStart );

		const S32 enumCount = combination.m_EnumList.Size();
		LOGINFO(g_Log, "   %d Enum(s) :", enumCount);

		for(S32 enumNo=0; enumNo<enumCount; enumNo++)
		{
			const S32 enumType = combination.m_EnumList[enumNo].m_EnumType;			

			LOGINFO(g_Log, "      Enum \"%s\" Values %d, Multiplier:%d", 
				m_StringTable.GetString( m_AxisEnumList[enumType].m_NameId ),
				m_AxisEnumList[enumType].m_ItemNameIdList.Size(),
				combination.m_EnumList[enumNo].m_IndexMultiplier);
		}
	}
}


} // namespace ShaderRegistry
