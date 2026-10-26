
#include "stdafx.h"

#include <converter_shared/config/PolyflagConfig.h>
#include <converter_shared/helpers/XmlHelpers.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <file_tag_db/FileTagDB.h>
#include <AftkUt/AftkUt.h>

// TODO: The LayerFlag would make more sense as a name flag with a source option. It should include the "Prefix/Infix/Postfix" options of the objectflag config too.
// TODO: if the layerflags are converted, the polyflag config could do everything that the objectflag config does making the objectflag system redundant.
// TODO: we could do with an include system or something to configure these to allow what nightwing do with the model names vs sharing the polyflag config between tools

namespace Polyflags
{

// Static class members.	
Btl::ChainedHashMap<C_Polyflags::PolyflagCacheKey, Btl::Vector<C_Value> >	C_Polyflags::m_PolyflagCache;

/// Hashing function for a polyflag cache key.
U32 Hash(const C_Polyflags::PolyflagCacheKey& rkKey)
{
	U32 iHash = 0;
	iHash = Btl::Hash(reinterpret_cast<const void*>(rkKey.m_pSceneNode), iHash);
	iHash = Btl::Hash(reinterpret_cast<const void*>(rkKey.m_pBlindDataResource), iHash);
	iHash = Btl::Hash(reinterpret_cast<const void*>(rkKey.m_pMaterialNode), iHash);
	return iHash;
}

/// Check for equality with another key.
const bool operator == (const C_Polyflags::PolyflagCacheKey& rkFirst, const C_Polyflags::PolyflagCacheKey& rkSecond)
{
	return ((rkFirst.m_pSceneNode == rkSecond.m_pSceneNode)					&& 
		(rkFirst.m_pBlindDataResource == rkSecond.m_pBlindDataResource)	&& 
		(rkFirst.m_pMaterialNode == rkSecond.m_pMaterialNode));
}

Util::C_String GetSourceString( const E_Source source )
{
	switch( source )
	{
	default:
	case E_Source_Polyflags: return "PolyFlags";
	case E_Source_TextureTags: return "TextureTags";
	case E_Source_NodeFlags: return "NodeFlags";
	}
}

E_Source GetSourceFromString( const Util::C_String& source )
{
	for(S32 i=0; i<E_Source_Count; i++)
	{
		if( stricmp(source.Chars(), GetSourceString((E_Source)i)) == 0)
		{
			return (E_Source)i;
		}

	}

	return E_Source_Default;
}

C_Define::C_Define(const Util::C_String& name, const Util::C_String& value)
	: m_Name(name)
	, m_Value(value)
{
}

void C_DefineList::AddDefine(const Util::C_String& name, const Util::C_String& value)
{
	Util::C_String preprocessedValue = Process(value);
	LOGDEBUG( Tools::g_log, "Adding Polyflag Define '%s' = '%s' = '%s'", name.Chars(), value.Chars(), preprocessedValue.Chars() );
	m_DefineList.PushBack( C_Define(name, preprocessedValue) );
}

S32 DefineSearch( const Util::C_String& src, const Util::C_String& search, const S32 index )
{
	const S32 length = src.Length();
	const S32 searchLength = search.Length();

	S32 found = src.Search(search, index);

	if(found == -1)
		return -1;

	if(found + searchLength >= length)
		return found;

	char ch = src[found + searchLength];

	switch(ch)
	{
		case ' ': return found;
		case '\t': return found;
		default: break;
	}

	return -1;
}


Util::C_String DefineReplace( const Util::C_String& src, const Util::C_String& search, const Util::C_String& dst )
{
	Util::C_String result;
	const S32 length = src.Length();
	const S32 searchLength = search.Length();
	if (0 == searchLength)
	{
		return search;
	}
	S32 index = 0;
	while ( index < length )
	{
		S32 pos = DefineSearch( src, search, index );
		//S32 pos = src.Search( search, index );
		if ( pos >= 0 )
		{
			if ( pos > index )
			{
				result = result + src( index, pos );
			}
			result += dst;
			index = pos + searchLength;
		}
		else
		{
			return result + src( index, length );
		}
	}
	return result;
}

Util::C_String C_DefineList::Process(const Util::C_String& input) const
{
	Util::C_String result = input;
	for(S32 i=0 ;i<m_DefineList.Size(); i++)
	{
		const C_Define& define = m_DefineList[i];
		result = DefineReplace( result, define.m_Name, define.m_Value );
	}
	return result;
}

U32 EvaluateNumericExpression(const Util::C_String& input)
{
	Ut::Vector<Util::C_String> values;
	input.Tokenise( values, '|' );

	U32 result = 0;
	for(S32 i=0; i<values.Size(); i++)
	{
		U32 value = atoi( values[i].Chars() );
		result |= value;
	}
	return result;
}

const U32 EvaluateValue( const Util::C_String& input, const C_DefineList& defineList )
{
	Util::C_String preprocessedString = defineList.Process( input );

	return EvaluateNumericExpression(preprocessedString.Chars());
}

bool ParseSourceXml(Sys::C_XmlDomBaseNode * const pNode, I_Source** ppSource, const C_DefineList& defineList)
{
	if( _stricmp( pNode->GetName().Chars(), "Container") == 0 )
	{
		I_Source* pNewSource = new C_Container;
		*ppSource = pNewSource;

		bool success = pNewSource->ParseXml( pNode, defineList );
		if(!success) return false;
	}
	else
	if( _stricmp( pNode->GetName().Chars(), "Flag") == 0 )
	{
		I_Source* pNewSource = new C_Flag;
		*ppSource = pNewSource;

		bool success = pNewSource->ParseXml( pNode, defineList );
		if(!success) return false;
	}
	else
	if( _stricmp( pNode->GetName().Chars(), "Enum") == 0 )
	{
		I_Source* pNewSource = new C_Enum;
		*ppSource = pNewSource;

		bool success = pNewSource->ParseXml( pNode, defineList );
		if(!success) return false;
	}
	else
	if( _stricmp( pNode->GetName().Chars(), "LayerFlag") == 0 )
	{
		I_Source* pNewSource = new C_LayerFlag;
		*ppSource = pNewSource;

		bool success = pNewSource->ParseXml( pNode, defineList );
		if(!success) return false;
	}

	return true;
}

// C_PolyflagDataSource
C_PolyflagDataSource::C_PolyflagDataSource()
	: m_pSceneNode(NULL)
	, m_pBlindDataResource(NULL)
	, m_pMaterialNode(NULL)
{
}

C_PolyflagDataSource::C_PolyflagDataSource( const Aftk::C_SceneNode* pMeshNode, const Aftk::C_BlindDataResource* pBlindDataResource, const Aftk::C_MaterialNode* pMaterialNode )
	: m_pSceneNode(pMeshNode)
	, m_pBlindDataResource(pBlindDataResource)
	, m_pMaterialNode(pMaterialNode)
{
}

// I_Source
I_Source::I_Source()
{
}

I_Source::~I_Source()
{
}

void I_Source::Dump(const Util::C_String& prefix) 
{
}

void I_Source::GetFlagNames( C_PolyflagNames& polyFlagNames ) const
{
}

// C_Flag

C_Flag::C_Flag()
	: m_Source( E_Source_Default )
	, m_Id("<NotSet>")
	, m_Value(0)
{
}

C_Flag::~C_Flag()
{
}

bool C_Flag::ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList )
{
	if(pNode->GetType() != Sys::E_XmlDomNodeType_Element)
		return false;
	
	Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pNode;

	// read attributes
	const Sys::C_XmlAttributeList& attributes = pElement->GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "id" ) == 0)
			{
				m_Id = pAttribute->value;
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "source" ) == 0)
			{
				m_Source = GetSourceFromString( pAttribute->value );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "value" ) == 0)
			{
				m_Value = EvaluateValue( pAttribute->value, defineList );
			}
			else
			{
				LOGWARN( Tools::g_log, "Unknown attribute '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}
		}
	}

	// read child nodes
	const Sys::C_XmlDomBaseNode::Array& childArray = pNode->GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			{
				LOGWARN( Tools::g_log, "Unrecognised XML node '%s'", pChildNode->GetName().Chars());
			}
		}
	}		

	return true;
}


