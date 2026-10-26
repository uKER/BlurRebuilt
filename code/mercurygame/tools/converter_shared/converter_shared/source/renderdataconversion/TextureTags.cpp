#include "stdafx.h"

#include <converter_shared/renderdataconversion/TextureTags.h>
#include <file_tag_db/FileTagDB.h>

namespace RenderDataConversion
{

// C_TextureTags
C_TextureTags::C_TextureTags() 
	: m_Exclude( false )
	, m_Shared( false )
	, m_OverrideIprReduce( false )
	, m_AddressModeU( E_TextureTagAddressMode_Wrap )
	, m_AddressModeV( E_TextureTagAddressMode_Wrap )
	, m_FilteringType( E_TextureTagFilter_Linear )
	, m_MipmapBias( 0 )
	, m_MaxAnisotropy( 0 )
	, m_AlphaType( E_TextureTagAlphaType_AlphaDither )
	, m_AlphaRef( 128 )
	, m_DecalOrder( 128 )
{
}

C_TextureTags::~C_TextureTags()
{
}

// I_TextureOptionReader
I_TextureTagsReader::I_TextureTagsReader()
{
}

I_TextureTagsReader::~I_TextureTagsReader()
{
}

// C_DefaultTextureTagOptionReader, default implementation of I_TextureTagOptionReader
C_DefaultTextureTagsReader::C_DefaultTextureTagsReader()
{
}

C_DefaultTextureTagsReader::~C_DefaultTextureTagsReader()
{
}

E_TextureTagAddressMode LookupAddressMode( const Util::C_String& string )
{
	if( stricmp( string.Chars(), "Wrap" ) == 0 )		
	{
		return E_TextureTagAddressMode_Wrap;
	}
	else
	if( stricmp( string.Chars(), "Mirror" ) == 0 )		
	{
		return E_TextureTagAddressMode_Mirror;
	}
	else
	if( stricmp( string.Chars(), "Clamp" ) == 0 )		
	{
		return E_TextureTagAddressMode_Clamp;
	}

	return E_TextureTagAddressMode_Wrap;
}

E_TextureTagFilter LookupFilteringType( const Util::C_String& string )
{
	if( stricmp( string.Chars(), "Linear" ) == 0 )		
	{
		return E_TextureTagFilter_Linear;
	}
	else
	if( stricmp( string.Chars(), "Anisotropic" ) == 0 )		
	{
		return E_TextureTagFilter_Anisotropic;
	}
	else
	if( stricmp( string.Chars(), "Point" ) == 0 )		
	{
		return E_TextureTagFilter_Point;
	}

	return E_TextureTagFilter_Linear;
}

E_TextureTagAlphaType LookupAlphaType( const Util::C_String& string )
{
	if( stricmp( string.Chars(), "Alpha Dither" ) == 0 )		
	{
		return E_TextureTagAlphaType_AlphaDither;
	}
	else
	if( stricmp( string.Chars(), "Punchthrough" ) == 0 )		
	{
		return E_TextureTagAlphaType_Punchthrough;
	}
	else
	if( stricmp( string.Chars(), "Alpha Blend" ) == 0 )		
	{
		return E_TextureTagAlphaType_AlphaBlend;
	}
	else
	if( stricmp( string.Chars(), "Decal" ) == 0 )		
	{
		return E_TextureTagAlphaType_Decal;
	}

	return E_TextureTagAlphaType_AlphaDither;
}

void C_DefaultTextureTagsReader::ReadTextureTags( const Util::C_String& textureFilename, C_TextureTags& result )
{
	const FileTag::C_Tag* pTag = 
		FileTag::C_TagDBManager::Instance().GetTagPtr( textureFilename.Chars() );
	
	if (!pTag)
		return;

	bool bTemp;
	S32 iTemp;
	Util::C_String sTemp;

	if (pTag->GetAttributeValue("exclude", bTemp))
	{
		result.m_Exclude = bTemp;
	}

	//[Ken] - Bug fix here, was looking for "exclude"! Copy and paste error.
	if (pTag->GetAttributeValue("shared", bTemp))
	{
		result.m_Shared = bTemp;
	}

	if (pTag->GetAttributeValue("override_ipr_reduce", bTemp))
	{
		result.m_OverrideIprReduce = bTemp;
	}
		
	if (pTag->GetAttributeValue("address_mode_u", sTemp))
	{
		result.m_AddressModeU = LookupAddressMode( sTemp );
	}
	
	if (pTag->GetAttributeValue("address_mode_v", sTemp))
	{
		result.m_AddressModeV = LookupAddressMode( sTemp );
	}

	if (pTag->GetAttributeValue("filtering_type", sTemp))
	{
		result.m_FilteringType = LookupFilteringType( sTemp );
	}

	if (pTag->GetAttributeValue("mipmap_bias", iTemp))
	{
		result.m_MipmapBias = (S8)Maths::Clamp(iTemp, -127, 127);
	}
	
	if (pTag->GetAttributeValue("max_anisotropy", iTemp))
	{
		result.m_MaxAnisotropy = (U8)Maths::Clamp(iTemp, 0, 15);
	}

	if (pTag->GetAttributeValue("alpha_type", sTemp))
	{
		result.m_AlphaType = LookupAlphaType( sTemp );
	}

	if (pTag->GetAttributeValue("alpha_ref", iTemp))
	{
		result.m_AlphaRef = (U8)Maths::Clamp(iTemp, 0, 255);
	}

	if (pTag->GetAttributeValue("decal_order", iTemp))
	{
		result.m_DecalOrder = (U8)Maths::Clamp(iTemp, 0, 255);
	}

	if (pTag->GetAttributeValue("override_IPR0", sTemp) && sTemp.Length())
	{
		result.m_Override_IPR0 = sTemp;
	}

	if (pTag->GetAttributeValue("override_NEON", sTemp) && sTemp.Length())
	{
		result.m_Override_NEON = sTemp;
	}

	if (pTag->GetAttributeValue("override_BUMP_NORM", sTemp) && sTemp.Length())
	{
		result.m_Override_BUMP_NORM = sTemp;
	}
}

} // RenderDataConversion
