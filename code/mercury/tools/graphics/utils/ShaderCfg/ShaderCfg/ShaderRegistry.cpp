
#include "stdafx.h"

#include <System/DebugLog.h>

#include <core/containers.h>
#include <core/utility/endian.h>
#include <system/Serialise.h>
#include <misc_shared/DataStructure/StringTable.h>

#include "ShaderConfig.h"
#include "ShaderRegistry.h"

namespace ShaderCfg
{

// C_FragmentList

void C_FragmentList::Add( S32 fragmentIndex )
{
	for(S32 i=0; i<m_FragmentList.Size(); i++)
	{
		if(m_FragmentList[i] == fragmentIndex)
			return;
	}
	
	m_FragmentList.PushBack(fragmentIndex);
}

// C_DefineList

void C_DefineList::operator = (const C_DefineList& other)
{
	m_DefineList.Clear();
	m_DefineList.Reserve(other.m_DefineList.Size());
	for(S32 i=0; i<other.m_DefineList.Size(); i++)
	{
		m_DefineList.PushBack(other.m_DefineList[i]);
	}
}

const bool C_DefineList::operator ==( const C_DefineList& other ) const
{
	if(m_DefineList.Size() != other.m_DefineList.Size())
		return false;

	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		if(!other.HasDefine(m_DefineList[i]))
			return false;
	}

	return true;
}

const bool C_DefineList::HasDefine( const S32 defineIndex ) const
{
	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		if(m_DefineList[i] == defineIndex)
			return true;
	}

	return false;
}

void C_DefineList::Add( S32 defineIndex )
{
	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		if(m_DefineList[i] == defineIndex)
			return;
	}
	
	m_DefineList.PushBack(defineIndex);
}

const U32 C_DefineList::GetDefineBits()
{
	U32 bits = 0;
	
	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		bits |= 1 << m_DefineList[i];
	}

	return bits;
}

// C_ShaderEnumItem

C_ShaderEnumItem::C_ShaderEnumItem()
{
}

C_ShaderEnumItem::~C_ShaderEnumItem()
{
}

// C_ShaderEnum

C_ShaderEnum::C_ShaderEnum()
{
}

C_ShaderEnum::~C_ShaderEnum()
{
}

void C_ShaderEnum::Dump()
{
	LOGINFO( g_Log, "C_ShaderEnum %s[%d]", m_Name.Chars(), m_Index );

	for(S32 i=0; i<m_ItemList.Size(); i++)
	{
		LOGINFO( g_Log, "   Value %s", m_ItemList[i].m_Name.Chars() );		
	}

}


// C_AxisCombinationEnum

C_AxisCombinationEnum::C_AxisCombinationEnum() :
	m_pEnum(NULL),
	m_EnumType(-1),
	m_IndexMultiplier(0)
{
}

C_AxisCombinationEnum::~C_AxisCombinationEnum()
{
}

void C_AxisCombinationEnum::Dump()
{
	LOGINFO( g_Log, "      C_AxisCombinationEnum %s[%d] Mul = %d", m_pEnum->m_Name.Chars(), m_pEnum->m_ItemList.Size(), m_IndexMultiplier );
}

// C_IndexCombination

C_AxisCombination::C_AxisCombination()
{
}

C_AxisCombination::~C_AxisCombination()
{
}

void C_AxisCombination::Dump()
{
	LOGINFO( g_Log, "   C_AxisCombination %s", m_Name.Chars() );

	for(S32 i=0; i<m_EnumList.Size(); i++)
	{
		m_EnumList[i].Dump();
	}
}

// C_ShaderIndexList

C_ShaderTableAxis::C_ShaderTableAxis() : 
	m_Size(0)
{
}

C_ShaderTableAxis::~C_ShaderTableAxis()
{
}

void C_ShaderTableAxis::Dump()
{
	LOGINFO( g_Log, "C_ShaderTableAxis %d items", Size() );

	for(S32 i=0; i<m_AxisCombinationList.Size(); i++)
	{
		m_AxisCombinationList[i].Dump();
	}
}