C_Value C_Flag::GetValue( const C_PolyflagDataSource& dataSource ) const
{
	switch(m_Source)
	{
		default:
		case E_Source_Polyflags: return GetValue( dataSource.m_pBlindDataResource );
		case E_Source_TextureTags: return GetValue( dataSource.m_pMaterialNode );
		case E_Source_NodeFlags: return GetValue( dataSource.m_pSceneNode );
	}
}

C_Value C_Flag::GetValue( const Aftk::C_BlindDataResource* pBlindDataResourc ) const
{
	if(pBlindDataResourc == NULL)
		return 0;

	const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResourc->GetContent();
	for (U32 i = 0; i < aftkBlindDataNode.GetNumAttributes(); i++) 
	{
		const Aftk::C_GenericAttribute& aftkAttribute = aftkBlindDataNode.GetAttribute( i ); 

		const char* attrName = aftkAttribute.GetName().c_str();

		//LOGDEBUG( Tools::g_log, "   %s", attrName );

		if( _stricmp( m_Id.Chars(), attrName ) == 0)
		{
			return m_Value;
		}
	}

	return 0;
}

C_Value C_Flag::GetValue( const Aftk::C_MaterialNode* pMaterialNode ) const
{
	if( pMaterialNode == NULL )
		return 0;

	U32 shaderImageNodeCount = pMaterialNode->GetNumShaderImageNodes();

	// only check first texture on node
	if( shaderImageNodeCount > 1 )
		shaderImageNodeCount = 1;

	for(U32 shaderImageNodeIndex = 0; shaderImageNodeIndex < shaderImageNodeCount; shaderImageNodeIndex++)
	{
		const Aftk::C_ShaderImageNode* pImageNode = pMaterialNode->GetShaderImageNodePtr(shaderImageNodeIndex);
		if (!pImageNode)
			continue;

		std::string fileName;
		pImageNode->GetFilePlug().GetValue(fileName);

		const FileTag::C_Tag* pTag = FileTag::C_TagDBManager::Instance().GetTagPtr( fileName.c_str() );
		
		if (!pTag)
			continue;

		bool bValue;
		if (pTag->GetAttributeValue(m_Id, bValue))
		{
			if( bValue )
			{
				return m_Value;
			}
		}
	}

	return 0;
}

