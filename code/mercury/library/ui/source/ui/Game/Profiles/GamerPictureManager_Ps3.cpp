
#include "stdafx.h"
#include "UI/Game/Profiles/GamerPictureManager_Ps3.h"
#include "UI/Game/Profiles/ProfileInterface.h"
#include "UI/UILogger.h"
#include <systemprofile/SystemProfile.h>
#include <np.h>

namespace UI
{

static void* pngDecCbControlMalloc( U32 size, void* )
{
	return (void*)new U8[size];
}


static S32 pngDecCbControlFree( void *ptr, void* )
{
	delete[](U8*)ptr;
	return 0;
}


const bool C_GamerPictureManager_Ps3::Create( const S32 sLocalMax, const S32 sRemoteMax, UITextureHandle pDefaultTexture, bool bSmall )
{
	Util::C_String32 name;

	U32 size = 50;

	// Allocate the textures
	for ( S32 i = 0; i < sLocalMax; i++ )
	{
		name.Printf( "GAMERPIC_%i", i );
		C_GamerPicture* pGamerPic = new C_GamerPicture( name, pDefaultTexture );

		if ( Display::Device()->CreateTexture2D( size, size, 1, Display::E_Format_Linear | Display::E_Format_Raw, Display::E_Format_A8R8G8B8, &pGamerPic->m_Texture ) )
		{
			pGamerPic->m_Texture.m_gcmTexture.remap = CELL_GCM_REMAP_MODE(
				CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
				CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
				CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);

			m_LocalGamerPictures.PushBack( pGamerPic );
			gSimple2D.SetAlternateTexture( pGamerPic );
		}
		else
		{
			LOGERROR( g_Log, "Failed to create gamerpicture texture" ); 
			SAFE_DELETE_PTR( pGamerPic );
		}
	}

	// Start at 1 since the active player is present in slot 0
	for ( S32 i = 0; i < sRemoteMax; i++ )
	{
		name.Printf( "REMOTE_GAMERPIC_%i", i );
		C_GamerPicture* pGamerPic = new C_GamerPicture( name, pDefaultTexture );

		if ( Display::Device()->CreateTexture2D( size, size, 1, Display::E_Format_Linear | Display::E_Format_Raw, Display::E_Format_A8R8G8B8, &pGamerPic->m_Texture ) )
		{
			pGamerPic->m_Texture.m_gcmTexture.remap = CELL_GCM_REMAP_MODE(
				CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
				CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
				CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);

			m_RemoteGamerPictures.PushBack( pGamerPic );
			gSimple2D.SetAlternateTexture( pGamerPic );
		}
		else
		{
			LOGERROR( g_Log, "Failed to create gamerpicture texture" ); 
			SAFE_DELETE_PTR( pGamerPic );
		}
	}
	return true;
}


const bool C_GamerPictureManager_Ps3::GetLocalPicture( const S32 sPadIndex )
{
	if ( sPadIndex >= m_LocalGamerPictures.Size() )
		return false;

	// The player must be online in order to request gamer pictures
	SysProfile::FullProfile& profile = C_ProfileInterface::Instance().GetPadProfile( sPadIndex );
	if ( profile.m_State != SysProfile::eProfileStateOnline )
		return false;

	RemoteRequest request =
	{
		sPadIndex,
		profile,
		true,
	};

	m_LocalGamerPictures[sPadIndex]->m_Ref = sPadIndex;
	m_LocalGamerPictures[sPadIndex]->m_Free = false;
	m_LocalGamerPictures[sPadIndex]->m_Active = false;

	m_RemoteRequestList.PushBack( request );

	return true;
}


const bool C_GamerPictureManager_Ps3::GetRemotePictures()
{
	if ( !m_RemoteGamerPictures.Size() || !m_RemoteJobList.Size() )
		return false;

	S32 result = 0;
	S32 titleCtxId = 0, transCtxId = 0;

	U8* pData = new U8[ SCE_NET_NP_AVATAR_IMAGE_MAX_SIZE_SMALL ];

	C_PngDecoder PngDecoder;

	S32 sPadIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();

	// TODO -> this should really be passed in, since its being updated from another thread !
	SceNpId localNpId = C_ProfileInterface::Instance().GetPadProfile( sPadIndex ).m_Uid;
	
	// Start grabbing gamer pictures
	for ( S32 i = 0; i < m_RemoteJobList.Size(); i++ )
	{
		result = sceNpLookupCreateTitleCtx( &m_TitleID, &localNpId );

		if ( result > 0 )
			titleCtxId = result;
		else
			continue;

		result = sceNpLookupCreateTransactionCtx( titleCtxId );
		if ( result > 0 )
			transCtxId = result;
		else
		{
			sceNpLookupDestroyTitleCtx(titleCtxId);
			continue;
		}

		const S32 picIndex = m_RemoteJobList[i].slot; 
		C_GamerPicture* picture = ( m_RemoteJobList[i].local ) ? m_LocalGamerPictures[picIndex] : m_RemoteGamerPictures[picIndex];

		U32 dataSize = 0;

		PngDecoder.Create();

		result = sceNpLookupUserProfileWithAvatarSize(  transCtxId,								// ID of an NP lookup transaction context 
														SCE_NP_AVATAR_SIZE_SMALL,				// Picture size
														&m_RemoteJobList[i].user.m_Uid,			// User Id
														NULL,									// Pointer to userInfo structure
														NULL,									// Pointer to aboutme structure
														NULL,									// Pointer to language structure
														NULL,									// Pointer to countryCode structure
														pData,									// Buffer to store obtained image
														SCE_NET_NP_AVATAR_IMAGE_MAX_SIZE_SMALL,	// Buffer size
														&dataSize,								// Actual size of obtained image
														NULL );									// Future extension. Always null
		if ( result == 0 )
		{
			Display::C_LockedRect rect = picture->m_Texture.Lock( 0 );

			if ( rect.GetBits() != NULL ) 
			{
				if ( !PngDecoder.Decode( pData, dataSize, picture->m_Texture.GetWidth() * 4, rect.GetBits() ) )
				{
					picture->m_Texture.Unlock( 0 );
					LOGERROR( g_Log, "Failed to decode gamer pictre png" ); 
				}
				else
				{
					picture->m_Texture.Unlock( 0 );

					MT::MemoryFence();

					picture->m_Active = true;
				}
			}
		}
		else
		{
			LOGINFO(g_Log, "sceNpLookupUserProfileWithAvatarSize() - failed");
		}

		sceNpLookupDestroyTransactionCtx(transCtxId);
		sceNpLookupDestroyTitleCtx(titleCtxId);

		// Release resources
		PngDecoder.Close();
	}

	SAFE_DELETE_ARY( pData );
	m_RemoteJobList.Clear();

	return false;
}

const bool C_PngDecoder::Create()
{
	CellPngDecThreadInParam 	threadInParam;
	CellPngDecThreadOutParam 	threadOutParam;
	
	// Run on PPU and set callbacks for new / delete
	threadInParam.spuThreadEnable	= CELL_PNGDEC_SPU_THREAD_DISABLE;
	threadInParam.ppuThreadPriority	= 512;
	threadInParam.spuThreadPriority	= 200;
	threadInParam.cbCtrlMallocFunc	= pngDecCbControlMalloc;
	threadInParam.cbCtrlMallocArg	= NULL;
	threadInParam.cbCtrlFreeFunc	= pngDecCbControlFree;
	threadInParam.cbCtrlFreeArg		= NULL;

	S32 result = cellPngDecCreate( &m_MainHandle, &threadInParam, &threadOutParam );
	BIZ_ASSERT_MSG( !result, "Failed to create Png Decoder" );

	return ( !result ) ? true : false;
}


const bool C_PngDecoder::Decode( U8* pSource, const S32 sSourceSize, const S32 sBytesPerLine, U8* pDest )
{
	CellPngDecSrc pngdecSrc; 
	pngdecSrc.srcSelect			= CELL_PNGDEC_BUFFER;
	pngdecSrc.fileName			= "";
	pngdecSrc.fileOffset		= 0;
	pngdecSrc.fileSize			= 0;
	pngdecSrc.streamPtr			= pSource;
	pngdecSrc.streamSize		= sSourceSize;
	pngdecSrc.spuThreadEnable	= CELL_PNGDEC_SPU_THREAD_DISABLE;

	CellPngDecOpnInfo pngdecOpenInfo;
	S32 result = cellPngDecOpen( m_MainHandle, &m_SubHandle, &pngdecSrc, &pngdecOpenInfo);
	if ( result )
	{
		BIZ_ASSERT_MSG( 0, "Failed to set stream source" );
		return false;
	}

	//*********************************
	// Set decode params
	//*********************************

	// Read in PNG header
	CellPngDecInfo pngdecInfo;
	result = cellPngDecReadHeader( m_MainHandle, m_SubHandle, &pngdecInfo);
	if ( result )
	{
		BIZ_ASSERT_MSG( 0, "Failed to read header" );
		return false;
	}

	// Set the parameter for PNG Decoder.
	CellPngDecInParam pngdecInParam;
	pngdecInParam.commandPtr		= NULL;
	pngdecInParam.outputMode		= CELL_PNGDEC_TOP_TO_BOTTOM;
	pngdecInParam.outputColorSpace	= CELL_PNGDEC_ARGB;
	pngdecInParam.outputBitDepth	= 8;
	pngdecInParam.outputPackFlag	= CELL_PNGDEC_1BYTE_PER_1PIXEL;

	if ( ( pngdecInfo.colorSpace == CELL_PNGDEC_GRAYSCALE_ALPHA ) || ( pngdecInfo.colorSpace == CELL_PNGDEC_RGBA ) || ( pngdecInfo.chunkInformation & 0x10 ) )
	{
		pngdecInParam.outputAlphaSelect = CELL_PNGDEC_STREAM_ALPHA;
	}
	else
	{
		pngdecInParam.outputAlphaSelect = CELL_PNGDEC_FIX_ALPHA;
	}
	pngdecInParam.outputColorAlpha  = 0xff;

	CellPngDecOutParam pngdecOutParam;
	result = cellPngDecSetParameter( m_MainHandle, m_SubHandle, &pngdecInParam, &pngdecOutParam);
	if ( result )
	{
		BIZ_ASSERT_MSG( 0, "Failed to set parameters" );
		return false;
	}

	//*********************************
	// Decode the PNG
	//*********************************
	CellPngDecDataCtrlParam	pngdecCtrlParam;
	pngdecCtrlParam.outputBytesPerLine = sBytesPerLine;

	CellPngDecDataOutInfo pngdecDataOutInfo;
	result = cellPngDecDecodeData( m_MainHandle, m_SubHandle, pDest, &pngdecCtrlParam, &pngdecDataOutInfo);
	if ( result )
	{
		BIZ_ASSERT_MSG( 0, "Failed to decode PNG" );
		return false;
	}

	return ( !result ) ? true : false;
}


void C_PngDecoder::Close()
{
	cellPngDecClose( m_MainHandle, m_SubHandle );
	cellPngDecDestroy( m_MainHandle );
}

}; // namespace UI