void C_ShaderTableAxis::CalcInfo()
{
	S32 indexStart = 0;

	for(S32 i=0; i<m_AxisCombinationList.Size(); i++)
	{
		C_AxisCombination& axisCombination = m_AxisCombinationList[i];

		//LOGINFO( g_Log, "Combination %s", axisCombination.m_Name.Chars() );

		axisCombination.m_IndexStart = indexStart;
		S32 multiplier = 1;

		const S32 enumCount = axisCombination.m_EnumList.Size();
		for(S32 enumNo=0; enumNo<enumCount; enumNo++)
		{
			C_AxisCombinationEnum& combinationEnum = axisCombination.m_EnumList[enumNo];
			const S32 noEnumItems = combinationEnum.m_pEnum->m_ItemList.Size();

			combinationEnum.m_IndexMultiplier = multiplier;
			//LOGINFO( g_Log, "   Enums:%d", noEnumItems );
			//LOGINFO( g_Log, "   Multiplier:%d", multiplier );

			multiplier = multiplier * noEnumItems;
		}		


		indexStart += multiplier;
	}

	//LOGINFO( g_Log, "Size:%d", indexStart );
	m_Size = indexStart;

	BuildIndexNameList();
}

void C_ShaderTableAxis::BuildIndexNameList()
{
	C_TableAxisIndex index(this);

	S32 i = 0;

	index.Reset();
	while(!index.m_Finished)
	{
		Util::C_String string = index.GetEnumString();
		m_IndexNameList.PushBack( string );
		//LOGINFO( g_Log, "Axis Index Name[%d] = %s", i, string.Chars() );
		index.Increment();
		i++;
	}


	//for(S32 i=0; i<m_AxisCombinationList.Size(); i++)
	//{
	//	C_AxisCombination& axisCombination = m_AxisCombinationList[i];

	//	LOGINFO( g_Log, "Combination %s", axisCombination.m_Name.Chars() );

	//	const S32 enumCount = axisCombination.m_EnumList.Size();
	//	for(S32 enumNo=0; enumNo<enumCount; enumNo++)
	//	{
	//		C_AxisCombinationEnum& combinationEnum = axisCombination.m_EnumList[enumNo];
	//		const S32 noEnumItems = combinationEnum.m_pEnum->m_ItemList.Size();
	//		for(S32 enumNo=0; enumNo<enumCount; enumNo++)
	//		{
	//		}
	//	}
	//}
}


const S32 C_ShaderTableAxis::Size()
{
	return m_Size;
}

// C_TableAxisIndex

C_TableAxisIndex::C_TableAxisIndex( C_ShaderTableAxis* pAxis ) :
	m_pAxis( pAxis )
{
}

C_TableAxisIndex::~C_TableAxisIndex()
{
}

void C_TableAxisIndex::Set( const S32 index )
{
	assert(false && "Not Implemented");
}

void C_TableAxisIndex::ResetCombination( const S32 combinationIndex )
{
	C_AxisCombination& axisCombination = m_pAxis->m_AxisCombinationList[combinationIndex];
	const S32 enumCount = axisCombination.m_EnumList.Size();
	for(S32 enumNo = 0; enumNo<enumCount; enumNo++)
	{
		m_EnumValues[enumNo] = 0;
	}
}

void C_TableAxisIndex::Reset( )
{
	m_Index = 0;
	m_Finished = false;

	m_CombinationIndex = 0;

	if(m_pAxis->m_AxisCombinationList.Size() > 0)
	{
		ResetCombination( 0 );
	}
}

void C_TableAxisIndex::Increment( )
{
	m_Index++;

	if((m_CombinationIndex < 0) || (m_CombinationIndex >= m_pAxis->m_AxisCombinationList.Size()))
		return;

	C_AxisCombination& axisCombination = m_pAxis->m_AxisCombinationList[m_CombinationIndex];
	const S32 enumCount = axisCombination.m_EnumList.Size();
	
	S32 enumNo = 0;
	while( enumNo < enumCount )
	{
		C_AxisCombinationEnum& enumDef = axisCombination.m_EnumList[enumNo];
		m_EnumValues[enumNo]++;
		if(m_EnumValues[enumNo] >= enumDef.m_pEnum->m_ItemList.Size())
		{
			m_EnumValues[enumNo] = 0;
		}
		else
		{
			return;
		}

		enumNo++;
	}

	m_CombinationIndex++;

	if(m_CombinationIndex < m_pAxis->m_AxisCombinationList.Size())
	{
		ResetCombination( m_CombinationIndex );
	}
	else
	{
		m_Finished = true;
	}
}

