#include "stdafx.h"

#include <system/serialise/C_FileBlock.h>
#include <TextureResource/TextureResource.h>
#include <display/texture/Texture2D.h>

#define LOAD_TEXTURE_RESOURCE_USING_DISPLAY_UTILS 1

#if LOAD_TEXTURE_RESOURCE_USING_DISPLAY_UTILS
#include <display/texture/TextureUtils.h>
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace TextureResource
{

static Debug::Logger &g_Log = Debug::GetLogger("graphics.textureresource");

#if !LOAD_TEXTURE_RESOURCE_USING_DISPLAY_UTILS
enum E_TextureFileVersion
{
	E_TextureFileVersion_Initial = 0,
	E_TextureFileVersion_Current = E_TextureFileVersion_Initial
};
#endif

// C_ModelData

C_TextureResource::C_TextureResource()
	: m_pDisplayTexture(NULL)
{
}

C_TextureResource::~C_TextureResource()
{
}

bool C_TextureResource::LoadResourceData()
{
	Sys::I_File* pFile = OpenResourceFile();

	if( !pFile )
		return false;

	bool success = LoadTextureData( pFile );

	SAFE_DELETE_PTR( pFile );

	return success;
}

bool C_TextureResource::FreeResourceData()
{
	Display::Device()->DestroyTexture2D((Display::C_Texture2D*)m_pDisplayTexture);
	SAFE_DELETE_PTR(m_pDisplayTexture);
	return true;
}

S32 C_TextureResource::GetEvictDelay()
{
	return 0;
}

Util::C_String C_TextureResource::StaticGetResourceTypeName()
{
	return "Texture";
}

bool C_TextureResource::LoadTextureData( Sys::I_File* pFile )
{
#if LOAD_TEXTURE_RESOURCE_USING_DISPLAY_UTILS
	RESOURCE_MEMORY_ID("TextureResource");

	MEMORY_TRACKER_PUSH_SUB("texture_resource");

	U32 flags = 0;
	
	BIZ_ASSERT(!m_pDisplayTexture);

	m_pDisplayTexture = new Display::C_Texture2D();

	if(Display::LoadTexture(m_pDisplayTexture, pFile, flags))
	{
		m_SRGBFlag = (flags != 0) ? true : false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(m_pDisplayTexture, GetResourceFilename().Chars());
		#endif
	}
	else
	{
		SAFE_DELETE_PTR(m_pDisplayTexture);
	}

	MEMORY_TRACKER_POP_SUB("texture_resource");
#else
	const U32 prevFileVersion = pFile->GetVersionNumber();
	U32 fileVersion = E_TextureFileVersion_Initial;

	RESOURCE_MEMORY_ID("TextureResource");
	MEMORY_TRACKER_PUSH_SUB("texture_resource");
	Sys::C_FileBlock rootBlock( pFile );
	if( rootBlock.HeaderId() != "Texture" )
	{
		LOGWARN( g_Log, "Expected a 'Texture' Block" );
		MEMORY_TRACKER_POP_SUB("texture_resource");
		return false;
	}

	// keep reading child blocks
	while ( rootBlock.GetPosition() < rootBlock.Length() )
	{
		Sys::C_FileBlock block( &rootBlock );

		if( block.HeaderId() == "Header" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> fileVersion;
			pFile->SetVersionNumber( fileVersion );
			U32 gamma;
			ar >> gamma;
			m_SRGBFlag = (gamma != 0) ? true : false;
		}
		else			
		if( block.HeaderId() == "Data" )
		{
			Display::C_Texture2D* pTexture2D = new Display::C_Texture2D();
			
			Display::LoadTexture2D(pTexture2D, &block);
			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetTextureName(pTexture2D, GetResourceFilename().Chars());
			#endif

			m_pDisplayTexture = pTexture2D;
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
		}
	}

	pFile->SetVersionNumber( prevFileVersion );
	MEMORY_TRACKER_POP_SUB("texture_resource");
#endif

	return true;
}

} // namespace GameResource