C_Value C_Flag::GetValue( const Aftk::C_SceneNode* pSceneNode ) const
{
	if( pSceneNode == NULL )
		return 0;

	AftkUt::C_NodeFlagParser nodeFlagParser(*pSceneNode);

	bool isSet;
	nodeFlagParser.GetParameterValue(m_Id.Chars(), isSet);
	if( isSet )
	{
		return m_Value;
	}	

	return 0;
}

void C_Flag::Dump(const Util::C_String& prefix)
{
	LOGDEBUG( Tools::g_log, "%s[%s Flag] '%s' = %d", prefix.Chars(), GetSourceString(m_Source).Chars(), m_Id.Chars(), m_Value );
}

void C_Flag::GetFlagNames( C_PolyflagNames& polyFlagNames ) const
{
	polyFlagNames.PushBack( m_Id );
}

// C_EnumItem

C_EnumItem::C_EnumItem()
	: m_Index(0)
	, m_Id("<NotSet>")
	, m_Value(0)
{
}

C_EnumItem::~C_EnumItem()
{
}

bool C_EnumItem::ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList )
{
	if(pNode->GetType() != Sys::E_XmlDomNodeType_Element)
		return false;
	
	Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pNode;

	// read attributes
	const Sys::C_XmlAttributeList& attributes = pElement->GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "index" ) == 0)
			{
				m_Index = EvaluateValue( pAttribute->value, defineList );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "id" ) == 0)
			{
				m_Id = pAttribute->value;
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "value" ) == 0)
			{
				m_Value = EvaluateValue( pAttribute->value, defineList );
			}
			else
			{
				LOGWARN( Tools::g_log, "Unknown attribute '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}
		}
	}

	// read child nodes
	const Sys::C_XmlDomBaseNode::Array& childArray = pNode->GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			{
				LOGWARN( Tools::g_log, "Unrecognised XML node '%s'", pChildNode->GetName().Chars());
			}
		}
	}		

	return true;
}

void C_EnumItem::Dump( const Util::C_String& prefix )
{
	LOGDEBUG( Tools::g_log, "%sEnumItem[%d:%s] = %d", prefix.Chars(), m_Index, m_Id.Chars(), m_Value );
}

// C_Enum
C_Enum::C_Enum()
	: m_Source( E_Source_Default )
	, m_Id("<NotSet>")
{
}

C_Enum::~C_Enum()
{
}