Util::C_String C_TableAxisIndex::GetString( )
{
	Util::C_String string;

	string << "[(" << m_CombinationIndex << ")";

	if((m_CombinationIndex < 0) || (m_CombinationIndex >= m_pAxis->m_AxisCombinationList.Size()))
		return string;

	C_AxisCombination& axisCombination = m_pAxis->m_AxisCombinationList[m_CombinationIndex];
	const S32 enumCount = axisCombination.m_EnumList.Size();

	for(S32 enumNo=0; enumNo<enumCount; enumNo++)
	{
		string << "[" << m_EnumValues[enumNo] << "]";		
	}

	return string;
}

Util::C_String C_TableAxisIndex::GetEnumString()
{
	Util::C_String string;

	if((m_CombinationIndex < 0) || (m_CombinationIndex >= m_pAxis->m_AxisCombinationList.Size()))
		return string;

	C_AxisCombination& axisCombination = m_pAxis->m_AxisCombinationList[m_CombinationIndex];
	const S32 enumCount = axisCombination.m_EnumList.Size();
	

	for(S32 enumNo=0; enumNo<enumCount; enumNo++)
	{
		C_AxisCombinationEnum& combinationEnum = axisCombination.m_EnumList[enumNo];		
		string << combinationEnum.m_pEnum->m_ItemList[m_EnumValues[enumNo]].m_Name.Chars();		
	}

	return string;
}

// C_ShaderRegistry

C_ShaderRegistry::C_ShaderRegistry( C_ShaderConfig* pShaderConfig ) :
	m_pShaderConfig( pShaderConfig )
{
}

C_ShaderRegistry::~C_ShaderRegistry()
{
}

const S32 C_ShaderRegistry::LookupEnum( const Util::C_String& name )
{
	for(S32 i=0; i<m_ShaderEnumList.Size(); i++)
	{
		if( m_ShaderEnumList[i].m_Name == name )
			return i;
	}

	return -1;
}

const S32 C_ShaderRegistry::LookupAxis( const Util::C_String& name )
{
	for(S32 i=0; i<m_ShaderTableAxisList.Size(); i++)
	{
		if( m_ShaderTableAxisList[i].m_Name == name )
			return i;
	}

	return -1;
}

void C_ShaderRegistry::Dump()
{
	for(S32 i=0; i<m_ShaderEnumList.Size(); i++)
	{
		m_ShaderEnumList[i].Dump();
	}

	for(S32 i=0; i<m_ShaderTableAxisList.Size(); i++)
	{
		m_ShaderTableAxisList[i].Dump();
	}
	LOGINFO( g_Log, "\n" );

	LOGINFO( g_Log, "%d Vertex Shaders", m_VertexShaderList.Size() );
	for(S32 i=0; i<m_VertexShaderList.Size();i++)
	{
		LOGINFO( g_Log, "   Vertex Shader[%d] = %s", i, m_VertexShaderList[i].m_TextId.Chars() );
	}
	LOGINFO( g_Log, "\n" );

	LOGINFO( g_Log, "%d Pixel Shaders", m_PixelShaderList.Size() );
	for(S32 i=0; i<m_PixelShaderList.Size();i++)
	{
		LOGINFO( g_Log, "   Pixel Shader[%d] = %s", i, m_PixelShaderList[i].m_TextId.Chars() );
	}

	LOGINFO( g_Log, "\n" );

	LOGINFO( g_Log, "   m_OfflineAxis %d", m_OfflineAxis );
	LOGINFO( g_Log, "   m_IngameAxis %d", m_IngameAxis );
}

C_ShaderTableAxis& C_ShaderRegistry::GetOfflineAxis()
{
	return m_ShaderTableAxisList[m_OfflineAxis];
}

C_ShaderTableAxis& C_ShaderRegistry::GetIngameAxis()
{
	return m_ShaderTableAxisList[m_IngameAxis];
}

void C_ShaderRegistry::CalcInfo()
{
	for(S32 i=0; i<m_ShaderTableAxisList.Size(); i++)
	{
		m_ShaderTableAxisList[i].CalcInfo();
	}
}

C_TableItem& C_ShaderRegistry::GetTableItem(const S32 offlineIndex, const S32 ingameIndex)
{
	const S32 ingameCount = GetOfflineAxis().Size();
	return m_TableItemList[ offlineIndex + ingameCount * ingameIndex ];
}

