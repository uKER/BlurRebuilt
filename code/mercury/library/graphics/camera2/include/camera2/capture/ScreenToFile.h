
#ifndef CAMERA2_SCREENTOFILE_H
#define CAMERA2_SCREENTOFILE_H

namespace Sys { class I_File; }

namespace Cam2
{

S32 ScreenToTgaFile(Sys::I_File* file);
S32 ScreenToTgaFile(const char* string);
bool ScreenToNextTga();

}

#endif