bool C_Enum::ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList )
{
	if(pNode->GetType() != Sys::E_XmlDomNodeType_Element)
		return false;
	
	Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pNode;

	// read attributes
	const Sys::C_XmlAttributeList& attributes = pElement->GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "id" ) == 0)
			{
				m_Id = pAttribute->value;
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "source" ) == 0)
			{
				m_Source = GetSourceFromString( pAttribute->value );
			}
			else
			{
				LOGWARN( Tools::g_log, "Unknown attribute '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}
		}
	}

	// read child nodes
	const Sys::C_XmlDomBaseNode::Array& childArray = pNode->GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			if( _stricmp( pChildNode->GetName().Chars(), "Value") == 0 )
			{
				// add new enum item
				const S32 enumItemCount = m_EnumItemList.Size();
				m_EnumItemList.Resize( enumItemCount + 1 );
				C_EnumItem& newEnumItem = m_EnumItemList[enumItemCount];

				bool success = newEnumItem.ParseXml( pChildNode, defineList );
				if(!success) return false;
			}
			else
			{
				LOGWARN( Tools::g_log, "Unrecognised XML node '%s'", pChildNode->GetName().Chars());
			}
		}
	}		

	return true;
}

C_Value C_Enum::GetValue( const C_PolyflagDataSource& dataSource ) const
{
	switch(m_Source)
	{
		default:
		case E_Source_Polyflags: return GetValue( dataSource.m_pBlindDataResource );
		case E_Source_TextureTags: return GetValue( dataSource.m_pMaterialNode );
		case E_Source_NodeFlags: return GetValue( dataSource.m_pSceneNode );
	}
}

C_Value C_Enum::GetValue( const Aftk::C_BlindDataResource* pBlindDataResource ) const
{
	if(pBlindDataResource == NULL)
		return 0;

	const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();
	for (U32 i = 0; i < aftkBlindDataNode.GetNumAttributes(); i++) 
	{
		const Aftk::C_GenericAttribute& aftkAttribute = aftkBlindDataNode.GetAttribute( i ); 

		const char* attrName = aftkAttribute.GetName().c_str();

		//LOGDEBUG( Tools::g_log, "   %s", attrName );

		if( _stricmp( m_Id.Chars(), attrName ) == 0)
		{
			if( aftkAttribute.GetDataType() == Aftk::E_GenericAttributeType_U32 ) 
			{
				// get value from enum
				U32 enumValue; //enum value as unsigned int
				//AFTK_ASSERT(aftkAttribute.GetDataType() == Aftk::E_GenericAttributeType_U32);
				aftkAttribute.GetValue(enumValue);

				// find enum value
				const S32 enumItemCount = m_EnumItemList.Size();
				for(S32 enumItemNo=0; enumItemNo<enumItemCount; enumItemNo++)
				{
					const C_EnumItem& currEnumItem = m_EnumItemList[enumItemNo];
					if( currEnumItem.m_Index == enumValue )
					{
						return currEnumItem.m_Value;
						//LOGDEBUG( Tools::g_log, "      found %d |= %d", currEnumItem.m_Index, currEnumItem.m_Value );
					}
				}
			}
			else
			{
				std::string enumValue;

				//AFTK_ASSERT(aftkAttribute.GetDataType() == Aftk::E_GenericAttributeType_String);
				aftkAttribute.GetValue(enumValue);

				// find enum value
				const S32 enumItemCount = m_EnumItemList.Size();
				for(S32 enumItemNo=0; enumItemNo<enumItemCount; enumItemNo++)
				{
					const C_EnumItem& currEnumItem = m_EnumItemList[enumItemNo];
					if( _stricmp( currEnumItem.m_Id.Chars(), enumValue.c_str() ) == 0 )
					{
						return currEnumItem.m_Value;
						//LOGDEBUG( Tools::g_log, "      found %s |= %d", currEnumItem.m_Id.Chars(), currEnumItem.m_Value );
					}
				}
			}
		}
	}

	return 0;
}