void C_ShaderRegistry::BuildTable()
{
	const S32 offlineCount = GetOfflineAxis().Size();
	const S32 ingameCount = GetIngameAxis().Size();

	m_TableItemList.Resize( offlineCount * ingameCount );

	C_TableAxisIndex offlineIndex( &GetOfflineAxis() );
	C_TableAxisIndex ingameIndex( &GetIngameAxis());

	offlineIndex.Reset();
	while( offlineIndex.m_Index < offlineCount )
	{
		ingameIndex.Reset();
		while( ingameIndex.m_Index < ingameCount )
		{
			C_TableItem& tableItem = GetTableItem( offlineIndex.m_Index, ingameIndex.m_Index );

			// create enum string...
			Util::C_String enumString = offlineIndex.GetEnumString() + ingameIndex.GetEnumString();

			C_FragmentList fragmentList;
			BuildFragmentList(fragmentList, enumString);

			C_DefineList vertexDefines;
			C_DefineList pixelDefines;

			BuildDefineLists( fragmentList, vertexDefines, pixelDefines );

			//LOGINFO( g_Log, "ingame : %s = %d offline : %s = %d EnumString:%s\n", ingameIndex.GetString().Chars(), ingameIndex.m_Index, offlineIndex.GetString().Chars(), offlineIndex.m_Index, enumString.Chars() );

			//LOGINFO( g_Log, "%d fragments %d vertex defines %d pixel defines ", fragmentList.Size(), vertexDefines.Size(), pixelDefines.Size());

			tableItem.m_VertexShaderIndex = GetVertexShader(vertexDefines);
			tableItem.m_PixelShaderIndex = GetPixelShader(pixelDefines);

			//LOGINFO( g_Log, "Vertex: %s Pixel: %s",
			//	m_VertexShaderList[tableItem.m_VertexShaderIndex].m_TextId.Chars(),
			//	m_PixelShaderList[tableItem.m_PixelShaderIndex].m_TextId.Chars() );

			ingameIndex.Increment( );
		}

		offlineIndex.Increment();
	}
}


void C_ShaderRegistry::BuildFragmentList(C_FragmentList& fragmentList, const Util::C_String& enumString)
{	
	const S32 fragmentCount = m_pShaderConfig->m_FragmentList.Size();

	Ut::Vector<bool> fragmentEnabledArray;
	fragmentEnabledArray.Resize(fragmentCount);

	// process regular conditions
	for(S32 fragmentNo=0; fragmentNo<fragmentCount; fragmentNo++)
	{
		C_Fragment& fragment = m_pShaderConfig->m_FragmentList[fragmentNo];

		bool fragmentValid = true;

		const S32 conditionCount = fragment.m_ConditionList.Size();
		for(S32 conditionNo=0; conditionNo<conditionCount; conditionNo++)
		{
			C_Condition& condition = fragment.m_ConditionList[conditionNo];
			switch(condition.m_Type)
			{
				case E_ConditionType_None:
				break;

				case E_ConditionType_Mutex:
				break;

				case E_ConditionType_Required:
					if(enumString.Search(condition.m_Id) == -1)
					{
						fragmentValid = false;
					}
				break;

				case E_ConditionType_Exclude:
					if(enumString.Search(condition.m_Id) != -1)
					{
						fragmentValid = false;
					}
				break;
			}
		}

		fragmentEnabledArray[fragmentNo] = fragmentValid;
	}

	// process the Mutex conditions
	for(S32 fragmentNo=0; fragmentNo<fragmentCount; fragmentNo++)
	{
		C_Fragment& fragment = m_pShaderConfig->m_FragmentList[fragmentNo];

		bool fragmentValid = fragmentEnabledArray[fragmentNo];

		const S32 conditionCount = fragment.m_ConditionList.Size();
		for(S32 conditionNo=0; conditionNo<conditionCount; conditionNo++)
		{
			C_Condition& condition = fragment.m_ConditionList[conditionNo];
			switch(condition.m_Type)
			{
				case E_ConditionType_None:
				break;

				case E_ConditionType_Mutex:
				{
					// if other fragment enabled, disable this fragment
					//LOGINFO( g_Log, "Testing Mutex Condition of %s on %s", condition.m_Id.Chars(), fragment.m_Name.Chars() );

					// lookup fragment from value
					ShaderCfg::C_Fragment* pFragment = m_pShaderConfig->LookupFragment( condition.m_Id );
					if(pFragment)
					{
						// TODO : Make sure indices are valid at this point!
						const S32 fragmentIndex = pFragment->m_Index;
						if(fragmentIndex != -1)
						{
							if(fragmentEnabledArray[fragmentIndex])
							{
								//LOGINFO( g_Log, "Ignoring due to mutex condition" );
								fragmentValid = false;
							}
						}
					}
				}
				break;

				case E_ConditionType_Required:
				break;

				case E_ConditionType_Exclude:
				break;
			}
		}


		if(fragmentValid)
		{
			fragmentList.Add(fragment.m_Index);
		}
	}
}

