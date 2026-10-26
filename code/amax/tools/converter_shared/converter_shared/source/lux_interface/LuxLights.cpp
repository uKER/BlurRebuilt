#include "stdafx.h"

#include <converter_shared/lux_interface/LuxLights.h>

namespace LuxInterface
{

// C_Light

C_Light::C_Light()
	: m_Name( "Unnamed" )
	, m_Position( 0.0f, 0.0f, 0.0f )
	, m_Colour( 1.0f, 1.0f, 1.0f, 1.0f )
	, m_Intensity( 1.0f )
	, m_Samples( 1 )
	, m_Channel( E_Channel_None )
{
}

C_Light::~C_Light()
{
}

void C_Light::ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes )
{
	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "Name" ) == 0)
			{
				GetValue( pAttribute->value, m_Name );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Position" ) == 0)
			{
				GetValue( pAttribute->value, m_Position );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Colour" ) == 0)
			{
				GetValue( pAttribute->value, m_Colour );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Intensity" ) == 0)
			{
				GetValue( pAttribute->value, m_Intensity );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Samples" ) == 0)
			{
				GetValue( pAttribute->value, m_Samples );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Channel" ) == 0)
			{
				GetValue( pAttribute->value, m_Channel );
			}
		}
	}		
}

void C_Light::BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const
{
	Util::C_String attributeValue;

	GetString( GetLightType(), attributeValue );
	attributes.Add( "Type", attributeValue );

	GetString( m_Name, attributeValue );
	attributes.Add( "Name", attributeValue );

	GetString( m_Position, attributeValue );
	attributes.Add( "Position", attributeValue );

	GetString( m_Colour, attributeValue );
	attributes.Add( "Colour", attributeValue );

	GetString( m_Intensity, attributeValue );
	attributes.Add( "Intensity", attributeValue );

	GetString( m_Samples, attributeValue );
	attributes.Add( "Samples", attributeValue );

	GetString( m_Channel, attributeValue );
	attributes.Add( "Channel", attributeValue );
}


// C_LightHemisphere

C_LightHemisphere::C_LightHemisphere()
	: m_Hemisphere( E_True )
{
}

C_LightHemisphere::~C_LightHemisphere()
{
}

void C_LightHemisphere::ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes )
{
	C_Light::ParseXmlAttributes(attributes );

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "Hemisphere" ) == 0)
			{
				GetValue( pAttribute->value, m_Hemisphere );
			}
		}
	}		
}

void C_LightHemisphere::BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const
{
	C_Light::BuildXmlAttributes( attributes );

	Util::C_String attributeValue;

	GetString( m_Hemisphere, attributeValue );
	attributes.Add( "Hemisphere", attributeValue );
}

// C_LightDirectional

C_LightDirectional::C_LightDirectional()
	: m_Direction( 0.0f, 1.0f, 0.0f )
	, m_Penumbra( 0.0f )
	, m_Lambert( E_Off )
{
}

C_LightDirectional::~C_LightDirectional()
{
}

void C_LightDirectional::ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes )
{
	C_Light::ParseXmlAttributes(attributes );

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "Direction" ) == 0)
			{
				GetValue( pAttribute->value, m_Direction );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Penumbra" ) == 0)
			{
				GetValue( pAttribute->value, m_Penumbra );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Lambert" ) == 0)
			{
				GetValue( pAttribute->value, m_Lambert );
			}
		}
	}		
}

void C_LightDirectional::BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const
{
	C_Light::BuildXmlAttributes( attributes );

	Util::C_String attributeValue;

	GetString( m_Direction, attributeValue );
	attributes.Add( "Direction", attributeValue );

	GetString( m_Penumbra, attributeValue );
	attributes.Add( "Penumbra", attributeValue );

	GetString( m_Lambert, attributeValue );
	attributes.Add( "Lambert", attributeValue );
}

// C_LightHemisphere2

C_LightHemisphere2::C_LightHemisphere2()
	: m_SunDirection( 0.0f, 1.0f, 0.0f)
	, m_SunIntensity( 1.0f )
{
}

C_LightHemisphere2::~C_LightHemisphere2()
{
}

void C_LightHemisphere2::ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes )
{
	C_Light::ParseXmlAttributes(attributes );

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "SunDirection" ) == 0)
			{
				GetValue( pAttribute->value, m_SunDirection );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "SunIntensity" ) == 0)
			{
				GetValue( pAttribute->value, m_SunIntensity );
			}
		}
	}		
}

void C_LightHemisphere2::BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const
{
	C_Light::BuildXmlAttributes( attributes );

	Util::C_String attributeValue;

	GetString( m_SunDirection, attributeValue );
	attributes.Add( "SunDirection", attributeValue );

	GetString( m_SunIntensity, attributeValue );
	attributes.Add( "SunIntensity", attributeValue );
}

// C_LightBounceGather

C_LightBounceGather::C_LightBounceGather()
	: m_BounceScale( 1.0f )
	, m_CalcLightTransfer( E_Off )
	, m_Hemisphere( E_False )
{
}

C_LightBounceGather::~C_LightBounceGather()
{
}

void C_LightBounceGather::ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes )
{
	C_Light::ParseXmlAttributes(attributes );

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "BounceScale" ) == 0)
			{
				GetValue( pAttribute->value, m_BounceScale );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "BouncePasses" ) == 0)
			{
				GetValue( pAttribute->value, m_BouncePasses );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "CalcLightTransfer" ) == 0)
			{
				GetValue( pAttribute->value, m_CalcLightTransfer );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Hemisphere" ) == 0)
			{
				GetValue( pAttribute->value, m_Hemisphere );
			}
		}
	}		
}

void C_LightBounceGather::BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const
{
	C_Light::BuildXmlAttributes( attributes );

	Util::C_String attributeValue;

	GetString( m_BounceScale, attributeValue );
	attributes.Add( "BounceScale", attributeValue );

	GetString( m_BouncePasses, attributeValue );
	attributes.Add( "BouncePasses", attributeValue );

	GetString( m_CalcLightTransfer, attributeValue );
	attributes.Add( "CalcLightTransfer", attributeValue );

	GetString( m_Hemisphere, attributeValue );
	attributes.Add( "Hemisphere", attributeValue );
}

// C_LightHDR

C_LightHDR::C_LightHDR()
	: m_HdrMap( "NoName" )
	, m_Rotation( 0.0f )
{
}

C_LightHDR::~C_LightHDR()
{
}

void C_LightHDR::ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes )
{
	C_Light::ParseXmlAttributes(attributes );

	const S32 attributeCount = attributes.Elements();
	for( S32 attributeNo=0; attributeNo<attributeCount; attributeNo++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex( attributeNo );
		if(pAttribute)
		{
			if( _stricmp( pAttribute->name.Chars(), "HdrMap" ) == 0)
			{
				GetValue( pAttribute->value, m_HdrMap );
			}
			else
			if( _stricmp( pAttribute->name.Chars(), "Rotation" ) == 0)
			{
				GetValue( pAttribute->value, m_Rotation );
			}
		}
	}		
}

void C_LightHDR::BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const
{
	C_Light::BuildXmlAttributes( attributes );

	Util::C_String attributeValue;

	GetString( m_HdrMap, attributeValue );
	attributes.Add( "HdrMap", attributeValue );

	GetString( m_Rotation, attributeValue );
	attributes.Add( "Rotation", attributeValue );
}

} // LuxInterface