C_Value C_Enum::GetValue( const Aftk::C_MaterialNode* pMaterialNode ) const
{
	if( pMaterialNode == NULL )
		return 0;

	U32 shaderImageNodeCount = pMaterialNode->GetNumShaderImageNodes();

	// only check first texture on node
	if( shaderImageNodeCount > 1 )
		shaderImageNodeCount = 1;

	for(U32 shaderImageNodeIndex = 0; shaderImageNodeIndex < shaderImageNodeCount; shaderImageNodeIndex++)
	{
		const Aftk::C_ShaderImageNode* pImageNode = pMaterialNode->GetShaderImageNodePtr(shaderImageNodeIndex);
		if (!pImageNode)
			continue;

		std::string fileName;
		pImageNode->GetFilePlug().GetValue(fileName);

		const FileTag::C_Tag* pTag = FileTag::C_TagDBManager::Instance().GetTagPtr( fileName.c_str() );
		
		if (!pTag)
			continue;

		Util::C_String sValue;
		if (pTag->GetAttributeValue(m_Id, sValue))
		{
			// find enum value
			const S32 enumItemCount = m_EnumItemList.Size();
			for(S32 enumItemNo=0; enumItemNo<enumItemCount; enumItemNo++)
			{
				const C_EnumItem& currEnumItem = m_EnumItemList[enumItemNo];
				if( _stricmp( currEnumItem.m_Id.Chars(), sValue.Chars() ) == 0 )
				{
					return currEnumItem.m_Value;
					//LOGDEBUG( Tools::g_log, "      found %s |= %d", currEnumItem.m_Id.Chars(), currEnumItem.m_Value );
				}
			}
		}
	}

	return 0;
}

C_Value C_Enum::GetValue( const Aftk::C_SceneNode* pSceneNode ) const
{
	if( pSceneNode == NULL )
		return 0;

	AftkUt::C_NodeFlagParser nodeFlagParser(*pSceneNode);

	std::string stdValue;
	AftkUt::E_ParamType paramType = AftkUt::E_ParamType_Unknown;
	bool isSet = nodeFlagParser.GetParameterValue(m_Id.Chars(), stdValue, &paramType);
	Util::C_String sValue = stdValue.c_str();

	if( isSet )
	{
		if( paramType == AftkUt::E_ParamType_Enum )
		{
			// find enum value
			const S32 enumItemCount = m_EnumItemList.Size();
			for(S32 enumItemNo=0; enumItemNo<enumItemCount; enumItemNo++)
			{
				const C_EnumItem& currEnumItem = m_EnumItemList[enumItemNo];
				if( _stricmp( currEnumItem.m_Id.Chars(), sValue.Chars() ) == 0 )
				{
					return currEnumItem.m_Value;
					//LOGDEBUG( Tools::g_log, "      found %s |= %d", currEnumItem.m_Id.Chars(), currEnumItem.m_Value );
				}
			}
		}	
	}

	return 0;
}


void C_Enum::Dump(const Util::C_String& prefix)
{
	LOGDEBUG( Tools::g_log, "%s[%s Enum] '%s'", prefix.Chars(), GetSourceString( m_Source ).Chars(), m_Id.Chars() );

	for(Btl::Vector<C_EnumItem>::Iterator i = m_EnumItemList.Begin(); i < m_EnumItemList.End(); ++i)
	{
		(*i).Dump( prefix + "   ");
	}
}

// C_LayerFlag
C_LayerFlag::C_LayerFlag()
	: m_Id("<NotSet>")
	, m_Value(0)
{
}

C_LayerFlag::~C_LayerFlag()
{
}

bool C_LayerFlag::ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList )
{
	if(pNode->GetType() != Sys::E_XmlDomNodeType_Element)
		return false;
	
	Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pNode;

	// read attributes
	const Sys::C_XmlAttributeList& attributes = pElement->GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "id" ) == 0)
			{
				m_Id = pAttribute->value;
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "value" ) == 0)
			{
				m_Value = EvaluateValue( pAttribute->value, defineList );
			}
			else
			{
				LOGWARN( Tools::g_log, "Unknown attribute '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}
		}
	}

	// read child nodes
	const Sys::C_XmlDomBaseNode::Array& childArray = pNode->GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			{
				LOGWARN( Tools::g_log, "Unrecognised XML node '%s'", pChildNode->GetName().Chars());
			}
		}
	}		

	return true;
}

C_Value C_LayerFlag::GetValue( const C_PolyflagDataSource& dataSource ) const
{
	if( AftkHelpers::InNamedLayer((Aftk::C_SceneNode*)dataSource.m_pSceneNode, m_Id) )
	{
		return m_Value;
	}

	return 0;
}

