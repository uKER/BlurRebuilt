#include "stdafx.h"

#include <converter_shared/lux_interface/LuxInterface.h>
#include <system/misc/StringUtil.h>

namespace LuxInterface
{	

// C_LightingModelProperties

C_LightingModelProperties::C_LightingModelProperties()
	: m_Gamma( E_Gamma_Off )
	, m_InputGamma( 1.0f )
	, m_OutputGamma( 1.0f )
	, m_MeterSize( 1.0f )
	, m_Brightness( 1.0f )
	, m_BounceScale( 1.0f )
	, m_Epsilon( 0.0001f )
	, m_AreaLightScale( 1.0f )
	, m_MaxClusterSize( 250.0f )
	, m_MaxClusterArea( 40.0f )
	, m_TexelDensity( 1.0f )
	, m_AlternateTexelDensityRatio( 1.0f/10.0f )
	, m_DefaultLightmapSize( 256 )
	, m_MaxBounces( 2 )
	, m_DefaultBakeTarget( E_BakeTarget_Lightmap )
	, m_AntiAliasing( 8 )
	, m_AlphaScaling( E_Off )
	, m_IgnoreDirectionalLightsFromMaya( E_Off )
	, m_TimeOfDay( E_TimeOfDay_All )
	, m_OutputFormat( E_TextureFormat_DontCare )
	, m_OutputType( E_TextureType_RGB )
	, m_ProgressiveBouncePasses( 0 )
	, m_ColouredBounceLighting( E_Off )
	, m_PrescaleSH( E_Off )
	, m_TiledOutput( E_Off )
	, m_AntiLightLeaking( 0.0f )
	, m_SaveExtraSamplePointData( E_Off )
	, m_CarMode( E_Off )
	, m_DebugMode( E_Off )
	, m_ForceGothamChannels( E_ForceGothamChannels_Off )
	, m_Optimise( E_Off )
	, m_ForceShooterChannels( E_Off )
	, m_BounceLighting( E_Off )
	, m_BouncePathsPerLight( 32 )
	, m_InstanceMode( E_InstanceMode_Instance )
	, m_InstancesDefaultToSHLighting( E_On )
	//, m_BounceChannel( E_Channel_None )
	, m_FixSeams( E_FixSeams_Off )
{
}

C_LightingModelProperties::~C_LightingModelProperties()
{
}


bool C_LightingModelProperties::ParseXml( const Sys::C_XmlDomElement& element )
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