void C_ShaderRegistry::BuildDefineLists(C_FragmentList& fragmentList, C_DefineList& vertexDefines, C_DefineList& pixelDefines)
{
	for(S32 i=0; i<fragmentList.Size();i++)
	{
		C_Fragment& fragment = m_pShaderConfig->m_FragmentList[ fragmentList.m_FragmentList[i] ];

		const S32 defineCount = fragment.m_DefineList.Size();
		for(S32 defineNo=0; defineNo<defineCount; defineNo++)
		{
			S32 defineIndex = fragment.m_DefineList[defineNo];
			C_Define& define = m_pShaderConfig->m_DefineList[defineIndex];

			if(define.m_VertexShader)
			{
				vertexDefines.Add(defineIndex);
			}

			if(define.m_PixelShader)
			{
				pixelDefines.Add(defineIndex);
			}
		}
	}
}

S32 C_ShaderRegistry::GetVertexShader(C_DefineList& defineList)
{
	S32 vertexShaderCount = m_VertexShaderList.Size();
	for(S32 i=0; i<vertexShaderCount; i++)
	{
		if(m_VertexShaderList[i].m_DefineList == defineList)
			return i;
	}
	
	m_VertexShaderList.PushBack();

	C_Shader& newShader = m_VertexShaderList[vertexShaderCount];
	
	SetupShader(newShader, defineList, "VS_");

	return vertexShaderCount;
}

S32 C_ShaderRegistry::GetPixelShader(C_DefineList& defineList)
{
	S32 pixelShaderCount = m_PixelShaderList.Size();
	for(S32 i=0; i<pixelShaderCount; i++)
	{
		if(m_PixelShaderList[i].m_DefineList == defineList)
			return i;
	}
	
	m_PixelShaderList.PushBack();

	C_Shader& newShader = m_PixelShaderList[pixelShaderCount];
	
	SetupShader(newShader, defineList, "PS_");

	return pixelShaderCount;
}

void C_ShaderRegistry::SetupShader( C_Shader& shader, C_DefineList& defineList, const Util::C_String& prefix )
{
	shader.m_DefineList = defineList;

	shader.m_TextId = prefix;
	S32 defineCount = shader.m_DefineList.Size();
	for(S32 i=0; i<defineCount; i++)
	{
		S32 defineIndex = shader.m_DefineList.m_DefineList[i];

		C_Define& define = m_pShaderConfig->m_DefineList[defineIndex];
		shader.m_TextId << define.m_TextId;
	}
}

void C_ShaderRegistry::CreateGameRegistryAxis( Util::C_StringTableBuilder& stringTableBuilder, ShaderRegistry::C_Registry& reg, ShaderRegistry::C_TableAxis& regTableAxis, C_ShaderTableAxis& srcTableAxis )
{
	regTableAxis.m_Size = srcTableAxis.m_Size;

	const S32 combinationCount = srcTableAxis.m_AxisCombinationList.Size();
	for(S32 combinationNo=0; combinationNo<combinationCount; combinationNo++)
	{
		C_AxisCombination& srcCombination = srcTableAxis.m_AxisCombinationList[combinationNo];
		const S32 nameId = stringTableBuilder.AddString(srcCombination.m_Name);
		
		regTableAxis.m_AxisCombinationList.PushBack( ShaderRegistry::C_AxisCombination(nameId) );
		ShaderRegistry::C_AxisCombination& dstCombination = regTableAxis.m_AxisCombinationList[combinationNo];
		
		dstCombination.m_IndexStart = srcCombination.m_IndexStart;

		S32 enumCount = srcCombination.m_EnumList.Size();
		for(S32 enumNo=0; enumNo<enumCount; enumNo++)
		{
			S32 enumType = srcCombination.m_EnumList[enumNo].m_EnumType;
			S32 multiplier = srcCombination.m_EnumList[enumNo].m_IndexMultiplier;
			dstCombination.m_EnumList.PushBack( ShaderRegistry::C_AxisCombinationEnum(enumType, multiplier) );
		}
	}

	const S32 indexCount = srcTableAxis.m_IndexNameList.Size();
	regTableAxis.m_IndexNameIdList.Resize(indexCount);

	for(S32 indexNo=0; indexNo<indexCount; indexNo++)
	{
		regTableAxis.m_IndexNameIdList[indexNo] = stringTableBuilder.AddString( srcTableAxis.m_IndexNameList[indexNo] );
	}
}