void C_LayerFlag::Dump(const Util::C_String& prefix)
{
	LOGDEBUG( Tools::g_log, "%sLayer Flag '%s' = %d", prefix.Chars(), m_Id.Chars(), m_Value );
}

// C_Container

C_Container::C_Container()
	: m_Mode( E_Mode_Combine )
{
}

C_Container::~C_Container()
{
	for(Btl::Vector<I_Source*>::Iterator i = m_pSourceList.Begin(); i < m_pSourceList.End(); ++i)
	{
		I_Source*& pSource = (*i);
		SAFE_DELETE_PTR( pSource );
	}
}

bool C_Container::ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList )
{
	if(pNode->GetType() != Sys::E_XmlDomNodeType_Element)
		return false;
	
	Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pNode;

	// read attributes
	const Sys::C_XmlAttributeList& attributes = pElement->GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "Mode" ) == 0)
			{
				if( _stricmp( pAttribute->value.Chars(), "Combine" ) == 0)
				{
					m_Mode = E_Mode_Combine;
				}
				else
				if( _stricmp( pAttribute->value.Chars(), "SelectFirst" ) == 0)
				{
					m_Mode = E_Mode_SelectFirst;
				}
				else
				if( _stricmp( pAttribute->value.Chars(), "MaskOut" ) == 0)
				{
					m_Mode = E_Mode_MaskOut;
				}
				else
				{
					LOGWARN( Tools::g_log, "Unknown mode '%s'", pAttribute->value.Chars());
				}
			}
			else
			{
				LOGWARN( Tools::g_log, "Unknown attribute '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}
		}
	}

	// read child nodes
	const Sys::C_XmlDomBaseNode::Array& childArray = pNode->GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			bool success = true;
			I_Source* pSource = NULL;
			success = ParseSourceXml( pChildNode, &pSource, defineList );
			if(pSource)
			{
				m_pSourceList.PushBack( pSource );
			}
			else
			{
				LOGWARN( Tools::g_log, "Unrecognised XML node '%s'", pChildNode->GetName().Chars());
			}
		}
	}		

	return true;
}

C_Value C_Container::GetValue( const C_PolyflagDataSource& dataSource ) const
{
	C_Value result = 0;
	switch( m_Mode )
	{
		case E_Mode_Combine:
		{
			for(Btl::Vector<I_Source*>::ConstIterator i = m_pSourceList.Begin(); i < m_pSourceList.End(); ++i)
			{
				result |= (*i)->GetValue( dataSource );
			}
		}
		break;
		case E_Mode_SelectFirst:
		{
			for(Btl::Vector<I_Source*>::ConstIterator i = m_pSourceList.Begin(); i < m_pSourceList.End(); ++i)
			{
				result = (*i)->GetValue( dataSource );
				if( result != 0 )
					break;
			}
		}
		break;
		case E_Mode_MaskOut:
		{
			bool firstItem = true;
			for(Btl::Vector<I_Source*>::ConstIterator i = m_pSourceList.Begin(); i < m_pSourceList.End(); ++i)
			{
				const U32 iValue = (*i)->GetValue( dataSource );
				if(firstItem)
				{
					result |= iValue;
				}
				else
				{
					result &= ~iValue;
				}

				firstItem = false;
			}
		}
		break;
	}

	return result;
}

void C_Container::GetFlagNames( C_PolyflagNames& polyFlagNames ) const
{
	for(Btl::Vector<I_Source*>::ConstIterator i = m_pSourceList.Begin(); i < m_pSourceList.End(); ++i)
	{
		(*i)->GetFlagNames( polyFlagNames );
	}
}

void C_Container::Dump(const Util::C_String& prefix)
{
	Util::C_String mode;
	switch(m_Mode)
	{
		case E_Mode_Combine:
			mode = "Combine";
		break;
		case E_Mode_SelectFirst:
			mode = "SelectFirst";
		break;
		case E_Mode_MaskOut:
			mode = "MaskOut";
		break;
	}

	LOGDEBUG( Tools::g_log, "%s[Container] %s", prefix.Chars(), mode.Chars() );
	for(Btl::Vector<I_Source*>::Iterator i = m_pSourceList.Begin(); i < m_pSourceList.End(); ++i)
	{
		(*i)->Dump( prefix + "   " );
	}
}


// C_Item

C_Item::C_Item()
	: m_pSource(NULL)
	, m_Name("<Unknown>")
{
}

