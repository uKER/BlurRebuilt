#include "stdafx.h"

#include <core/utility/singleton.h>

#include <converter_shared/helpers/XmlHelpers.h>
#include <tools_shared/utility/MacroExpander.h>

#include <data/ToolConfig.h>

C_ModelOptions::C_ModelOptions()
: m_Name("Not Set")
, m_DefaultEffect("")
, m_LodConfigFilename("")
, m_UseSharedTextures(true)
{
}

C_ModelOptions::~C_ModelOptions()
{
}

bool C_ModelOptions::ParseXml( const Sys::C_XmlDomElement& element )
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
			if( _stricmp( pAttribute->name.Chars(), "default_effect" ) == 0)
			{
				m_DefaultEffect = pAttribute->value;
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "lod_config_filename" ) == 0)
			{
				m_LodConfigFilename = pAttribute->value;
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "pvs_config_filename" ) == 0)
			{
				m_PVSConfigFilename = pAttribute->value;
				Tools::C_MacroExpander::Instance().Expand(m_PVSConfigFilename);
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "use_shared_textures" ) == 0)
			{
				if( _stricmp( pAttribute->value.Chars(), "true" ) == 0)
					m_UseSharedTextures = true;

				if( _stricmp( pAttribute->value.Chars(), "false" ) == 0)
					m_UseSharedTextures = false;
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

const Util::C_String& C_ModelOptions::GetName() const
{
	return m_Name;
}

const Util::C_String& C_ModelOptions::GetDefaultEffect() const
{
	return m_DefaultEffect;
}

const Util::C_String& C_ModelOptions::GetLodConfigFilename() const
{
	return m_LodConfigFilename;
}

const Util::C_String& C_ModelOptions::GetPVSConfigFilename() const
{
	return m_PVSConfigFilename;
}

const bool C_ModelOptions::GetUseSharedTextures() const
{
	return m_UseSharedTextures;
}

void C_ModelOptions::Dump() const
{
	LOGDEBUG( Tools::g_log, "   C_ModelOptions" );
	LOGDEBUG( Tools::g_log, "   m_Name: %s", m_Name.Chars() );
	LOGDEBUG( Tools::g_log, "   m_DefaultEffect: %s", m_DefaultEffect.Chars() );
	LOGDEBUG( Tools::g_log, "   m_LodConfigFilename: %s", m_LodConfigFilename.Chars() );	
	LOGDEBUG( Tools::g_log, "   m_UseSharedTextures: %d", m_UseSharedTextures );		
	LOGDEBUG( Tools::g_log, "" );
}



// class C_ToolConfig

C_ToolConfig::C_ToolConfig()
	: m_UseVertexNormalForTangents(false)
	, m_SmoothNormalAngularTolerance(30.0f)
	, m_SmoothTangentAngularTolerance(30.0f)
	, m_MaxConvexHullTriangles(64)
{

}

C_ToolConfig::~C_ToolConfig()
{

}

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

	m_VisFilenameWin32 = "you don't have vis_filename_win32 defined in your tools config xml";
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "vis_filename_win32", m_VisFilenameWin32);
	Tools::C_MacroExpander::Instance().Expand(m_VisFilenameWin32);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "route_layer_name_base", m_RouteLayerNameBase);
	if(!success) return false;
	
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "texture_cache_path", m_TextureCachePath);
	if(!success) return false;

	Tools::C_MacroExpander::Instance().Expand(m_TextureCachePath);
	
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "fxb_path", m_FxbPath);
	if(!success) return false;

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "missing_texture_filename", m_MissingTextureFilename, XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_MissingTextureFilename);

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "use_vertex_normal_for_tangents", m_UseVertexNormalForTangents, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "smooth_normal_angular_tolerance", m_SmoothNormalAngularTolerance, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "smooth_tangent_angular_tolerance", m_SmoothTangentAngularTolerance, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "max_convex_hull_triangles", m_MaxConvexHullTriangles, XmlHelpers::E_Behaviour_Silent);
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
			if(_stricmp(pChildNode->GetName().Chars(), "model_type") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					S32 index = m_ModelOptions.Size();
					m_ModelOptions.Resize( index + 1 );
					C_ModelOptions& ModelOptions = m_ModelOptions[index];
					bool success = ModelOptions.ParseXml(*pChildElement);
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

const Util::C_String& C_ToolConfig::GetMissingTextureFilename() const
{
	return m_MissingTextureFilename;
}
const bool C_ToolConfig::GetUseVertexNormalForTangents() const
{
	return m_UseVertexNormalForTangents;
}

const S32 C_ToolConfig::GetMaxConvexHullTriangles() const
{
	return m_MaxConvexHullTriangles;
}

const F32 C_ToolConfig::GetSmoothNormalAngularTolerance() const
{
	return m_SmoothNormalAngularTolerance;
}

const F32 C_ToolConfig::GetSmoothTangentAngularTolerance() const
{
	return m_SmoothTangentAngularTolerance;
}

const S32 C_ToolConfig::GetModelOptionsCount() const
{
	return m_ModelOptions.Size();
}

const Util::C_String& C_ToolConfig::GetVisFilenameWin32() const
{
	return m_VisFilenameWin32;
}

const C_ModelOptions& C_ToolConfig::GetModelOptions( const S32 index ) const
{
	return m_ModelOptions[index];
}

const C_ModelOptions* C_ToolConfig::LookupModelOptions( const Util::C_String& name ) const
{
	for(S32 i=0; i<GetModelOptionsCount(); i++)
	{		
		const C_ModelOptions* pModelOptions = &GetModelOptions(i);
		if(pModelOptions->m_Name == name)
		{
			return pModelOptions;
		}
	}
	return NULL;
}

void C_ToolConfig::Dump()
{
	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "C_ToolConfig:" );
	LOGDEBUG( Tools::g_log, "m_PolyflagConfigFilename: %s", m_PolyflagConfigFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LuxBaseConfigFilename: %s", m_LuxBaseConfigFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LuxFilenameWin32: %s", m_LuxFilenameWin32.Chars() );
	LOGDEBUG( Tools::g_log, "m_LuxFilenameYdl: %s", m_LuxFilenameYdl.Chars() );
	LOGDEBUG( Tools::g_log, "m_LuxFilenameServer: %s", m_LuxFilenameServer.Chars() );
	LOGDEBUG( Tools::g_log, "m_RouteLayerNameBase: %s", m_RouteLayerNameBase.Chars() );
	LOGDEBUG( Tools::g_log, "m_UseVertexNormalForTangents: %d", m_UseVertexNormalForTangents );
	LOGDEBUG( Tools::g_log, "m_MaxConvexHullTriangles: %d", m_MaxConvexHullTriangles );
	LOGDEBUG( Tools::g_log, "m_SmoothNormalAngularTolerance: %f", m_SmoothNormalAngularTolerance);
	LOGDEBUG( Tools::g_log, "m_SmoothTangentAngularTolerance: %f", m_SmoothTangentAngularTolerance );
	for(S32 i = 0; i < GetModelOptionsCount(); i++)
	{
		GetModelOptions(i).Dump();
	}
	LOGDEBUG( Tools::g_log, "" );
}