#ifndef __FXB_H__
#define __FXB_H__

namespace Sys {class I_File;}

namespace FX
{

#define FXB_EXPORT_PC 1
#define FXB_EXPORT_XENON 2
#define FXB_EXPORT_PS3 4

class I_Effect;

/// read in an .fx file, returns a pointer to a new Effect
I_Effect *ImportFXB(Sys::I_File *pFile);

/// read in an .fx file, passes in a pointer to an Effect
bool ImportFXB(Sys::I_File *pFile, I_Effect *pTargetEffect);

}

/// compile an input .fx file into an output binary stream containing an .FXB format file
bool ExportFXB(Sys::I_File *pFile, const char *pszInFile, int iFlags, const char **ppszDefines, const char **ppszIncludes, const char *pszOutputFilename);


#endif
