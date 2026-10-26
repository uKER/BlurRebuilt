#include "stdafx.h"

#include <core/utility/singleton.h>

#include <converter_shared/helpers/XmlHelpers.h>
#include <tools_shared/utility/MacroExpander.h>

#include <data/ToolConfig.h>
#include <data/ConversionOptions.h>
#include <converter_shared/lux_interface/LuxInterface.h>

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
			if( _stricmp( pAttribute->name.Chars(), "lightpointsampledata_format" ) == 0)
			{
				m_LightPointSampleDataFormat = pAttribute->value;
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

const Util::C_String& C_LightingModelOptions::GetLightPointSampleDataFormat() const
{
	return m_LightPointSampleDataFormat;
}

void C_LightingModelOptions::Dump() const
{
	LOGDEBUG( Tools::g_log, "   C_LightingModelOptions" );
	LOGDEBUG( Tools::g_log, "   m_Name: %s", m_Name.Chars() );
	LOGDEBUG( Tools::g_log, "   m_LuxSpecificConfigFilename: %s", m_LuxSpecificConfigFilename.Chars() );
	LOGDEBUG( Tools::g_log, "   m_EffectVariation: %s", m_EffectVariation.Chars() );
	LOGDEBUG( Tools::g_log, "   m_LightPointSampleDataFormat: %s", m_LightPointSampleDataFormat.Chars() );
	LOGDEBUG( Tools::g_log, "" );
}

C_CrowdCropSheetOptions::C_CrowdCropSheetOptions()
: width(0), height(0), border(0), minImageSize(0), maxImageSize(0)
{
}
bool C_CrowdCropSheetOptions::ParseXml( const Sys::C_XmlDomElement& element )
{
	const Sys::C_XmlAttributeList& attributes = element.GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if (_stricmp( pAttribute->name.Chars(), "width" ) == 0)
				width = atoi(pAttribute->value);
			else
			if (_stricmp( pAttribute->name.Chars(), "height" ) == 0)
				height = atoi(pAttribute->value);
			else
			if (_stricmp( pAttribute->name.Chars(), "border" ) == 0)
				border = atoi(pAttribute->value);
			else
			if (_stricmp( pAttribute->name.Chars(), "minImageSize" ) == 0)
				minImageSize = atoi(pAttribute->value);
			else
			if (_stricmp( pAttribute->name.Chars(), "maxImageSize" ) == 0)
				maxImageSize = atoi(pAttribute->value);
		}
	}

	if (!IsValid())
	{
		width = height = border = minImageSize = maxImageSize = 0;
		return false;
	}
	else
		return true;
}

bool C_CrowdCropSheetOptions::IsValid() const
{
	return	width && height &&
			minImageSize <= width && 
			minImageSize <= height &&
			border < width;
}

void C_CrowdCropSheetOptions::Dump() const
{
	LOGDEBUG( Tools::g_log, "C_CrowdCropSheetOptions" );
	LOGDEBUG( Tools::g_log, "   width: %d", width );
	LOGDEBUG( Tools::g_log, "   height: %d", height );
	LOGDEBUG( Tools::g_log, "   border: %d", border );
	LOGDEBUG( Tools::g_log, "   minImageSize: %d", minImageSize );
	LOGDEBUG( Tools::g_log, "   maxImageSize: %d", maxImageSize );
	LOGDEBUG( Tools::g_log, "" );

}


// class C_ToolConfig