void C_ShaderRegistry::CreateGameRegistry( ShaderRegistry::C_Registry& reg )
{
	Util::C_StringTableBuilder stringTableBuilder;

	const S32 defineCount = m_pShaderConfig->m_DefineList.Size();
	for(S32 i=0; i<defineCount; i++)
	{
		const S32 nameId = stringTableBuilder.AddString( m_pShaderConfig->m_DefineList[i].m_Name );
		const S32 textId = stringTableBuilder.AddString( m_pShaderConfig->m_DefineList[i].m_TextId );
		reg.m_DefineList.PushBack( ShaderRegistry::C_Define( nameId, textId ) );
	}

	const S32 enumCount = m_ShaderEnumList.Size();
	for(S32 i=0; i<enumCount; i++)
	{
		const S32 nameId = stringTableBuilder.AddString( m_ShaderEnumList[i].m_Name );		
		reg.m_AxisEnumList.PushBack( ShaderRegistry::C_AxisEnum( nameId ) );
	}

	for(S32 i=0; i<enumCount; i++)
	{
		ShaderRegistry::C_AxisEnum& regEnum = reg.m_AxisEnumList[i];

		const S32 itemCount = m_ShaderEnumList[i].m_ItemList.Size();
		for(S32 j=0; j<itemCount; j++)
		{
			S32 itemNameId = stringTableBuilder.AddString( m_ShaderEnumList[i].m_ItemList[j].m_Name );
			regEnum.m_ItemNameIdList.PushBack(itemNameId);
		}
	}

	CreateGameRegistryAxis(stringTableBuilder, reg, reg.m_IngameAxis, m_ShaderTableAxisList[m_IngameAxis] );
	CreateGameRegistryAxis(stringTableBuilder, reg, reg.m_OfflineAxis, m_ShaderTableAxisList[m_OfflineAxis] );
	

	const S32 vertexShaderCount = m_VertexShaderList.Size();
	for(S32 i=0; i<vertexShaderCount;i++)
	{
		const S32 nameId = stringTableBuilder.AddString( m_VertexShaderList[i].GetName() );
		reg.m_VertexShaderList.PushBack( ShaderRegistry::C_Shader(nameId) );	

		reg.m_VertexShaderList[i].m_DefineBits = m_VertexShaderList[i].m_DefineList.GetDefineBits();
	}

	const S32 pixelShaderCount = m_PixelShaderList.Size();
	for(S32 i=0; i<pixelShaderCount;i++)
	{
		const S32 nameId = stringTableBuilder.AddString( m_PixelShaderList[i].GetName() );
		reg.m_PixelShaderList.PushBack( ShaderRegistry::C_Shader(nameId) );	

		reg.m_PixelShaderList[i].m_DefineBits = m_PixelShaderList[i].m_DefineList.GetDefineBits();
	}

	const S32 tableItemCount = m_TableItemList.Size();
	for(S32 i=0; i<tableItemCount; i++)
	{
		const S32 vertexShaderIndex = m_TableItemList[i].m_VertexShaderIndex;
		const S32 pixelShaderIndex = m_TableItemList[i].m_PixelShaderIndex;
		reg.m_TableItemList.PushBack( ShaderRegistry::C_TableItem( vertexShaderIndex, pixelShaderIndex ) );

		ShaderRegistry::C_TableItem& tableItem = reg.m_TableItemList[i];
		
		tableItem.m_DefineBits = m_VertexShaderList[vertexShaderIndex].m_DefineList.GetDefineBits() |
								 m_PixelShaderList[pixelShaderIndex].m_DefineList.GetDefineBits();
	}


	reg.m_StringTable.Build(stringTableBuilder);
}

} // namespace ShaderCfg