C_Item::C_Item( const Util::C_String& name )
	: m_pSource(NULL)
	, m_Name(name)
{
}

C_Item::~C_Item()
{
	SAFE_DELETE_PTR( m_pSource );
}

bool C_Item::ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList )
{
	if(pNode->GetType() != Sys::E_XmlDomNodeType_Element)
		return false;
	
	Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pNode;

	// read attributes
	const Sys::C_XmlAttributeList& attributes = pElement->GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "Name" ) == 0)
			{
				m_Name = pAttribute->value;
			}
			else
			{
				LOGWARN( Tools::g_log, "Unknown attribute '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}
		}
	}

	// read child nodes
	const Sys::C_XmlDomBaseNode::Array& childArray = pNode->GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			if( m_pSource != NULL )
			{
				LOGWARN( Tools::g_log, "Source already set for this item '%s'", m_Name.Chars() );
			}
			else
			{
				bool success = true;
				success = ParseSourceXml( pChildNode, &m_pSource, defineList );
				if(m_pSource == NULL)
				{
					LOGWARN( Tools::g_log, "Unrecognised XML node '%s'", pChildNode->GetName().Chars());
				}
			}
		}
	}		

	return true;
}

const Util::C_String& C_Item::GetName() const
{
	return m_Name;
}

void C_Item::GetFlagNames( C_PolyflagNames& polyFlagNames ) const
{
	if( m_pSource )
	{
		m_pSource->GetFlagNames( polyFlagNames );
	}
}

void C_Item::Dump()
{
	LOGDEBUG( Tools::g_log, "   *Item '%s'", GetName().Chars() );

	if( m_pSource )
	{
		m_pSource->Dump( "      " );
	}
}

C_Value C_Item::GetValue( const C_PolyflagDataSource& dataSource ) const
{
	if(m_pSource == NULL)
		return 0;

	return m_pSource->GetValue( dataSource );
}

// C_Config

C_Config::C_Config()
{
}

C_Config::~C_Config()
{
}

void C_Config::Dump()
{
	LOGDEBUG( Tools::g_log, "<pre><tt>" );
	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "Polyflags::C_Config:" );

	for(Btl::Vector<C_Item>::Iterator i = m_ItemList.Begin(); i < m_ItemList.End(); ++i)
	{
		(*i).Dump();
	}

	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "</tt></pre>" );
}

bool C_Config::ReadXml( const Util::C_String& xmlFilePath )
{
	bool success = true;
	LOGDEBUG(Tools::g_log, "Parsing: '%s'\n", xmlFilePath.Chars());
	Sys::C_XmlDom dom;

	success = dom.Read(xmlFilePath.Chars());
	if(!success)
	{
		LOGERROR(Tools::g_log, " Problem reading '%s'\n", xmlFilePath.Chars());
		return false;
	}

	success = ParseXml( xmlFilePath, dom );
	if (!success)
	{
		LOGERROR(Tools::g_log, " Problem parsing '%s'\n", xmlFilePath.Chars());
		return false;
	}

	return true;
}

bool C_Config::RegisterItems( Btl::Vector<Util::C_String> itemNameList )
{
	const S32 itemCount = itemNameList.Size();

	m_ItemList.Reserve( itemCount );

	for(S32 i=0; i<itemCount; i++)
	{
		m_ItemList.PushBack( C_Item( itemNameList[i] ) );
	}

	return true;
}

C_Item* C_Config::LookupItem( const Util::C_String& itemName )
{
	S32 itemCount = m_ItemList.Size();
	for(S32 i=0; i<itemCount; i++)
	{
		if(_stricmp( m_ItemList[i].GetName().Chars(), itemName.Chars()) == 0)
		{
			return &m_ItemList[i];
		}
	}

	return NULL;
}

S32 C_Config::GetItemCount() const
{
	return m_ItemList.Size();
}

C_Item& C_Config::GetItem( const S32 itemIndex )
{
	return m_ItemList[itemIndex];
}

const C_Item& C_Config::GetItem( const S32 itemIndex ) const
{
	return m_ItemList[itemIndex];
}

bool C_Config::ParseXml( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom )
{
	const Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr();

	if(!pRootNode)
		return false;

	return ParseXml( pRootNode );
}