			if( _stricmp( pAttribute->name.Chars(), "Gamma" ) == 0)
			{
				success = GetValue( pAttribute->value, m_Gamma );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "InputGamma" ) == 0)
			{
				success = GetValue( pAttribute->value, m_InputGamma );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "OutputGamma" ) == 0)
			{
				success = GetValue( pAttribute->value, m_OutputGamma );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "MeterSize" ) == 0)
			{
				success = GetValue( pAttribute->value, m_MeterSize );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Brightness" ) == 0)
			{
				success = GetValue( pAttribute->value, m_Brightness );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "BounceScale" ) == 0)
			{
				success = GetValue( pAttribute->value, m_BounceScale );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Epsilon" ) == 0)
			{
				success = GetValue( pAttribute->value, m_Epsilon );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "AreaLightScale" ) == 0)
			{
				success = GetValue( pAttribute->value, m_AreaLightScale );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "MaxClusterSize" ) == 0)
			{
				success = GetValue( pAttribute->value, m_MaxClusterSize );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "MaxClusterArea" ) == 0)
			{
				success = GetValue( pAttribute->value, m_MaxClusterArea );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "TexelDensity" ) == 0)
			{
				success = GetValue( pAttribute->value, m_TexelDensity );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "AlternateTexelDensityRatio" ) == 0)
			{
				success = GetValue( pAttribute->value, m_AlternateTexelDensityRatio );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "DefaultLightmapSize" ) == 0)
			{
				success = GetValue( pAttribute->value, m_DefaultLightmapSize );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "MaxBounces" ) == 0)
			{
				success = GetValue( pAttribute->value, m_MaxBounces );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "DefaultBakeTarget" ) == 0)
			{
				success = GetValue( pAttribute->value, m_DefaultBakeTarget );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "AntiAliasing" ) == 0)
			{
				success = GetValue( pAttribute->value, m_AntiAliasing );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "AlphaScaling" ) == 0)
			{
				success = GetValue( pAttribute->value, m_AlphaScaling );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "IgnoreDirectionalLightsFromMaya" ) == 0)
			{
				success = GetValue( pAttribute->value, m_IgnoreDirectionalLightsFromMaya );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "TimeOfDay" ) == 0)
			{
				success = GetValue( pAttribute->value, m_TimeOfDay );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "OutputFormat" ) == 0)
			{
				success = GetValue( pAttribute->value, m_OutputFormat );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "OutputType" ) == 0)
			{
				success = GetValue( pAttribute->value, m_OutputType );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "ProgressiveBouncePasses" ) == 0)
			{
				success = GetValue( pAttribute->value, m_ProgressiveBouncePasses );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "ColouredBounceLighting" ) == 0)
			{
				success = GetValue( pAttribute->value, m_ColouredBounceLighting );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "PrescaleSH" ) == 0)
			{
				success = GetValue( pAttribute->value, m_PrescaleSH );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "TiledOutput" ) == 0)
			{
				success = GetValue( pAttribute->value, m_TiledOutput );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "AntiLightLeaking" ) == 0)
			{
				success = GetValue( pAttribute->value, m_AntiLightLeaking );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "SaveExtraSamplePointData" ) == 0)
			{
				success = GetValue( pAttribute->value, m_SaveExtraSamplePointData );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "CarMode" ) == 0)
			{
				success = GetValue( pAttribute->value, m_CarMode );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "DebugMode" ) == 0)
			{
				success = GetValue( pAttribute->value, m_DebugMode );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "ForceGothamChannels" ) == 0)
			{
				success = GetValue( pAttribute->value, m_ForceGothamChannels );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Optimise" ) == 0)
			{
				success = GetValue( pAttribute->value, m_Optimise );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "ForceShooterChannels" ) == 0)
			{
				success = GetValue( pAttribute->value, m_ForceShooterChannels );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "BounceLighting" ) == 0)
			{
				success = GetValue( pAttribute->value, m_BounceLighting );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "BouncePathsPerLight" ) == 0)
			{
				success = GetValue( pAttribute->value, m_BouncePathsPerLight );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "InstanceMode" ) == 0)
			{
				success = GetValue( pAttribute->value, m_InstanceMode );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "InstancesDefaultToSHLighting" ) == 0)
			{
				success = GetValue( pAttribute->value, m_InstancesDefaultToSHLighting );
			}
			else
			//if( _stricmp( pAttribute->name.Chars(), "BounceChannel" ) == 0)
			//{
			//	success = GetValue( pAttribute->value, m_BounceChannel );
			//}
			//else
			if( _stricmp( pAttribute->name.Chars(), "FixSeams" ) == 0)
			{
				success = GetValue( pAttribute->value, m_FixSeams );
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

bool C_LightingModelProperties::BuildXml( Sys::C_XmlDomElement& element ) const
{
	Sys::C_XmlAttributeList attributes;
	Util::C_String attributeValue;

	if(m_Gamma != E_Gamma_User)
	{
		GetString( m_Gamma, attributeValue );
		attributes.Add( "Gamma", attributeValue );
	}
	else
	{
		GetString( m_InputGamma, attributeValue );
		attributes.Add( "InputGamma", attributeValue );

		GetString( m_OutputGamma, attributeValue );
		attributes.Add( "OutputGamma", attributeValue );
	}

	GetString( m_MeterSize, attributeValue );
	attributes.Add( "MeterSize", attributeValue );

	GetString( m_Brightness, attributeValue );
	attributes.Add( "Brightness", attributeValue );

	GetString( m_BounceScale, attributeValue );
	attributes.Add( "BounceScale", attributeValue );

	GetString( m_Epsilon, attributeValue );
	attributes.Add( "Epsilon", attributeValue );

	GetString( m_AreaLightScale, attributeValue );
	attributes.Add( "AreaLightScale", attributeValue );

	GetString( m_MaxClusterSize, attributeValue );
	attributes.Add( "MaxClusterSize", attributeValue );

	GetString( m_MaxClusterArea, attributeValue );
	attributes.Add( "MaxClusterArea", attributeValue );

	GetString( m_TexelDensity, attributeValue );
	attributes.Add( "TexelDensity", attributeValue );

	GetString( m_AlternateTexelDensityRatio, attributeValue );
	attributes.Add( "AlternateTexelDensityRatio", attributeValue );

	GetString( m_DefaultLightmapSize, attributeValue );
	attributes.Add( "DefaultLightmapSize", attributeValue );

	GetString( m_MaxBounces, attributeValue );
	attributes.Add( "MaxBounces", attributeValue );

	GetString( m_DefaultBakeTarget, attributeValue );
	attributes.Add( "DefaultBakeTarget", attributeValue );

	GetString( m_AntiAliasing, attributeValue );
	attributes.Add( "AntiAliasing", attributeValue );

	GetString( m_AlphaScaling, attributeValue );
	attributes.Add( "AlphaScaling", attributeValue );

	GetString( m_IgnoreDirectionalLightsFromMaya, attributeValue );
	attributes.Add( "IgnoreDirectionalLightsFromMaya", attributeValue );

	GetString( m_TimeOfDay, attributeValue );
	attributes.Add( "TimeOfDay", attributeValue );

	if( m_OutputFormat != E_TextureFormat_DontCare )
	{
		GetString( m_OutputFormat, attributeValue );
		attributes.Add( "OutputFormat", attributeValue );
	}

	GetString( m_OutputType, attributeValue );
	attributes.Add( "OutputType", attributeValue );

	GetString( m_ProgressiveBouncePasses, attributeValue );
	attributes.Add( "ProgressiveBouncePasses", attributeValue );

	GetString( m_ColouredBounceLighting, attributeValue );
	attributes.Add( "ColouredBounceLighting", attributeValue );

	GetString( m_PrescaleSH, attributeValue );
	attributes.Add( "PrescaleSH", attributeValue );

	GetString( m_TiledOutput, attributeValue );
	attributes.Add( "TiledOutput", attributeValue );

	GetString( m_AntiLightLeaking, attributeValue );
	attributes.Add( "AntiLightLeaking", attributeValue );

	GetString( m_SaveExtraSamplePointData, attributeValue );
	attributes.Add( "SaveExtraSamplePointData", attributeValue );

	GetString( m_CarMode, attributeValue );
	attributes.Add( "CarMode", attributeValue );

	GetString( m_DebugMode, attributeValue );
	attributes.Add( "DebugMode", attributeValue );

	GetString( m_ForceGothamChannels, attributeValue );
	attributes.Add( "ForceGothamChannels", attributeValue );

	GetString( m_Optimise, attributeValue );
	attributes.Add( "Optimise", attributeValue );

	GetString( m_ForceShooterChannels, attributeValue );
	attributes.Add( "ForceShooterChannels", attributeValue );

	GetString( m_BounceLighting, attributeValue );
	attributes.Add( "BounceLighting", attributeValue );

	GetString( m_BouncePathsPerLight, attributeValue );
	attributes.Add( "BouncePathsPerLight", attributeValue );

	GetString( m_InstanceMode, attributeValue );
	attributes.Add( "InstanceMode", attributeValue );

	GetString( m_InstancesDefaultToSHLighting, attributeValue );
	attributes.Add( "InstancesDefaultToSHLighting", attributeValue );

	//GetString( m_BounceChannel, attributeValue );
	//attributes.Add( "BounceChannel", attributeValue );

	GetString( m_FixSeams, attributeValue );
	attributes.Add( "FixSeams", attributeValue );

	element.GetName() = "ModelProperties";
	element.GetAttributes() = attributes;

	return true;
}



// C_Set

C_Set::C_Set()
	: m_SetAttribute( E_SetAttribute_Light )
	, m_LayerName( "" )
{
}

C_Set::C_Set( const E_SetAttribute setAttribute, const Util::C_String layerName )
	: m_SetAttribute( setAttribute )
	, m_LayerName( layerName )
{
}

C_Set::~C_Set()
{
}

bool C_Set::ParseXml( const Sys::C_XmlDomElement& element )
{
	const Sys::C_XmlAttributeList& attributes = element.GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			bool success = true;

			success = GetValue( pAttribute->name, m_SetAttribute );
			success &= GetValue( pAttribute->value, m_LayerName );

			if(!success)
			{
				LOGWARN( Tools::g_log, "Invalid XML parameter '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
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
		}
	}
	

	return true;
}

bool C_Set::BuildXml( Sys::C_XmlDomElement& element ) const
{
	Sys::C_XmlAttributeList attributes;

	Util::C_String attributeName;
	Util::C_String attributeValue;

	GetString( m_SetAttribute, attributeName );
	GetString( m_LayerName, attributeValue );
	attributes.Add( attributeName, attributeValue );
	
	element.GetName() = "Set";
	element.GetAttributes() = attributes;

	return true;
}

// C_Pass

C_Pass::C_Pass()
{
}

C_Pass::~C_Pass()
{
}

bool C_Pass::ParseXml( const Sys::C_XmlDomElement& element )
{
	const Sys::C_XmlDomBaseNode::Array& childArray = element.GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			if(_stricmp(pChildNode->GetName().Chars(), "Set") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					C_Set set;					

					bool success = set.ParseXml( *pChildElement );
					if(!success) return false;

					m_SetList.PushBack( set );
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

bool C_Pass::BuildXml( Sys::C_XmlDomElement& element ) const
{
	element.GetName() = "Pass";

	const S32 setCount = m_SetList.Size();
	for(S32 setNo=0; setNo<setCount; setNo++)
	{
		const C_Set& set = m_SetList[setNo];

		Sys::C_XmlDomElement* newSetElement = new Sys::C_XmlDomElement();
		element.AddChild( newSetElement );

		bool success = set.BuildXml( *newSetElement );
		if(!success) return false;
	}

	return true;
}

// C_SamplePointsProperties

C_SamplePointsProperties::C_SamplePointsProperties()
	: m_Type( E_SamplePointsType_None )
	, m_InputFilename( "" )
	, m_OutputFilename( "" )
	, m_Output( E_SamplePointsOutputFormat_Binary )
{
}

C_SamplePointsProperties::~C_SamplePointsProperties()
{
}


bool C_SamplePointsProperties::ParseXml( const Sys::C_XmlDomElement& element )
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

			if( _stricmp( pAttribute->name.Chars(), "Type" ) == 0)
			{
				success = GetValue( pAttribute->value, m_Type );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "InputFilename" ) == 0)
			{
				success = GetValue( pAttribute->value, m_InputFilename );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Filename" ) == 0)
			{
				success = GetValue( pAttribute->value, m_OutputFilename );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Output" ) == 0)
			{
				success = GetValue( pAttribute->value, m_Output );
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

bool C_SamplePointsProperties::BuildXml( Sys::C_XmlDomElement& element ) const
{
	Sys::C_XmlAttributeList attributes;
	Util::C_String attributeValue;

	GetString( m_Type, attributeValue );
	attributes.Add( "Type", attributeValue );

	GetString( m_InputFilename, attributeValue );
	attributes.Add( "InputFilename", attributeValue );

	GetString( m_OutputFilename, attributeValue );
	attributes.Add( "Filename", attributeValue );

	GetString( m_Output, attributeValue );
	attributes.Add( "Output", attributeValue );

	element.GetName() = "SamplePoints";
	element.GetAttributes() = attributes;

	return true;
}

// C_Channel
C_Channel::C_Channel()
	: m_Name("Unnamed")
	, m_Target( E_Channel_None )
{
}

C_Channel::~C_Channel()
{
}

void C_Channel::ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes )
{
	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			bool success = true;

			if( _stricmp( pAttribute->name.Chars(), "Name" ) == 0)
			{
				success = GetValue( pAttribute->value, m_Name );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Target" ) == 0)
			{
				success = GetValue( pAttribute->value, m_Target );
			}		
			else
			{
				LOGWARN( Tools::g_log, "Unrecognised XML parameter '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}

			if(!success)
			{
				LOGWARN( Tools::g_log, "Invalid XML parameter value '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
			}
		}
	}		
}

void C_Channel::BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const
{
	Util::C_String attributeValue;

	GetString( m_Name, attributeValue );
	attributes.Add( "Name", attributeValue );

	GetString( m_Target, attributeValue );
	attributes.Add( "Target", attributeValue );
}

// C_LightingModel

C_LightingModel::C_LightingModel()
	: m_LightingModelType( E_LightingModelType_GlobalIllumination )
{
}

C_LightingModel::~C_LightingModel()
{
	const S32 lightCount = m_LightList.Size();
	for(S32 i=0; i<lightCount; i++)
	{
		SAFE_DELETE_PTR( m_LightList[i] );
	}
}

bool C_LightingModel::ReadXml( const Util::C_String& filename )
{
	bool success = true;

	LOGDEBUG(Tools::g_log, "Parsing: '%s'\n", filename.Chars());
	Sys::C_XmlDom lightingDom;

	success = lightingDom.Read(filename.Chars());
	if(!success)
	{
		LOGERROR(Tools::g_log, " Problem reading '%s'\n", filename.Chars());
		return false;
	}

	ParseXml( lightingDom );

	return true;
}

bool C_LightingModel::WriteXml( const Util::C_String& filename ) const
{
	bool success = true;

	LOGDEBUG(Tools::g_log, "Writing: '%s'\n", filename.Chars());
	Sys::C_XmlDom lightingDom;

	BuildXml( lightingDom );

	// create directory
	Util::C_String outputDir = Util::ExtractDirectory(filename);
	Sys::C_FileSystem::Instance().CreateDirectory( outputDir );

	// write file
	success = lightingDom.Write(filename.Chars());
	if(!success)
	{
		LOGERROR(Tools::g_log, " Problem writing '%s'\n", filename.Chars());
		return false;
	}

	return true;
}

bool C_LightingModel::ParseLightingModelXml( const Sys::C_XmlDomElement & element )
{
	const Sys::C_XmlAttributeList& attributes = element.GetAttributes();

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "Type" ) == 0)
			{
				bool success = GetValue( pAttribute->value, m_LightingModelType );
				if(!success)
				{
					LOGWARN( Tools::g_log, "Invalid XML parameter '%s = \"%s\"\'", pAttribute->name.Chars(), pAttribute->value.Chars());
					return false;
				}
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
			if(_stricmp(pChildNode->GetName().Chars(), "ModelProperties") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					bool success = m_LightingModelProperties.ParseXml( *pChildElement );
					if(!success) return false;
				}
			}
			else
			if(_stricmp(pChildNode->GetName().Chars(), "Light") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					const Sys::C_XmlAttributeList& lightAttributes = pChildElement->GetAttributes();

					const char* pType = lightAttributes.GetValueByName("Type");
					E_LightType lightType;

					C_Light* pLight = NULL;

					if(GetValue( Util::C_String(pType), lightType ))
					{
						switch(lightType)
						{
							default: pLight = NULL;

							case E_LightType_Directional:	pLight = new C_LightDirectional(); break;
							case E_LightType_Hemisphere:	pLight = new C_LightHemisphere(); break;
							case E_LightType_Hemisphere2:	pLight = new C_LightHemisphere2(); break;
							case E_LightType_BounceGather:	pLight = new C_LightBounceGather(); break;
							case E_LightType_HDR:			pLight = new C_LightHDR(); break;
						}
					}

					if( pLight )
					{
						m_LightList.PushBack( pLight );
						pLight->ParseXmlAttributes( lightAttributes );
					}
					else
					{
						LOGWARN( Tools::g_log, "Failed to create light");
						
					}
				}
			}
			else
			if(_stricmp(pChildNode->GetName().Chars(), "SamplePoints") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;
					bool success = m_SamplePointProperties.ParseXml( *pChildElement );
					if(!success) return false;

				}
			}
			else
			if(_stricmp(pChildNode->GetName().Chars(), "Pass") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					const Sys::C_XmlAttributeList& channelAttributes = pChildElement->GetAttributes();

					C_Pass newPass;

					bool success = newPass.ParseXml( *pChildElement );
					if(!success) return false;

					m_PassList.PushBack( newPass );
				}
			}
			else
			if(_stricmp(pChildNode->GetName().Chars(), "Channel") == 0)
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					const Sys::C_XmlAttributeList& channelAttributes = pChildElement->GetAttributes();

					C_Channel newChannel;

					newChannel.ParseXmlAttributes( channelAttributes );

					m_ChannelList.PushBack( newChannel );
				}
			}
			else
			{
				LOGWARN( Tools::g_log, "Unrecognised XML Node '%s'", pChildNode->GetName().Chars());
			}
		}
	}		

	return true;
}