C_ToolConfig::C_ToolConfig()
	: m_LightSampleGridSpacing( 1.0f )
	, m_LightSampleYSpacing( 1.1f )
	, m_LightSampleHeightOffset( 1.0f )
	, m_LightSampleMaxCount(200000)
	, m_LightmapDensity(2.0f)
	, m_UseVertexNormalForTangents(false)
	, m_SmoothNormalAngularTolerance(30.0f)
	, m_SmoothTangentAngularTolerance(30.0f)
	, m_MaxConvexHullTriangles(64)
	, m_PvsZoneSize(32.0f)
	, m_PvsZoneHeight(32.0f)
	, m_PvsSampleSpacing(4.0f)
	, m_FluidCollisionFlag(0)
	, m_ForceExtraCollideMeshFlagsValue(0)
	, m_CrowdRotateY180(false)
	, m_SunPenumbra(0.5f)
{
	m_LightmapDensityMultipliers.Resize(LuxInterface::E_PolyflagLightmapDensity_Count);
	for(S32 i=0; i<m_LightmapDensityMultipliers.Size(); i++)
	{
		m_LightmapDensityMultipliers[i] = 1.0f;
	}
	
	m_LightmapResolutionMultipliers.Resize(E_LightmapResolution_Count);
	for(S32 i=0; i<m_LightmapResolutionMultipliers.Size(); i++)
	{
		m_LightmapResolutionMultipliers[i] = 1.0f;
	}
	
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

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "imposter_filename_win32", m_ImposterFilenameWin32);
	//if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_ImposterFilenameWin32);	

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "speedtreegen_filename_win32", m_SpeedTreeGenFilenameWin32);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_SpeedTreeGenFilenameWin32);	

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "vis_filename_win32", m_VisFilenameWin32);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_VisFilenameWin32);	
	
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lux_filename_ydl", m_LuxFilenameYdl);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_LuxFilenameYdl);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lux_filename_server", m_LuxFilenameServer);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_LuxFilenameServer);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "vis_filename_win32", m_VisFilenameWin32);
	if (!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_VisFilenameWin32);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "route_layer_name_base", m_RouteLayerNameBase);
	if(!success) return false;
	
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "texture_cache_path", m_TextureCachePath);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "light_sample_grid_spacing", m_LightSampleGridSpacing);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "light_sample_y_spacing", m_LightSampleYSpacing);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "light_sample_height_offset", m_LightSampleHeightOffset);
	if(!success) return false;
	
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_density", m_LightmapDensity, XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "use_vertex_normal_for_tangents", m_UseVertexNormalForTangents, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "smooth_normal_angular_tolerance", m_SmoothNormalAngularTolerance, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "smooth_tangent_angular_tolerance", m_SmoothTangentAngularTolerance, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "max_convex_hull_triangles", m_MaxConvexHullTriangles, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;
	

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "pvs_zone_size", m_PvsZoneSize, XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "pvs_sample_spacing", m_PvsSampleSpacing, XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	S32 iFluidCollisionFlag = 0;
	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "fluid_collision_flag", iFluidCollisionFlag, XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;
	m_FluidCollisionFlag = iFluidCollisionFlag;

	S32 iForceExtraCollideMeshFlagsValue = 0;
	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "force_extra_collide_mesh_flags_value", iForceExtraCollideMeshFlagsValue, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	m_ForceExtraCollideMeshFlagsValue = iForceExtraCollideMeshFlagsValue;
	
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "sun_penumbra", m_SunPenumbra, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	// lightmap density multipliers controlled by polyflags

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_density_multiplier_high", m_LightmapDensityMultipliers[LuxInterface::E_PolyflagLightmapDensity_High], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_density_multiplier_normal", m_LightmapDensityMultipliers[LuxInterface::E_PolyflagLightmapDensity_Normal], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_density_multiplier_low", m_LightmapDensityMultipliers[LuxInterface::E_PolyflagLightmapDensity_Low], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_density_multiplier_very_low", m_LightmapDensityMultipliers[LuxInterface::E_PolyflagLightmapDensity_VeryLow], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_density_multiplier_ultra_low", m_LightmapDensityMultipliers[LuxInterface::E_PolyflagLightmapDensity_UltraLow], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	// lightmap resolution multipliers controlled by executer drop box

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_resolution_multiplier_low", m_LightmapResolutionMultipliers[E_LightmapResolution_Low], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_resolution_multiplier_medium", m_LightmapResolutionMultipliers[E_LightmapResolution_Medium], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_resolution_multiplier_high", m_LightmapResolutionMultipliers[E_LightmapResolution_High], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_resolution_multiplier_very_high", m_LightmapResolutionMultipliers[E_LightmapResolution_VeryHigh], XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "crowd_rotate_y_180", m_CrowdRotateY180, XmlHelpers::E_Behaviour_Silent);

	const Sys::C_XmlDomBaseNode* pInteger = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), "light_sample_max_count");
	if (pInteger)
		m_LightSampleMaxCount = atoi(((Sys::C_XmlDomElement*)(pInteger))->GetAttributes().GetAttributeByName("value")->value.Chars());

	Tools::C_MacroExpander::Instance().Expand(m_TextureCachePath);
	
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "fxb_path", m_FxbPath);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_FxbPath);
	
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "missing_texture_filename", m_MissingTextureFilename, XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_MissingTextureFilename);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "localised_cubemap_path", m_LocalisedCubemapPath, XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_LocalisedCubemapPath);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "default_cubemap_filename", m_DefaultCubemapFilename, XmlHelpers::E_Behaviour_Warn);
	if(!success) return false;
	Tools::C_MacroExpander::Instance().Expand(m_DefaultCubemapFilename);

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
			if(_stricmp(pChildNode->GetName().Chars(), "crowd_cropsheet") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;
					bool success = m_CrowdCropSheetOptions.ParseXml(*pChildElement);
					//do not fail if not present
				}
			}
			else
			if(_stricmp(pChildNode->GetName().Chars(), "speedtree_flags") == 0)
			{
				if (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					for (S32 i=0; i<pChildNode->GetChildren().Size(); ++i)
					{
						const Sys::C_XmlDomElement* pChild = (const Sys::C_XmlDomElement*) pChildNode->GetChildren()[i];
						const Sys::C_XmlAttribute *pNameAttr = pChild->GetAttributes().GetAttributeByName("name");
						const Sys::C_XmlAttribute *pValueAttr = pChild->GetAttributes().GetAttributeByName("value");

						if (!pNameAttr || !pValueAttr)
							continue;

						C_SpeedTreeFlags flag = {pNameAttr->value, pValueAttr->value};
						m_SpeedTreeFlags.PushBack(flag);
					}
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

const Util::C_String& C_ToolConfig::GetImposterFilenameWin32() const
{
	return m_ImposterFilenameWin32;
}

const Util::C_String& C_ToolConfig::GetSpeedTreeGenFilenameWin32() const
{
	return m_SpeedTreeGenFilenameWin32;
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

const Util::C_String& C_ToolConfig::GetVisFilenameWin32() const
{
	return m_VisFilenameWin32;
}

const Util::C_String& C_ToolConfig::GetRouteLayerNameBase() const
{
	return m_RouteLayerNameBase;
}

const Util::C_String& C_ToolConfig::GetTextureCachePath() const
{
	return m_TextureCachePath;
}

const F32 C_ToolConfig::GetLightSampleGridSpacing() const
{
	return m_LightSampleGridSpacing;
}

const F32 C_ToolConfig::GetLightSampleYSpacing() const
{
	return m_LightSampleYSpacing;
}

const F32 C_ToolConfig::GetLightSampleHeightOffset() const
{
	return m_LightSampleHeightOffset;
}

const S32 C_ToolConfig::GetLightSampleMaxCount() const
{
	return m_LightSampleMaxCount;
}

const bool C_ToolConfig::GetUseVertexNormalForTangents() const
{
	return m_UseVertexNormalForTangents;
}

const F32 C_ToolConfig::GetSmoothNormalAngularTolerance() const
{
	return m_SmoothNormalAngularTolerance;
}

const F32 C_ToolConfig::GetSmoothTangentAngularTolerance() const
{
	return m_SmoothTangentAngularTolerance;
}

const Util::C_String& C_ToolConfig::GetLocalisedCubemapPath() const
{
	return m_LocalisedCubemapPath;
}

const Util::C_String& C_ToolConfig::GetFxbPath() const
{
	return m_FxbPath;
}

const Util::C_String& C_ToolConfig::GetDefaultCubemapFilename() const
{
	return m_DefaultCubemapFilename;
}

const Util::C_String& C_ToolConfig::GetMissingTextureFilename() const
{
	return m_MissingTextureFilename;
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

const F32 C_ToolConfig::GetLightmapDensity() const
{
	return m_LightmapDensity;
}

const U32 C_ToolConfig::GetFluidCollisionFlag() const
{
	return m_FluidCollisionFlag;
}

const F32 C_ToolConfig::GetSunPenumbra() const
{
	return m_SunPenumbra;
}

const F32 C_ToolConfig::GetPvsZoneSize() const
{
	return m_PvsZoneSize;
}

const F32 C_ToolConfig::GetPvsZoneHeight() const
{
	return m_PvsZoneHeight;
}

const S32 C_ToolConfig::GetMaxConvexHullTriangles() const
{
	return m_MaxConvexHullTriangles;
}

const F32 C_ToolConfig::GetPvsSampleSpacing() const
{
	return m_PvsSampleSpacing;
}

const U32 C_ToolConfig::GetForceExtraCollideMeshFlagsValue() const
{
	return m_ForceExtraCollideMeshFlagsValue;
}

const F32 C_ToolConfig::GetLightmapDensityMultiplier( const LuxInterface::E_PolyflagLightmapDensity index ) const
{
	return m_LightmapDensityMultipliers[index];
}

const F32 C_ToolConfig::GetLightmapResolutionMultiplier( const E_LightmapResolution index ) const
{
	return m_LightmapResolutionMultipliers[index];
}

const SpeedTreeFlagArray& C_ToolConfig::GetSpeedTreeFlags() const
{
	return m_SpeedTreeFlags;
}

const C_CrowdCropSheetOptions& C_ToolConfig::GetCrowdCropSheetOptions() const
{
	return m_CrowdCropSheetOptions;
}

const bool C_ToolConfig::CrowdRotateY180() const
{
	return m_CrowdRotateY180;
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
	LOGDEBUG( Tools::g_log, "m_VisFilenameWin32: %s", m_VisFilenameWin32.Chars() );
	LOGDEBUG( Tools::g_log, "m_SpeedTreeGenFilenameWin32: %s", m_SpeedTreeGenFilenameWin32.Chars() );
	LOGDEBUG( Tools::g_log, "m_RouteLayerNameBase: %s", m_RouteLayerNameBase.Chars() );
	LOGDEBUG( Tools::g_log, "m_LightmapDensity: %f", m_LightmapDensity );
	LOGDEBUG( Tools::g_log, "m_UseVertexNormalForTangents: %d", m_UseVertexNormalForTangents );
	LOGDEBUG( Tools::g_log, "m_SmoothNormalAngularTolerance: %f", m_SmoothNormalAngularTolerance);
	LOGDEBUG( Tools::g_log, "m_SmoothTangentAngularTolerance: %f", m_SmoothTangentAngularTolerance );
	LOGDEBUG( Tools::g_log, "m_FluidCollisionFlag: 0x%08x", m_FluidCollisionFlag );
	LOGDEBUG( Tools::g_log, "m_MaxConvexHullTriangles: %d", m_MaxConvexHullTriangles );	
	LOGDEBUG( Tools::g_log, "m_PvsZoneSize: %f", m_PvsZoneSize );
	LOGDEBUG( Tools::g_log, "m_PvsSampleSpacing: %f", m_PvsSampleSpacing );
	LOGDEBUG( Tools::g_log, "m_ForceExtraCollideMeshFlagsValue: 0x%08x", m_ForceExtraCollideMeshFlagsValue );
	LOGDEBUG( Tools::g_log, "m_SunPenumbra: %f", m_SunPenumbra );	
	LOGDEBUG( Tools::g_log, "m_LocalisedCubemapPath: %s", m_LocalisedCubemapPath.Chars() );
	LOGDEBUG( Tools::g_log, "m_DefaultCubemapFilename: %s", m_DefaultCubemapFilename.Chars() );

	for(S32 i=0; i<m_LightmapDensityMultipliers.Size(); i++)
	{
		LOGDEBUG( Tools::g_log, "m_LightmapDensityMultipliers[%d]: %f", i, m_LightmapDensityMultipliers[i] );
	}
	for(S32 i=0; i<m_LightmapResolutionMultipliers.Size(); i++)
	{
		LOGDEBUG( Tools::g_log, "m_LightmapResolutionMultipliers[%d]: %f", i, m_LightmapResolutionMultipliers[i] );
	}
	LOGDEBUG( Tools::g_log, "" );
	for(S32 i = 0; i < GetLightingModelOptionsCount(); i++)
	{
		GetLightingModelOptions(i).Dump();
	}
	LOGDEBUG( Tools::g_log, "" );

	m_CrowdCropSheetOptions.Dump();

	LOGDEBUG( Tools::g_log, "m_CrowdRotateY180: %d", m_CrowdRotateY180 );
	LOGDEBUG( Tools::g_log, "" );
}
