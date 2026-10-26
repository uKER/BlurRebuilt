#include "stdafx.h"

#include <core/utility/singleton.h>

#include <converter_shared/helpers/XmlHelpers.h>
#include <tools_shared/utility/MacroExpander.h>

#include <data/ToolConfig.h>

C_LightingModelOptions::C_LightingModelOptions()
: m_Name("Not Set")
, m_LuxSpecificConfigFilename("")
, m_EffectVariation("")
{
}

C_LightingModelOptions::~C_LightingModelOptions()
{
}

bool C_LightingModelOptions::ParseXml( const Sys::C_XmlDomElement& element )
{
	// TODO: implement string.IEquals() 
	const Sys::C_XmlAttributeList& attributes = element.GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			bool success = true;

			if( _stricmp( pAttribute->name.Chars(), "name" ) == 0)
			{
				m_Name = pAttribute->value;
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "lux_specific_config" ) == 0)
			{
				m_LuxSpecificConfigFilename = pAttribute->value;
				Tools::C_MacroExpander::Instance().Expand(m_LuxSpecificConfigFilename);
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "effect_variation" ) == 0)
			{
				m_EffectVariation = pAttribute->value;
			}
			else
			{
				LOGWARN( Tools::g_log, "Unrecognised XML parameter '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}

			if(!success)
			{
				LOGWARN( Tools::g_log, "Invalid XML parameter value '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
				return false;
			}
		}
	}

	
	const Sys::C_XmlDomBaseNode::Array& childArray = element.GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			LOGWARN( Tools::g_log, "Unexpected XML node '%s'", pChildNode->GetName().Chars());
			//if(_stricmp(pChildNode->GetName().Chars(), "ModelProperties") == 0)
			//{
			//	if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
			//	{
			//		Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

			//		bool success = m_LightingModelProperties.ParseXml( *pChildElement );
			//		if(!success) return false;
			//	}
			//}
		}
	}
	

	return true;
}

const Util::C_String& C_LightingModelOptions::GetName() const
{
	return m_Name;
}

const Util::C_String& C_LightingModelOptions::GetLuxSpecificConfigFilename() const
{
	return m_LuxSpecificConfigFilename;
}

const Util::C_String& C_LightingModelOptions::GetEffectVariation() const
{
	return m_EffectVariation;
}

void C_LightingModelOptions::Dump() const
{
	LOGDEBUG( Tools::g_log, "   C_LightingModelOptions" );
	LOGDEBUG( Tools::g_log, "   m_Name: %s", m_Name.Chars() );
	LOGDEBUG( Tools::g_log, "   m_LuxSpecificConfigFilename: %s", m_LuxSpecificConfigFilename.Chars() );
	LOGDEBUG( Tools::g_log, "   m_EffectVariation: %s", m_EffectVariation.Chars() );
	LOGDEBUG( Tools::g_log, "" );
}

// class C_ToolConfig

bool C_ToolConfig::ReadXml( const Util::C_String& xmlFilePath )
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

bool C_ToolConfig::ParseXml( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom )
{
	bool success = true;

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "polyflag_config_filename", m_PolyflagConfigFilename);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_PolyflagConfigFilename);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "objectflag_config_filename", m_ObjectflagConfigFilename);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_ObjectflagConfigFilename);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lux_base_config_filename", m_LuxBaseConfigFilename);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_LuxBaseConfigFilename);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lux_filename_win32", m_LuxFilenameWin32);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_LuxFilenameWin32);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lux_filename_ydl", m_LuxFilenameYdl);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_LuxFilenameYdl);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lux_filename_server", m_LuxFilenameServer);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_LuxFilenameServer);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "route_layer_name_base", m_RouteLayerNameBase);
	if(!success) return false;
	
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "texture_cache_path", m_TextureCachePath);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_TextureCachePath);
	
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "fxb_path", m_FxbPath);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_FxbPath);

	const Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr();

	if(!pRootNode)
		return false;

	return ParseXml( *pRootNode );
}

