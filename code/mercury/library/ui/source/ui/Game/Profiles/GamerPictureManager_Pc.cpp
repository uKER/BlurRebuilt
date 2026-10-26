
#include "stdafx.h"
#include "UI/Game/Profiles/GamerPictureManager_Pc.h"

namespace UI
{

const bool C_GamerPictureManager_Pc::GetLocalPicture( const S32 sPadIndex )
{
	return false;
}


const bool C_GamerPictureManager_Pc::GetRemotePictures( )
{
	// This will probally gobble up memory unless its cleared
	m_RemoteJobList.Clear();

	return false;
}


}; // namespace UI