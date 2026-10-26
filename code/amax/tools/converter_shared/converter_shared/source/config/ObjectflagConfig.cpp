
#include "stdafx.h"

#include <converter_shared/config/ObjectflagConfig.h>
#include <converter_shared/helpers/XmlHelpers.h>
#include <converter_shared/helpers/AftkHelpers.h>

namespace Objectflags
{

// C_ObjectNameFlag

C_ObjectNameFlag::C_ObjectNameFlag()
	: m_Id("<NotSet>")
	, m_Type( E_Type_Infix )
	, m_Value(0)
{
}

C_ObjectNameFlag::~C_ObjectNameFlag()
{
}

bool C_ObjectNameFlag::ParseXml( const Sys::C_XmlDomBaseNode* pNode )
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
			if( _stricmp( pAttribute->name.Chars(), "type" ) == 0)
			{
				if( _stricmp( pAttribute->value.Chars(), "infix" ) == 0)
				{
					m_Type = E_Type_Infix;
				}
				else
				if( _stricmp( pAttribute->value.Chars(), "prefix" ) == 0)
				{
					m_Type = E_Type_Prefix;
				}
				else
				if( _stricmp( pAttribute->value.Chars(), "postfix" ) == 0)
				{
					m_Type = E_Type_Postfix;
				}
				else
				{
					LOGWARN( Tools::g_log, "Unexpected Type '%s' in '%s'", pAttribute->value.Chars(), pAttribute->name.Chars());
					return false;
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
			{
				LOGWARN( Tools::g_log, "Unrecognised XML node '%s'", pChildNode->GetName().Chars());
			}
		}
	}		

	return true;
}

void C_ObjectNameFlag::Dump()
{
	Util::C_String typeString;
	switch( m_Type)
	{
		default:
			typeString = "(Unknown)";
		break;
		case E_Type_Infix:
			typeString = "(Infix)";
		break;
		case E_Type_Prefix:
			typeString = "(Prefix)";
		break;
		case E_Type_Postfix:
			typeString = "(Postfix)";
		break;
	}

	LOGDEBUG( Tools::g_log, "   Object Name Flag '%s' %s = %d", m_Id.Chars(), typeString.Chars(), m_Value );
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
			if( _stricmp( pChildNode->GetName().Chars(), "ObjectNameFlag") == 0 )
			{
				// add new object name flag
				const S32 objectNameFlagCount = m_ObjectNameFlagList.Size();
				m_ObjectNameFlagList.Resize( objectNameFlagCount + 1 );
				C_ObjectNameFlag& newObjectNameFlag = m_ObjectNameFlagList[objectNameFlagCount];

				bool success = newObjectNameFlag.ParseXml( pChildNode );
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

void C_Item::Dump()
{
	LOGDEBUG( Tools::g_log, "Item '%s'", GetName().Chars() );

	for(Btl::Vector<C_ObjectNameFlag>::Iterator i = m_ObjectNameFlagList.Begin(); i < m_ObjectNameFlagList.End(); ++i)
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
	LOGDEBUG( Tools::g_log, "Objectflags::C_Config:" );

	for(Btl::Vector<C_Item>::Iterator i = m_ItemList.Begin(); i < m_ItemList.End(); ++i)
	{
		(*i).Dump();
	}

	LOGDEBUG( Tools::g_log, "" );
}

// C_Objectflags

C_Objectflags::C_Objectflags()
{
}

C_Objectflags::C_Objectflags( const C_Config& config, const Aftk::C_SceneNode* pSceneNode )
{
	const Aftk::C_Scene* pAftk = pSceneNode->GetScenePtr();

	Util::C_String nodeName = pSceneNode->GetName().c_str();

	S32 itemCount = config.GetItemCount();
	m_ValueList.Resize( itemCount );
	for(S32 itemNo=0; itemNo<itemCount; itemNo++)
	{
		m_ValueList[itemNo]=0;
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
			if( AftkHelpers::InNamedLayer(pSceneNode, currLayerFlag.m_Id) )
			{
				value |= currLayerFlag.m_Value;
			}
		}

		const S32 objectNameFlagCount = item.m_ObjectNameFlagList.Size();
		for(S32 objectNameFlagNo=0; objectNameFlagNo<objectNameFlagCount; objectNameFlagNo++)
		{
			C_ObjectNameFlag currObjectNameFlag = item.m_ObjectNameFlagList[objectNameFlagNo];

			const S32 nodeNameLength = nodeName.Length();
			const S32 objectFlagNameLength = currObjectNameFlag.m_Id.Length();

			if(nodeNameLength >= objectFlagNameLength)
			{
				switch( currObjectNameFlag.m_Type )
				{
					default:
					break;

					case C_ObjectNameFlag::E_Type_Infix:
					{
						if( nodeName.ISearch( currObjectNameFlag.m_Id ) != -1 )
						{
							value |= currObjectNameFlag.m_Value;
						}
					}
					break;

					case C_ObjectNameFlag::E_Type_Prefix:
					{
						if( nodeName.ISearch( currObjectNameFlag.m_Id ) == 0 )
						{
							value |= currObjectNameFlag.m_Value;
						}
					}
					break;

					case C_ObjectNameFlag::E_Type_Postfix:
					{
						// This test relies on the earlier name length check being done
						if( nodeName.ISearch( currObjectNameFlag.m_Id ) ==  (nodeNameLength - objectFlagNameLength) )
						{
							value |= currObjectNameFlag.m_Value;
						}
					}
					break;
				}
			}	
		}
	}
}

C_Objectflags::C_Objectflags( const C_Config& config, const Aftk::C_LayerNode* pLayerNode )
{
	S32 itemCount = config.GetItemCount();
	m_ValueList.Resize( itemCount );
	for(S32 itemNo=0; itemNo<itemCount; itemNo++)
	{
		m_ValueList[itemNo]=0;
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
			if( AftkHelpers::IsNamedLayer(pLayerNode, currLayerFlag.m_Id) )
			{
				value |= currLayerFlag.m_Value;
			}
		}
	}
}

C_Objectflags::~C_Objectflags()
{
}

C_Value C_Objectflags::GetValue( const S32 index )
{
	return m_ValueList[index];
}


} // namespace Objectflags