bool C_Config::ParseXml( const Sys::C_XmlDomBaseNode* pRootNode )
{
	C_DefineList defineList;

	const Sys::C_XmlDomBaseNode::Array& childArray = pRootNode->GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			if( _stricmp( pChildNode->GetName().Chars(), "Define") == 0 )
			{
				if(pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

					const Sys::C_XmlAttributeList& attributes = pChildElement->GetAttributes();

					const char* defineName = attributes.GetValueByName( "Name" );
					const char* defineValue = attributes.GetValueByName( "Value" );
					if((defineName!=NULL) && (defineValue!=NULL))
					{
						defineList.AddDefine(defineName, defineValue);
					}
				}
			}
			else
			if( _stricmp( pChildNode->GetName().Chars(), "Item") == 0 )
			{
				// find item with name
				{
					if(pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
					{
						Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

						const Sys::C_XmlAttributeList& attributes = pChildElement->GetAttributes();

						const char* itemName = attributes.GetValueByName( "Name" );
						if(itemName)
						{
							C_Item* pItem = LookupItem( itemName );
							if( pItem )
							{
								bool success = pItem->ParseXml( pChildNode, defineList );
								if(!success) return false;
							}
							else
							{
								LOGWARN( Tools::g_log, "Polyflag Item '%s' not found. Was it registered?", itemName);
							}
						}
						else
						{
							LOGWARN( Tools::g_log, "Could not find Polyflag Item name");
						}
					}
				}
			}
			else
			{
				LOGWARN( Tools::g_log, "Unrecognised XML node '%s'", pChildNode->GetName().Chars());
			}
		}
	}		

	return true;
}

// C_Polyflags 

C_Polyflags::C_Polyflags()
{
}

C_Polyflags::C_Polyflags( const C_Config& config, const Aftk::C_Polygon* pPolygon )
{
	const Aftk::C_SceneMeshNode* pSceneMeshNode = pPolygon->GetMeshNodePtr();
	const Aftk::C_Scene* pAftk = pSceneMeshNode->GetScenePtr();
	S16 blindDataMappingIndex = pPolygon->GetBlindDataMappingIndex();

	// Get Blind Data Resource
	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);

	// Get Material Node
	S16 materialIndex = pPolygon->GetMaterialMappingIndex();
	const Aftk::C_SharedResource<Aftk::C_MaterialNode> *pMaterialResource = pSceneMeshNode->GetPolygonMaterialMappingPtr( materialIndex );

	const Aftk::C_MaterialNode* pMaterialNode = NULL;
	if(pMaterialResource != NULL)
		pMaterialNode = &pMaterialResource->GetContent();

	SetValues( config, C_PolyflagDataSource(pSceneMeshNode, pBlindDataResource, pMaterialNode) );

	return;
}

C_Polyflags::C_Polyflags( const C_Config& config, const Aftk::C_SceneNode* pSceneNode )
{
	SetValues( config, C_PolyflagDataSource(pSceneNode, NULL, NULL) );
}

C_Polyflags::~C_Polyflags()
{
}

void C_Polyflags::SetValues( const C_Config& config, const C_PolyflagDataSource& dataSource )
{
#if USEPOLYFLAG_CACHE
	const PolyflagCacheKey cacheKey(dataSource.m_pSceneNode, dataSource.m_pBlindDataResource, dataSource.m_pMaterialNode);

	Btl::ChainedHashMap<C_Polyflags::PolyflagCacheKey, Btl::Vector<C_Value> >::Iterator result = m_PolyflagCache.Find(cacheKey);
	if(result != m_PolyflagCache.End())
	{
		// Cache hit.
		m_ValueList = result->GetValue();
		return;
	}
	else
	{
		// Cache miss =(.
		S32 itemCount = config.GetItemCount();
		m_ValueList.Resize(itemCount);

		for(S32 itemNo=0; itemNo<itemCount; itemNo++)
			m_ValueList[itemNo]= config.GetItem(itemNo).GetValue(dataSource);

		m_PolyflagCache.Insert(cacheKey, m_ValueList);
	}
#else

	S32 itemCount = config.GetItemCount();
	m_ValueList.Resize(itemCount);

	for(S32 itemNo=0; itemNo<itemCount; itemNo++)
		m_ValueList[itemNo]= config.GetItem(itemNo).GetValue(dataSource);
#endif
}

C_Value C_Polyflags::GetValue( const S32 index )
{
	return m_ValueList[index];
}

} // namespace Polyflags
