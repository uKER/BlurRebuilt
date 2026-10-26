
#include "stdafx.h"

#include <converter_shared/config/PolyflagConfig.h>
#include <converter_shared/helpers/XmlHelpers.h>
#include <converter_shared/helpers/AftkHelpers.h>


namespace Polyflags
{

// C_Flag

C_Flag::C_Flag()
	: m_Id("<NotSet>")
	, m_Value(0)
{
}

C_Flag::~C_Flag()
{
}

bool C_Flag::ParseXml( const Sys::C_XmlDomBaseNode* pNode )
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
				m_Value = atoi( pAttribute->value.Chars() );
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

void C_Flag::Dump()
{
	LOGDEBUG( Tools::g_log, "   Flag '%s' = %d", m_Id.Chars(), m_Value );
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

bool C_EnumItem::ParseXml( const Sys::C_XmlDomBaseNode* pNode )
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
				m_Index = atoi( pAttribute->value.Chars() );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "id" ) == 0)
			{
				m_Id = pAttribute->value;
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "value" ) == 0)
			{
				m_Value = atoi( pAttribute->value.Chars() );
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

void C_EnumItem::Dump()
{
	LOGDEBUG( Tools::g_log, "      EnumItem[%d:%s] = %d", m_Index, m_Id.Chars(), m_Value );
}

// C_Enum

C_Enum::C_Enum()
	: m_Id("<NotSet>")
{
}

C_Enum::~C_Enum()
{
}

bool C_Enum::ParseXml( const Sys::C_XmlDomBaseNode* pNode )
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

				bool success = newEnumItem.ParseXml( pChildNode );
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

void C_Enum::Dump()
{
	LOGDEBUG( Tools::g_log, "   Enum '%s'", m_Id.Chars() );

	for(Btl::Vector<C_EnumItem>::Iterator i = m_EnumItemList.Begin(); i < m_EnumItemList.End(); ++i)
	{
		(*i).Dump();
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

bool C_LayerFlag::ParseXml( const Sys::C_XmlDomBaseNode* pNode )
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
				m_Value = atoi( pAttribute->value.Chars() );
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

void C_LayerFlag::Dump()
{
	LOGDEBUG( Tools::g_log, "   Layer Flag '%s' = %d", m_Id.Chars(), m_Value );
}

// C_Item

C_Item::C_Item()
	: m_Name("<NoName>")
{
}

C_Item::C_Item( const Util::C_String& name )
	: m_Name( name )
{
}

C_Item::~C_Item()
{
}

bool C_Item::ParseXml( const Sys::C_XmlDomBaseNode* pNode )
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
			if( _stricmp( pChildNode->GetName().Chars(), "Flag") == 0 )
			{
				// add new flag
				const S32 flagCount = m_FlagList.Size();
				m_FlagList.Resize( flagCount + 1 );
				C_Flag& newFlag = m_FlagList[flagCount];

				bool success = newFlag.ParseXml( pChildNode );
				if(!success) return false;
			}
			else
			if( _stricmp( pChildNode->GetName().Chars(), "Enum") == 0 )
			{
				// add new enum
				const S32 enumCount = m_EnumList.Size();
				m_EnumList.Resize( enumCount + 1 );
				C_Enum& newEnum = m_EnumList[enumCount];

				bool success = newEnum.ParseXml( pChildNode );
				if(!success) return false;
			}
			else
			if( _stricmp( pChildNode->GetName().Chars(), "LayerFlag") == 0 )
			{
				// add new layer flag
				const S32 layerFlagCount = m_LayerFlagList.Size();
				m_LayerFlagList.Resize( layerFlagCount + 1 );
				C_LayerFlag& newLayerFlag = m_LayerFlagList[layerFlagCount];

				bool success = newLayerFlag.ParseXml( pChildNode );
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

const Util::C_String& C_Item::GetName() const
{
	return m_Name;
}

const Util::C_String C_Item::GetFirstFlagId() const
{
	if(m_FlagList.Size() > 0)
	{		
		return m_FlagList[0].m_Id;
	}

	return Util::C_String("");
}

void C_Item::Dump()
{
	LOGDEBUG( Tools::g_log, "Item '%s'", GetName().Chars() );

	for(Btl::Vector<C_Flag>::Iterator i = m_FlagList.Begin(); i < m_FlagList.End(); ++i)
	{
		(*i).Dump();
	}

	for(Btl::Vector<C_Enum>::Iterator i = m_EnumList.Begin(); i < m_EnumList.End(); ++i)
	{
		(*i).Dump();
	}

	for(Btl::Vector<C_LayerFlag>::Iterator i = m_LayerFlagList.Begin(); i < m_LayerFlagList.End(); ++i)
	{
		(*i).Dump();
	}
}

// C_Config

C_Config::C_Config()
{
}

C_Config::~C_Config()
{
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

bool C_Config::ParseXml( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom )
{
	const Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr();

	if(!pRootNode)
		return false;

	return ParseXml( pRootNode );
}

bool C_Config::ParseXml( const Sys::C_XmlDomBaseNode* pRootNode )
{
	const Sys::C_XmlDomBaseNode::Array& childArray = pRootNode->GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			if( _stricmp( pChildNode->GetName().Chars(), "Item") == 0 )
			{
				// add new item
				//const S32 itemCount = m_ItemList.Size();
				//m_ItemList.Resize( itemCount + 1 );
				//C_Item& newItem = m_ItemList[itemCount];

				//bool success = newItem.ParseXml( pChildNode );
				//if(!success) return false;

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
								bool success = pItem->ParseXml( pChildNode );
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

void C_Config::Dump()
{
	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "Polyflags::C_Config:" );

	for(Btl::Vector<C_Item>::Iterator i = m_ItemList.Begin(); i < m_ItemList.End(); ++i)
	{
		(*i).Dump();
	}

	LOGDEBUG( Tools::g_log, "" );
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

	S32 itemCount = config.GetItemCount();
	m_ValueList.Resize( itemCount );
	for(S32 itemNo=0; itemNo<itemCount; itemNo++)
	{
		m_ValueList[itemNo]=0;
	}

	// TODO : optimise this function by using a hashmap between the set of shared aftkAttributes and the polyflag values?

	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
	if(pBlindDataResource != NULL)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();
		for (U32 i = 0; i < aftkBlindDataNode.GetNumAttributes(); i++) 
		{
			const Aftk::C_GenericAttribute&   aftkAttribute = aftkBlindDataNode.GetAttribute( i ); 

			const char* attrName = aftkAttribute.GetName().c_str();

			for(S32 itemNo=0; itemNo<itemCount; itemNo++)
			{
				const C_Item& item = config.GetItem(itemNo);
				C_Value& value = m_ValueList[itemNo];

				// get value from flags
				const S32 flagCount = item.m_FlagList.Size();
				for(S32 flagNo=0; flagNo < flagCount; flagNo++)
				{
					C_Flag currFlag = item.m_FlagList[flagNo];
					if( _stricmp( currFlag.m_Id.Chars(), attrName ) == 0)
					{
						value |= currFlag.m_Value;
					}
				}

				// get value from enum
				const S32 enumCount = item.m_EnumList.Size();
				for(S32 enumNo=0; enumNo < enumCount; enumNo++)
				{
					C_Enum currEnum = item.m_EnumList[enumNo];
					if( _stricmp( currEnum.m_Id.Chars(), attrName ) == 0)
					{
						if( aftkAttribute.GetDataType() == Aftk::E_GenericAttributeType_U32 ) 
						{
							// get value from enum
							U32 enumValue; //enum value as unsigned int
							//AFTK_ASSERT(aftkAttribute.GetDataType() == Aftk::E_GenericAttributeType_U32);
							aftkAttribute.GetValue(enumValue);

							// find enum value
							const S32 enumItemCount = currEnum.m_EnumItemList.Size();
							for(S32 enumItemNo=0; enumItemNo<enumItemCount; enumItemNo++)
							{
								C_EnumItem& currEnumItem = currEnum.m_EnumItemList[enumItemNo];
								if( currEnumItem.m_Index == enumValue )
								{
									value |= currEnumItem.m_Value;
								}
							}
						}
						else
						{
							std::string enumValue;

							//AFTK_ASSERT(aftkAttribute.GetDataType() == Aftk::E_GenericAttributeType_String);
							aftkAttribute.GetValue(enumValue);

							// find enum value
							const S32 enumItemCount = currEnum.m_EnumItemList.Size();
							for(S32 enumItemNo=0; enumItemNo<enumItemCount; enumItemNo++)
							{
								C_EnumItem& currEnumItem = currEnum.m_EnumItemList[enumItemNo];
								if( _stricmp( currEnumItem.m_Id.Chars(), enumValue.c_str() ) != 0 )
								{
									value |= currEnumItem.m_Value;
								}
							}
						}
					}
				}
			}
		}            
	}  

	// get value from layer names
	for(S32 itemNo=0; itemNo<itemCount; itemNo++)
	{
		const C_Item& item = config.GetItem(itemNo);
		C_Value& value = m_ValueList[itemNo];

		const S32 layerFlagCount = item.m_LayerFlagList.Size();
		for(S32 layerFlagNo=0; layerFlagNo < layerFlagCount; layerFlagNo++)
		{
			C_LayerFlag currLayerFlag = item.m_LayerFlagList[layerFlagNo];
			if( AftkHelpers::InNamedLayer((Aftk::C_SceneNode*)pSceneMeshNode, currLayerFlag.m_Id) )
			{
				value |= currLayerFlag.m_Value;
			}
		}
	}
}

C_Polyflags::~C_Polyflags()
{
}

C_Value C_Polyflags::GetValue( const S32 index )
{
	return m_ValueList[index];
}


} // namespace Polyflags