bool C_ToolConfig::ParseXml( const Sys::C_XmlDomBaseNode& root )
{
	if(root.GetType() == Sys::E_XmlDomNodeType_Element)
	{

		Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)&root;
		// TODO: implement string.IEquals() 
		const Sys::C_XmlAttributeList& attributes = pElement->GetAttributes();

		const S32 attributeCount = attributes.Elements();
		for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
		{
			const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
			if(pAttribute)
			{
				bool success = true;

				//if( _stricmp( pAttribute->name.Chars(), "Gamma" ) == 0)
				//{
				//	success = GetValue( pAttribute->value, m_Gamma );
				//}
				//else
				{
					LOGWARN( Tools::g_log, "Unrecognised XML parameter '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
				}

				if(!success)
				{
					LOGWARN( Tools::g_log, "Invalid XML parameter value '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
					return false;
				}
			}
		}
	}
	
	const Sys::C_XmlDomBaseNode::Array& childArray = root.GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			if(_stricmp(pChildNode->GetName().Chars(), "item") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					bool success = ParseXmlItem( *pChildElement );
					if(!success) return false;
				}
			}
			else
			if(_stricmp(pChildNode->GetName().Chars(), "lighting_model_type") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					S32 index = m_LightingModelOptions.Size();
					m_LightingModelOptions.Resize( index + 1 );
					C_LightingModelOptions& lightingModelOptions = m_LightingModelOptions[index];
					bool success = lightingModelOptions.ParseXml(*pChildElement);
					if(!success) return false;
				}
			}
			else
			{
				LOGWARN( Tools::g_log, "Unexpected XML node '%s'", pChildNode->GetName().Chars());
			}
		}
	}

	return true;
}

bool C_ToolConfig::ParseXmlItem( const Sys::C_XmlDomElement& element )
{
	return true;
}

const Util::C_String& C_ToolConfig::GetPolyflagConfigFilename() const
{
	return m_PolyflagConfigFilename;
}

const Util::C_String& C_ToolConfig::GetObjectflagConfigFilename() const
{
	return m_ObjectflagConfigFilename;
}

const Util::C_String& C_ToolConfig::GetLuxBaseConfigFilename() const
{
	return m_LuxBaseConfigFilename;
}

const Util::C_String& C_ToolConfig::GetLuxFilenameWin32() const
{
	return m_LuxFilenameWin32;
}

const Util::C_String& C_ToolConfig::GetLuxFilenameYdl() const
{
	return m_LuxFilenameYdl;
}

const Util::C_String& C_ToolConfig::GetLuxFilenameServer() const
{
	return m_LuxFilenameServer;
}

const Util::C_String& C_ToolConfig::GetRouteLayerNameBase() const
{
	return m_RouteLayerNameBase;
}

const Util::C_String& C_ToolConfig::GetTextureCachePath() const
{
	return m_TextureCachePath;
}

const Util::C_String& C_ToolConfig::GetFxbPath() const
{
	return m_FxbPath;
}

const S32 C_ToolConfig::GetLightingModelOptionsCount() const
{
	return m_LightingModelOptions.Size();
}

const C_LightingModelOptions& C_ToolConfig::GetLightingModelOptions( const S32 index ) const
{
	return m_LightingModelOptions[index];
}

const C_LightingModelOptions* C_ToolConfig::LookupLightingModelOptions( const Util::C_String& name ) const
{
	for(S32 i=0; i<GetLightingModelOptionsCount(); i++)
	{		
		const C_LightingModelOptions* pLightingModelOptions = &GetLightingModelOptions(i);
		if(pLightingModelOptions->m_Name == name)
		{
			return pLightingModelOptions;
		}
	}
	return NULL;
}

void C_ToolConfig::Dump() const
{
	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "C_ToolConfig:" );
	LOGDEBUG( Tools::g_log, "m_PolyflagConfigFilename: %s", m_PolyflagConfigFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LuxBaseConfigFilename: %s", m_LuxBaseConfigFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LuxFilenameWin32: %s", m_LuxFilenameWin32.Chars() );
	LOGDEBUG( Tools::g_log, "m_LuxFilenameYdl: %s", m_LuxFilenameYdl.Chars() );
	LOGDEBUG( Tools::g_log, "m_LuxFilenameServer: %s", m_LuxFilenameServer.Chars() );
	LOGDEBUG( Tools::g_log, "m_RouteLayerNameBase: %s", m_RouteLayerNameBase.Chars() );
	LOGDEBUG( Tools::g_log, "" );
	for(S32 i = 0; i < GetLightingModelOptionsCount(); i++)
	{
		GetLightingModelOptions(i).Dump();
	}
	LOGDEBUG( Tools::g_log, "" );
}