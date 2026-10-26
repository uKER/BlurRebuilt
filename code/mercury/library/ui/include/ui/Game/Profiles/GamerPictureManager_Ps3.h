
#ifndef UI_GAMER_PICTURE_MANAGER_PS3_H
#define UI_GAMER_PICTURE_MANAGER_PS3_H

#include "ui/Game/Profiles/GamerPictureManager.h"

#include <cell/codec.h>

namespace UI
{

/*---------------------------------------------------------------------------------------
	Class	: C_GamerPictureManager_Ps3

	Desc	:
---------------------------------------------------------------------------------------*/
class C_GamerPictureManager_Ps3 : public I_GamerPictureManager
{
public:
	const bool Create( const S32 sLocalMax, const S32 sRemoteMax, UITextureHandle pDefaultTexture, bool bSmall = false );

	const bool GetLocalPicture( const S32 sPadIndex );

	const bool GetRemotePictures();
};


/*---------------------------------------------------------------------------------------
	Class : C_PngDecoder
	Desc	:

---------------------------------------------------------------------------------------*/
class C_PngDecoder
{
public:
	const bool					Create();

	const bool					Decode( U8* pSource, const S32 sSourceSize, const S32 sBytesPerLine, U8* pDest );

	void						Close();

private:
	CellPngDecMainHandle		m_MainHandle;
	CellPngDecSubHandle			m_SubHandle;
};

}; // namespace UI

#endif // UI_GAMER_PICTURE_MANAGER_PS3_H