bool C_LightingModel::ParseXml( const Sys::C_XmlDom& dom )
{
	const Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr();

	if(!pRootNode)
		return false;

	return ParseXml( *pRootNode );
}

bool C_LightingModel::ParseXml( const Sys::C_XmlDomBaseNode& root )
{
	const Sys::C_XmlDomBaseNode::Array& childArray = root.GetChildren();
	const S32 elementCount = childArray.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		Sys::C_XmlDomBaseNode * const pChildNode = childArray[elementNo];

		if(pChildNode)
		{
			if( _stricmp( pChildNode->GetName().Chars(), "lightingmodel") == 0 )
			{
				if( pChildNode->GetType() == Sys::E_XmlDomNodeType_Element)
				{
					Sys::C_XmlDomElement * const pChildElement = (Sys::C_XmlDomElement * const)pChildNode;

					bool success = ParseLightingModelXml( *pChildElement );
					if(!success) return false;
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

bool C_LightingModel::BuildXml( Sys::C_XmlDom& dom ) const
{
	Sys::C_XmlAttributeList attributes;

	Util::C_String attributeValue;
	GetString( m_LightingModelType, attributeValue );
	attributes.Add( "Type", attributeValue );

	Sys::C_XmlDomBaseNode* newRootElement = new Sys::C_XmlDomBaseNode();
	newRootElement->GetName() = "root";

	dom.SetRootNodePtr( newRootElement );

	Sys::C_XmlDomElement* newLightingModelElement = new Sys::C_XmlDomElement();
	newRootElement->AddChild( newLightingModelElement );

	newLightingModelElement->GetName() = "LightingModel";
	newLightingModelElement->GetAttributes() = attributes;
	
	// Write Lighting Model Properties
	{
		Sys::C_XmlDomElement* newLightingModelPropertiesElement = new Sys::C_XmlDomElement();
		newLightingModelElement->AddChild( newLightingModelPropertiesElement );

		bool success = m_LightingModelProperties.BuildXml( *newLightingModelPropertiesElement );
		if(!success) return false;
	}

	// Write Lights
	const S32 lightCount = m_LightList.Size();
	for(S32 lightNo=0; lightNo<lightCount; lightNo++)
	{
		C_Light* pLight = m_LightList[lightNo];

		if(pLight)
		{
			Sys::C_XmlDomElement* newLightElement = new Sys::C_XmlDomElement();
			newLightingModelElement->AddChild( newLightElement );

			Sys::C_XmlAttributeList lightAttributes;

			pLight->BuildXmlAttributes( lightAttributes );

			newLightElement->GetName() = "Light";
			newLightElement->GetAttributes() = lightAttributes;
		}
	}

	// Write sample points info
	if( m_SamplePointProperties.m_Type != E_SamplePointsType_None )
	{
		Sys::C_XmlDomElement* newSamplePointPropertiesElement = new Sys::C_XmlDomElement();
		newLightingModelElement->AddChild( newSamplePointPropertiesElement );

		bool success = m_SamplePointProperties.BuildXml( *newSamplePointPropertiesElement );
		if(!success) return false;		
	}

	// Write passes, sets
	const S32 passCount = m_PassList.Size();
	for(S32 passNo=0; passNo<passCount; passNo++)
	{
		const C_Pass& pass = m_PassList[passNo];

		Sys::C_XmlDomElement* newPassElement = new Sys::C_XmlDomElement();
		newLightingModelElement->AddChild( newPassElement );

		bool success = pass.BuildXml( *newPassElement );
		if(!success) return false;
	}
	
	
	// Write channels
	const S32 channelCount = m_ChannelList.Size();
	for(S32 channelNo=0; channelNo<channelCount; channelNo++)
	{
		const C_Channel& channel = m_ChannelList[channelNo];

		Sys::C_XmlDomElement* newChannelElement = new Sys::C_XmlDomElement();
		newLightingModelElement->AddChild( newChannelElement );

		Sys::C_XmlAttributeList channelAttributes;

		channel.BuildXmlAttributes( channelAttributes );

		newChannelElement->GetName() = "Channel";
		newChannelElement->GetAttributes() = channelAttributes;
	}

	return true;
}

} // LuxInterface

