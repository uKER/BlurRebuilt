
#ifndef UI_GAMER_PICTURE_MANAGER_PC_H
#define UI_GAMER_PICTURE_MANAGER_PC_H

#include "ui/Game/Profiles/GamerPictureManager.h"

namespace UI
{

class C_GamerPictureManager_Pc : public I_GamerPictureManager
{
public:
	const bool GetLocalPicture( const S32 sPadIndex );

	const bool GetRemotePictures();
};

}; // namespace UI

#endif // UI_GAMER_PICTURE_MANAGER_PC_H